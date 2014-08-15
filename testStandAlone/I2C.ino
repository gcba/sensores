const uint8_t IMUAddress = 0x68; 
const uint16_t I2C_TIMEOUT = 1000;
uint8_t i2cWrite(uint8_t registerAddress, uint8_t data, bool sendStop) {
  return i2cWrite(registerAddress, &data, 1, sendStop); // Devuelve 0 si fuciona
}

uint8_t i2cWrite(uint8_t registerAddress, uint8_t *data, uint8_t length, bool sendStop) {
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.write(data, length);
  uint8_t rcode = Wire.endTransmission(sendStop); // Devuelve 0 si fuciona
  if (rcode) {
    Serial.print(F("Escritura fallo: "));
    Serial.println(rcode);
  }
  return rcode;
}

uint8_t i2cRead(uint8_t registerAddress, uint8_t *data, uint8_t nbytes) {
  uint32_t timeOutTimer;
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  uint8_t rcode = Wire.endTransmission(false); // No libera el bus
  if (rcode) {
    Serial.print(F("Lectura falla: "));
    Serial.println(rcode);
    return rcode;
  }
  Wire.requestFrom(IMUAddress, nbytes, (uint8_t)true); // Envia repetidamente un start y libera el bus despues de leer
  for (uint8_t i = 0; i < nbytes; i++) {
    if (Wire.available())
      data[i] = Wire.read();
    else {
      timeOutTimer = micros();
      while (((micros() - timeOutTimer) < I2C_TIMEOUT) && !Wire.available());
      if (Wire.available())
        data[i] = Wire.read();
      else {
        Serial.println(F("Timeout"));
        return 5; // Este error solo molesta si hay una diferencia de tiempo X donde el IMU quiere apropiarse del bus para escribir y este todabia esta en lectura del ACK
      }
    }
  }
  return 0; // Exito
}
