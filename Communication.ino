//*********************************************************************************
//***COMMUNICATION METHODS*********************************************************
//*********************************************************************************

void I2CwriteByte(uint8_t addr, uint8_t reg, uint8_t data) {
  /*
   * Write a byte (data) in device (addr) at register (reg)
   */
  // Set register address
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void I2Cread(uint8_t addr, uint8_t reg, uint8_t Nbytes, uint8_t* data) {
  /*
   * This function read Nbytes bytes from I2C device at address (addr).
   * Put read bytes starting at register (reg) in the (data) array.
   */
  // Set register address
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission();

  // Read Nbytes
  Wire.requestFrom(addr, Nbytes);
  uint8_t index = 0;
  while (Wire.available())
    data[index++] = Wire.read();
}

void sendUDPDatagram(WiFiUDP udp, IPAddress addr, int port, char message[]) {
  /*
   * sends a UDP datagram using wifi
   */
  udp.beginPacket(addr, port);
  udp.write(message);
  udp.endPacket();
  delay(15); //give time for UI processing to avoid filling buffer
}
