//*********************************************************************************
//***SENSOR READING METHODS********************************************************
//*********************************************************************************

String readSensorsSerial() {
  /*
   * return all sensor data as a string of integers and doubles
   * To be used for arduino serial monitor
   */
  //Create response packet
  //Read mux
  String str = "";
  for (int i = 0; i <= 15; i++) {
    str += ((String)(readMux(i) * 100 / 1024));
    str += ",";
  }

  //Read IMU
  int16_t gyr[3];
  //convert to deg/s: DPS = rawData*scale/2^15
  //scale used is 250
  int16_t acc[3];
  //convert to m/s^2: acceleration = rawData*scale/2^15*9.81
  //scale used is 16
  
  readIMU(gyr, acc);
  str += (String)(gyr[0]*250/pow(2,15)) + "," + (String)(gyr[1]*250/pow(2,15)) + "," + (String)(gyr[2]*250/pow(2,15)) + ",";
  str += (String)(acc[0]*16/pow(2,15)*9.81) + "," + (String)(acc[1]*16/pow(2,15)*9.81) + "," + (String)(acc[2]*16/pow(2,15)*9.81);

  return str;
}

void wrapSensorsUDP(char* bytes) {
  /*
   * puts sensor data into bytes[] for UDP transmission as follows
   *
   * bytes  sensor  size
   * =====================
   * 0      FSR0    1
   * 1      FSR1    1
   * 2      FSR2    1
   * 3      FSR3    1
   * ...    ...     ...
   * 11     FSR11   1
   * 12-15  NULL    4 (for future work)
   * 16-17  GYR_X   2
   * 18-19  GYR_Y   2
   * 20-21  GYR_Z   2
   * 22-23  ACC_X   2
   * 24-25  ACC_Y   2
   * 26-27  ACC_Z   2
   */
  //Read mux
  for (uint8_t i = 0; i <= 15; i++) {
    int32_t temp = readMux(i);
    temp = temp*100/1024;
    bytes[i] = (char)temp;
  }

  //Read IMU
  int16_t gyr[3];
  int16_t acc[3];
  readIMU(gyr, acc);

  bytes[16] = gyr[0] >> 8;
  bytes[17] = gyr[0];
  bytes[18] = gyr[1] >> 8;
  bytes[19] = gyr[1];
  bytes[20] = gyr[2] >> 8;
  bytes[21] = gyr[2];
  
  bytes[22] = acc[0] >> 8;
  bytes[23] = acc[0];
  bytes[24] = acc[1] >> 8;
  bytes[25] = acc[1];
  bytes[26] = acc[2] >> 8;
  bytes[27] = acc[2];

  for(int i=0; i<28; i++){
    bytes[i]+=48; //convert to character so UDP does not confuse it for null
  }
}

int16_t readMux(uint8_t channel) {
  /*
   * reads multiplexor data at specific channel
   */
  int muxChannel[16][4] = {
    {0, 0, 0, 0}, //channel 0
    {1, 0, 0, 0}, //channel 1
    {0, 1, 0, 0}, //channel 2
    {1, 1, 0, 0}, //channel 3
    {0, 0, 1, 0}, //channel 4
    {1, 0, 1, 0}, //channel 5
    {0, 1, 1, 0}, //channel 6
    {1, 1, 1, 0}, //channel 7
    {0, 0, 0, 1}, //channel 8
    {1, 0, 0, 1}, //channel 9
    {0, 1, 0, 1}, //channel 10
    {1, 1, 0, 1}, //channel 11
    {0, 0, 1, 1}, //channel 12
    {1, 0, 1, 1}, //channel 13
    {0, 1, 1, 1}, //channel 14
    {1, 1, 1, 1} //channel 15
  };
  for (int i = 0; i < 4; i ++) {
    digitalWrite(muxSelect[i], muxChannel[channel][i]);
  }
  //read the value at the SIG pin
  int16_t val = analogRead(SIG);
  return val;
}

void readIMU(int16_t* gyr, int16_t* acc) {
  /*
   * Reads MPU9250 gyroscope and accelerometer
   * stores X,Y,Z valus in gyr[] and acc[]
   */
  // Read accelerometer and gyroscope
  uint8_t buf[14];
  I2Cread(MPU9250_ADDRESS, 0x3B, 14, buf);

  // Gyroscope
  gyr[0] = buf[8]  << 8 | buf[9];
  gyr[1] = buf[10] << 8 | buf[11];
  gyr[2] = buf[12] << 8 | buf[13];
  
  // Accelerometer
  acc[0] = buf[0] << 8 | buf[1];
  acc[1] = buf[2] << 8 | buf[3];
  acc[2] = buf[4] << 8 | buf[5];
}
