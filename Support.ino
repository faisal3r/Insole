//*********************************************************************************
//***SUPPORT METHODS***************************************************************
//*********************************************************************************

String timeStamp(String str, boolean serialComm) {
  /*
   * returns a string in the following format:
   * [elapsedTime][SERIAL or WIFI] [str]
   */
  String response = elapsedTime();
  if (serialComm) {
    response += "[SERIAL] ";
  }
  else {
    response += "[WIFI] ";
  }
  response += str;
  return response;
}

String elapsedTime() {
  /*
   * return elapsed time as a readable String
   */
  int h, m, s, ms;
  unsigned long over, elapsed;
  String str = "";
  elapsed = millis();

  h = int(elapsed / 3600000);
  if (h < 10) str += "0" + (String)h;
  else str += h;
  str += ":";

  over = elapsed % 3600000;
  m = int(over / 60000);
  if (m < 10) str += "0" + (String)m;
  else str += m;
  str += ":";

  over = over % 60000;
  s = int(over / 1000);
  if (s < 10) str += "0" + (String)s;
  else str += s;
  str += ".";

  ms = over % 1000;
  if (ms < 10) str += "00" + (String)ms;
  else if (ms < 100) str += "0" + (String)ms;
  else str += (String)ms;

  return str;
}

void initializeServer() {
  /*
   * Initializes ESP8266 as a web server
   * This method is used only once in the microcontroller setup()
   */
  //Print initial status
  Serial.print("Start time\t: ");
  Serial.println(elapsedTime());
  Serial.println("===================");
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_AP_STA);//Access Point and Station
  //Create a semi-unique SSID by adding the last 2 bytes of its MAC address
  global_AP_SSID = "ESP-" + WiFi.macAddress().substring(12, 14) + WiFi.macAddress().substring(15);
  const char *AP_SSID = global_AP_SSID.c_str();
  WiFi.softAP(AP_SSID); //Create Access Point without password
  Serial.print("Access Point ID\t: ");
  Serial.println(AP_SSID);
  Serial.print("Access Point IP\t: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("WiFi Connection\t: ");
  delay(3000);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(WiFi.SSID());
    Serial.print("Local IP\t: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("None");
  }
  //Start server
  Serial.println("Starting HTTP server...");
  //setup http servers
  server.on("/stat", requestStatus);
  server.on("/scan", requestScan);
  server.on("/conn", requestConnect);
  server.on("/disc", requestDisconnect);
  server.on("/data", requestData);
  server.on("/vers", requestCurrentVersion);
  server.begin();
  Serial.println("HTTP server ready");
  Serial.println("===================");
}

int scanWifi(String networks[]) {
  /*
   * Scans for nearby WiFi networks
   * Adds unique SSIDs to networks[]
   * Returns number of SSIDs added
   */
  int n = WiFi.scanNetworks();
  int j = 0;
  String ssid;
  if (n != 0) {
    for (int i = 0; i < 50; i++) { //50 is the original size of the array
      networks[i] = ""; //create new list
    }
    for (int i = 0; i < n; ++i) {
      ssid = WiFi.SSID(i);
      delay(10);
      if (lookup(networks, n, ssid) == -1) { //check if network is already seen
        networks[j++] = ssid;
      }
    }
  }
  return j;
}

int lookup(String array[], int n, String element) {
  /*
   * searches "array" of size "n" for a "element";
   * returns index of found object, or -1 for not found
   */
  for (int i = 0; i < n; i++) {
    if (array[i] == element) {
      return i;
    }
  }
  return -1;
}

IPAddress parseIP(String string) {
  const char* str = string.c_str();
  IPAddress addr;
  for (int i = 0; i < 4; i++) {
    addr[i] = strtoul(str, NULL, 10);
    str = strchr(str, '.');
    if (str == NULL || *str == '0') {
      break; // No more separators, exit
    }
    str++;
  }
  return addr;
}
