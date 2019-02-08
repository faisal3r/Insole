//*********************************************************************************
//***REQUEST METHODS***************************************************************
//*********************************************************************************

void requestStatus(){
  /*
   * Responds with connected WiFi(s) and IP addresses
   */
  //Create response
  String response = ((String)global_AP_SSID)+"," + WiFi.softAPIP().toString();
  if(WiFi.status() == WL_CONNECTED){
    response += "," + WiFi.SSID() + "," + WiFi.localIP().toString();
  }
  response += "," + WiFi.macAddress();

  //Communicate response
  Serial.println(timeStamp(response,serialComm));
  if(!serialComm){
    server.send(200, "text/plain", response);
  }
}

void requestScan(){
  /*
   * Responds with list of available WiFi networks
   */
  //Create response
  String response = "";
  String networks[50];
  int numberOfNetworks = scanWifi(networks);
  if(numberOfNetworks>0){
    for(int i=0; i<numberOfNetworks; i++){
      response+= networks[i] + ",";
    }
  }
  else{
    response = "No networks found!";
  }

  //Communicate response
  Serial.println(timeStamp(response,serialComm));
  if(!serialComm){
    server.send(200, "text/plain", response);
  }
}

void requestConnect(){
  /*
   * Connects to a WiFi network
   * Responds with "Connected" or "Failed"
   */
  //Create response
  String response;
  String ssid;
  String password;
  boolean attemptConnection = false;
  
  if(serialComm){ //if requested through Serial
    Serial.println("Enter network SSID: ");
    Serial.flush();
    while(Serial.available()==0){} //wait for SSID from Serial
    ssid = Serial.readString();
    delay(10);
    Serial.println(ssid);
    if(true){ //if SSID requires password (assume always for now)
      Serial.println("Enter password: ");
      Serial.flush();
      while(Serial.available()==0){} //wait for password from Serial
      password = Serial.readString();
      delay(10);
      Serial.println(password);
    }
    attemptConnection = true;
  }
  else{ //if requested through HTTP
    if(server.arg("data") != ""){ //check if client provided credentials
      String httpArgs = server.arg("data");
      for(int i=0; i<httpArgs.length(); i++){ //divide received args into SSID and password
        if(httpArgs[i] == ','){
          ssid = httpArgs.substring(0,i);
          password = httpArgs.substring(i+1,httpArgs.length());
        }
      }
      attemptConnection = true;
    }
    else{
      attemptConnection = false;
      response = "Error: No credentials provided!";
    }
  }
  
  if(attemptConnection){
    //check if already connected to this WiFi
    if(WiFi.SSID()==ssid && WiFi.status()==WL_CONNECTED){
      response = "Error: Already connected to ("+ssid+")!";
    }
    else{
      Serial.println(timeStamp("Attempting connection to: ("+ssid+")", serialComm));
      int status = WiFi.begin(ssid.c_str(), password.c_str());
      unsigned long startTime = millis();
      while(WiFi.status() == status){
        Serial.print(".");
        digitalWrite(ledPin,!digitalRead(0)); //toggle led
        delay(500);
        if(millis()-startTime >= 10000){
          break;
        }
      }
      Serial.println();
      if(WiFi.status() == WL_CONNECTED){
        response = "Connected";
      }
      else{
        response = "Failed";
      }
    }
  }
  
  //Communicate response
  Serial.println(timeStamp(response,serialComm));
  if(!serialComm){
    server.send(200, "text/plain", response);
  }
}

void requestDisconnect(){
  /*
   * Disconnects from current WiFi
   */
  //Create response
  String response = "Disconnected"; 
    
  //Communicate response
  Serial.println(timeStamp(response,serialComm));
  if(!serialComm){
    server.send(200, "text/plain", response);
  }

  delay(1000);
  WiFi.disconnect();
}

void requestData(){
  /*
   * Communicate sensor data
   */
  //Create and communicate response
  String response;
  boolean transmitData;
  
  if(serialComm){
    stopData = false;
    while(!stopData){
      Serial.println(timeStamp(readSensorsSerial(),serialComm));
      if(serialComm = Serial.available()){
        inst = Serial.readString();
        Serial.println(inst);
        handleInstruction(inst);
      }
    }
  }else{
    //setup UDP communication
    WiFiUDP udp; //udp server
    IPAddress clientIP;
    unsigned int clientPort;
    unsigned int stopPort = 55555;

    //STAGE 1/2: HTTP Reply
    if(server.arg("data") != ""){ //check if client provided IP and port
      String httpArgs = server.arg("data");
      for(int i=0; i<httpArgs.length(); i++){ //divide received args into IP and port
        if(httpArgs[i] == ','){
          clientIP = parseIP(httpArgs.substring(0,i));
          clientPort = httpArgs.substring(i+1,httpArgs.length()).toInt();
        }
      }
      response = (String)stopPort;
      transmitData = true;
    }
    else{
      transmitData = false;
      response = "Error: IP and port must be provided!";
    }
    server.send(200, "text/plain", response);

    //STAGE 2/2: UDP Communication
    if(transmitData){
      unsigned long count = 0; //packet counter
      unsigned long startTime = millis();
      const uint8_t messageLength = 50;
      char message[messageLength];  //incoming udp buffer
      char bytes[28];//outgoing UDP buffer
      const int streamDuration = 3600; //in seconds
      udp.begin(stopPort); //listen for stop request
      delay(10);
      Serial.println(timeStamp("UDP stream started...",serialComm));
      Serial.print("Sending to ");
      Serial.print(clientIP.toString());
      Serial.print(":");
      Serial.println(clientPort);

      //Start transmission
      while(((millis()-startTime)/1000)<streamDuration){
        wrapSensorsUDP(bytes);//create sensor readings array
        sendUDPDatagram(udp, clientIP, clientPort, bytes);
        count++;
        Serial.print('.');
        if(count%10==0){
          digitalWrite(ledPin,!digitalRead(0)); //toggle led
        }
        if(count%50==0){
          Serial.println();
        }
        if(udp.parsePacket()>0){//Stop transmission if anything received
          Serial.println();
          udp.read(message, messageLength);
          Serial.print(timeStamp("UDP message received: ",false));
          Serial.println(message);
          Serial.println("********************");
          Serial.println("***STREAM STOPPED***");
          Serial.println("********************");
          digitalWrite(ledPin,LOW);
          break;
        }
      }
      Serial.println();
      Serial.println(timeStamp("UDP stream stopped",serialComm));
      Serial.print(count);
      Serial.print(" datagrams sent in ");
      Serial.print(millis()-startTime);
      Serial.println(" ms");
    }
  }
}

void requestCurrentVersion(){
  /*
   * Responds with current installed version number
   */
  //Create response
  String response = currentVersion;

  //Communicate response
  Serial.println(timeStamp(response,serialComm));
  if(!serialComm){
    server.send(200, "text/plain", response);
  }
}

void requestVibrate(){
  /*
   * Starts vibration
   */
  Serial.println("Starting vibration");
  digitalWrite(vibPin, HIGH);
}
void requestStopVibrate(){
  /*
   * Stops vibration
   */
  Serial.println("Vibration stopped!");
  digitalWrite(vibPin, LOW);
}

void requestStopData(){
  /*
   * Stops sending insole data through serial communication
   */
   stopData = true;
}

