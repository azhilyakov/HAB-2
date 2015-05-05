
// I2C helper functions

#define I2C_TIMEOUT 1000 // 1ms

// Initialize I2C interface
boolean i2c_init() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_100);
  return true;
}


// Print error message
void i2c_error(const char *s) {
  int e = Wire.getError();
  Serial.print(s);
  Serial.print(": ");
  Serial.println(e == 0 ? "no error" : e == 1 ? "data too long" : e == 2 ? "recv addr NACK" : e == 3 ? "recv data NACK" : "other error");
}


// Write a byte to address
boolean i2c_write8(uint8_t addr, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(val);
  if (Wire.endTransmission(I2C_STOP, I2C_TIMEOUT) != 0) {
    i2c_error("i2c_write8: write error");
    return false;
  }
  return true;
}


// Write a byte to address/register
boolean i2c_write8(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  if (Wire.endTransmission(I2C_STOP, I2C_TIMEOUT) != 0) {
    i2c_error("i2c_write8: write error");
    return false;
  }
  return true;
}


// Request a variable number of bytes from address/register
boolean i2c_request_bytes(uint8_t addr, uint8_t reg, uint8_t num_bytes) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(I2C_STOP, I2C_TIMEOUT) != 0) {
    i2c_error("i2c_request_bytes: endTx error");
    return false;
  }
  if (Wire.requestFrom(addr, num_bytes, I2C_STOP, I2C_TIMEOUT) != num_bytes) {
    i2c_error("i2c_request_bytes: reqFrom error");
    return false;
  }

  return true;
}


// Request a byte from address/register
boolean i2c_read8(uint8_t addr, uint8_t reg, uint8_t *val) {
  if (!i2c_request_bytes(addr, reg, 1)) return false;
  *val = Wire.read();
  return true;
}


// Request two bytes from address/register - LSB first
boolean i2c_read16_lsb(uint8_t addr, uint8_t reg, uint16_t *val) {
  if (!i2c_request_bytes(addr, reg, 2)) return false;
  *val = Wire.read();
  *val |= Wire.read() << 8;
  return true;
}


// Request two bytes from address/register - MSB first
boolean i2c_read16_msb(uint8_t addr, uint8_t reg, uint16_t *val) {
  if (!i2c_request_bytes(addr, reg, 2)) return false;
  *val = Wire.read() << 8;
  *val |= Wire.read();
  return true;
}


// Request two bytes from address/register - LSB first
boolean i2c_read24_lsb(uint8_t addr, uint8_t reg, uint32_t *val) {
  if (!i2c_request_bytes(addr, reg, 3)) return false;
  *val  = Wire.read();
  *val |= Wire.read() << 8;
  *val |= Wire.read() << 16;
  return true;
}


// Request two bytes from address/register - MSB first
boolean i2c_read24_msb(uint8_t addr, uint8_t reg, uint32_t *val) {
  if (!i2c_request_bytes(addr, reg, 3)) return false;
  *val  = Wire.read() << 16;
  *val |= Wire.read() << 8;
  *val |= Wire.read();
  return true;
}


