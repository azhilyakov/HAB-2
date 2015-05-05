
#define MS5607_ADDRESS 0x76
#define MS5607_PROM_ADDR 0xA0
#define MS5607_CMD_READ_ADC 0x00
#define MS5607_CMD_RESET 0x1E
#define MS5607_CMD_D1_OSR_4096 0x48
#define MS5607_CMD_D2_OSR_4096 0x58

uint16_t ms5607_prom[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

#define MS5607_C1 ((double)(ms5607_prom[1]))
#define MS5607_C2 ((double)(ms5607_prom[2]))
#define MS5607_C3 ((double)(ms5607_prom[3]))
#define MS5607_C4 ((double)(ms5607_prom[4]))
#define MS5607_C5 ((double)(ms5607_prom[5]))
#define MS5607_C6 ((double)(ms5607_prom[6]))

// CRC calculation - from AN.520
uint8_t ms5607_prom_crc(uint16_t n_prom[]) {
  int cnt; // simple counter
  unsigned int n_rem; // crc reminder
  unsigned int crc_read; // original value of the crc
  unsigned char n_bit;

  n_rem = 0x00;
  crc_read = n_prom[7]; //save read CRC
  n_prom[7] = (0xFF00 & (n_prom[7])); // CRC byte is replaced by 0

  for (cnt = 0; cnt < 16; cnt++) { // operation is performed on bytes
    // choose LSB or MSB
    if (cnt%2==1) n_rem ^= (uint8_t) ((n_prom[cnt>>1]) & 0x00FF);
    else n_rem ^= (uint8_t) (n_prom[cnt>>1]>>8);
    for (n_bit = 8; n_bit > 0; n_bit--) {
      if (n_rem & (0x8000)) {
        n_rem = (n_rem << 1) ^ 0x3000;
      } else {
        n_rem = (n_rem << 1);
      }
    }
  }

  n_rem = (0x000F & (n_rem >> 12)); // final 4-bit reminder is CRC code
  n_prom[7] = crc_read; // restore the crc_read to its original place
  return (n_rem ^ 0x0);
}


boolean ms5607_init() {

  // Reset sensor
  if (!i2c_write8(MS5607_ADDRESS, MS5607_CMD_RESET)) {
    Serial.println("Unable to reset MS5607");
    return false;
  }
  delay(10);

  // Read calibration coefficients
  for (int i = 0; i < 8; i++) {
    uint8_t c_addr = MS5607_PROM_ADDR + (i << 1);
    if (!i2c_read16_msb(MS5607_ADDRESS, c_addr, &ms5607_prom[i])) return false;
  }
 
  uint8_t crc_prom = ms5607_prom[7] & 0x0F;
  uint8_t crc_calc = ms5607_prom_crc(ms5607_prom);
  if (crc_prom != crc_calc) {
    Serial.print("MS5607 error - PROM CRC mismatch: PROM CRC = ");
    Serial.print(crc_prom);
    Serial.print(", calculated CRC = ");
    Serial.println(crc_calc);
    return false;
  }

  Serial.print("MS5607 PROM:");
  for (int i = 1; i < 7; i++) {
    Serial.print(" ");
    Serial.print(ms5607_prom[i]);
  }
  Serial.println("");
  // MS5607 PROM: 35286 34201 22467 22038 32474 28405

  return true;
}

// Returnes measurements from MS5607 sensor
// raw_D1 = raw uncompensated temperature
// raw_d2 = raw uncompensated pressure
// temp = temperature in 0.01C
// pressure = barometric pressure in Pa
// altitude = altitude in meters
void ms5607_measure(uint32_t *raw_D1, uint32_t *raw_D2, int32_t *temp, int32_t *pressure, int32_t *altitude) {

  *raw_D1   = 0;
  *raw_D2   = 0;
  *temp     = INVALID_INT;
  *pressure = INVALID_INT;
  *altitude = INVALID_INT;

  // Read uncompensated pressure  
  if (!i2c_write8(MS5607_ADDRESS, MS5607_CMD_D1_OSR_4096)) return;
  delay(10);
  if (!i2c_read24_msb(MS5607_ADDRESS, MS5607_CMD_READ_ADC, raw_D1)) return;
  if (*raw_D1 == 0) return;
  double D1 = *raw_D1;
  
  // Read uncompensated temperature
  if (!i2c_write8(MS5607_ADDRESS, MS5607_CMD_D2_OSR_4096)) return;
  delay(10);
  if (!i2c_read24_msb(MS5607_ADDRESS, MS5607_CMD_READ_ADC, raw_D2)) return;
  if (*raw_D2 == 0) return;
  double D2 = *raw_D2;

  double dT = D2 - MS5607_C5 * 256.0;
  double TEMP = 2000.0 + dT * MS5607_C6 / 8388608.0; // Temperature in 0.01C
  double OFF = MS5607_C2 * 131072.0 + MS5607_C4 * dT / 64.0;
  double SENS = MS5607_C1 * 65536.0 + MS5607_C3 * dT / 128.0;

  // Second order temperature compensation
  if (TEMP < 2000.0) {
    double T2 = dT * dT / 2147483648.0;
    double OFF2 = 61 * (TEMP - 2000.0) * (TEMP - 2000.0) / 16.0;
    double SENS2 = 2 * (TEMP - 2000.0) * (TEMP - 2000.0);
    if (TEMP < -1500.0) {
      OFF2  += 15 * (TEMP + 1500.0) * (TEMP + 1500.0);
      SENS2 += 8 * (TEMP + 1500.0) * (TEMP + 1500.0);
    }
    
    TEMP = TEMP - T2;
    OFF  = OFF - OFF2;
    SENS = SENS - SENS2;
  }

  double P = ((D1 * SENS / 2097152.0 - OFF) / 32768.0); // in 0.01mbar = 1Pa
  
  double A = 44330.0 * ( 1.0 - pow((P / 101325.0), 0.190295)); // in meters

  *temp = TEMP;
  *pressure = P;
  *altitude = A;
  
}



