#define currentVersion "Insole_v0.9"
/* NEW IN THIS VERSION:
 * -Added requestVibrate(), requestStopVibrate(), and requestStopData();
 * -Added writeMux(int channel, boolean value)
 */
#include <Wire.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

//MPU9250 I2C definitions
#define MPU9250_ADDRESS           0x68
#define MAG-ADDRESS               0x0C
#define ACC_FULL_SCALE_2_G        0x00  
#define ACC_FULL_SCALE_4_G        0x08
#define ACC_FULL_SCALE_8_G        0x10
#define ACC_FULL_SCALE_16_G       0x18
#define GYRO_FULL_SCALE_250_DPS   0x00  
#define GYRO_FULL_SCALE_500_DPS   0x08
#define GYRO_FULL_SCALE_1000_DPS  0x10
#define GYRO_FULL_SCALE_2000_DPS  0x18

//mux pins
#define SEL0  13
#define SEL1  12
#define SEL2  14
#define SEL3  16
#define SIG   A0
#define ledPin 0  //LED indicator
#define vibPin 0

//Global variables
String global_AP_SSID;
uint8_t muxSelect[] = {SEL0,SEL1,SEL2,SEL3};  //mux control pins
ESP8266WebServer server(80);                  //http server
boolean serialComm;                           //true if current communication is serial (false if HTTP)
String inst = "";                             //instruction received by serial
boolean stopData = false;

void setup(){
  Wire.begin();
  Serial.begin(115200);
  //Power indicator
  pinMode(ledPin,OUTPUT);
  
  initializeServer();   //Initialize web server for HTTP communications
    
  //Initialize mux
  for(uint8_t i=0; i<sizeof(muxSelect); i++){
    pinMode(muxSelect[i], OUTPUT);
    digitalWrite(muxSelect[i], LOW);
  }
  pinMode(SIG, INPUT);

  //Initialize IMU
  I2CwriteByte(MPU9250_ADDRESS,28,ACC_FULL_SCALE_16_G);
  I2CwriteByte(MPU9250_ADDRESS,27,GYRO_FULL_SCALE_250_DPS);
}

void loop(){  
  if(serialComm = Serial.available()){
    inst = Serial.readString();
    Serial.println(inst);
    handleInstruction(inst);
  }
  if(WiFi.status()==WL_CONNECTED){
    digitalWrite(0,LOW); //light up!
  }
  else{
    digitalWrite(0,LOW);
    delay(100);
    digitalWrite(0,HIGH);
    delay(1000);
  }
  
  server.handleClient();
}

void handleInstruction(String inst){
  /*
   * Handle received instruction
   */
  inst.toUpperCase();
  
  if(inst == "DATA"){
    requestData();
  }else if(inst == "STOPD"){
    stopData = true;
  }else if(inst == "STAT"){
    requestStatus();
  }else if(inst == "SCAN"){
    requestScan();
  }else if(inst == "CONN"){
    requestConnect();
  }else if(inst == "DISC"){
    requestDisconnect();
  }else if(inst == "VERS"){
    requestCurrentVersion();
  }else if(inst == "VIBR"){
    requestVibrate();
  }else if(inst == "STOPV"){
    requestStopVibrate();
  }
  else if(inst != ""){
    Serial.println(timeStamp("Incorrect instruction received: "+inst, serialComm));
  }
}

