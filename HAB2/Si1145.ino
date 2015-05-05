
#define SI1145_ADDRESS 0x60
#define SI1145_PART_ID 0x45

#define SI1145_REG_PART_ID 0x00
#define SI1145_REG_RESET   0x01
#define SI1145_REG_HWKEY   0x07
#define SI1145_REG_UCOEFF0 0x13
#define SI1145_REG_UCOEFF1 0x14
#define SI1145_REG_UCOEFF2 0x15
#define SI1145_REG_UCOEFF3 0x16
#define SI1145_REG_PARAM_WR 0x17
#define SI1145_REG_COMMAND 0x18
#define SI1145_REG_VIS_DATA0 0x22
#define SI1145_REG_VIS_DATA1 0x23
#define SI1145_REG_IR_DATA0 0x24
#define SI1145_REG_IR_DATA1 0x25
#define SI1145_REG_UVINDEX0 0x2C
#define SI1145_REG_UVINDEX1 0x2D
#define SI1145_REG_PARAM_RD 0x2E

#define SI1145_PARAM_SET 0xA0
#define SI1145_PARAM_QUERY 0x80
#define SI1145_PARAM_VIS_ADC_MISC 0x12
#define SI1145_PARAM_IR_ADC_MISC 0x1F

#define SI1145_PARAM_CHLIST 0x01
#define SI1145_PARAM_CHLIST_UV 0x80
#define SI1145_PARAM_CHLIST_IR 0x20
#define SI1145_PARAM_CHLIST_VIS 0x10

#define SI1145_ALS_FORCE 0x06


boolean si1145_init() {
  uint8_t part_id;
  
  // Reset sensor
  if (!i2c_write8(SI1145_ADDRESS, SI1145_REG_COMMAND, SI1145_REG_RESET)) {
    Serial.println("Unable to reset Si1145");
    return false;
  }
  delay(10);

  // required by "3.2 initialization mode"
  if (!i2c_write8(SI1145_ADDRESS, SI1145_REG_HWKEY, 0x17)) {
    Serial.println("Unable to reset Si1145");
    return false;
  }
  
  if (!i2c_read8(SI1145_ADDRESS, SI1145_REG_PART_ID, &part_id)) return false;
  if (part_id != SI1145_PART_ID) {
    Serial.println("Unable to detect Si1145");
    return false;
  }

  // Enable UV measurements
  i2c_write8(SI1145_ADDRESS, SI1145_REG_UCOEFF0, 0x29);
  i2c_write8(SI1145_ADDRESS, SI1145_REG_UCOEFF1, 0x89);
  i2c_write8(SI1145_ADDRESS, SI1145_REG_UCOEFF2, 0x02);
  i2c_write8(SI1145_ADDRESS, SI1145_REG_UCOEFF3, 0x00);

  // Enable high signal range for IR light
  i2c_write8(SI1145_ADDRESS, SI1145_REG_PARAM_WR, 0x20);
  i2c_write8(SI1145_ADDRESS, SI1145_REG_COMMAND, SI1145_PARAM_SET | SI1145_PARAM_IR_ADC_MISC);

  // Enable high signal range for visible light
  i2c_write8(SI1145_ADDRESS, SI1145_REG_PARAM_WR, 0x20);
  i2c_write8(SI1145_ADDRESS, SI1145_REG_COMMAND, SI1145_PARAM_SET | SI1145_PARAM_VIS_ADC_MISC);

  // Enable UV, IR, and visible light measurements
  i2c_write8(SI1145_ADDRESS, SI1145_REG_PARAM_WR, SI1145_PARAM_CHLIST_UV | SI1145_PARAM_CHLIST_IR | SI1145_PARAM_CHLIST_VIS);
  i2c_write8(SI1145_ADDRESS, SI1145_REG_COMMAND, SI1145_PARAM_SET | SI1145_PARAM_CHLIST);

  return true;
}

// Returns measurements from Si1145 sensor
// UVI = UV-index in 0.01
// IR = ambient IR reading
// VIS = ambient light reading
void si1145_measure(int16_t *uvi, int16_t *ir, int16_t *vis) {
  uint16_t m_uvi, m_ir, m_vis;

  *uvi = INVALID_INT;
  *ir  = INVALID_INT;
  *vis = INVALID_INT;
  
  // Force a measurement
  if (!i2c_write8(SI1145_ADDRESS, SI1145_REG_COMMAND, SI1145_ALS_FORCE)) return;
  delay(5); // wait for measurement to complete
  
  // Read UV index measurement
  if (i2c_read16_lsb(SI1145_ADDRESS, SI1145_REG_UVINDEX0, &m_uvi)) {
    *uvi = m_uvi;
  }

  // Read IR light measurement
  if (i2c_read16_lsb(SI1145_ADDRESS, SI1145_REG_IR_DATA0, &m_ir)) {
    *ir = m_ir;
  }
  
  // Read visible light measurement
  if (i2c_read16_lsb(SI1145_ADDRESS, SI1145_REG_VIS_DATA0, &m_vis)) {
    *vis = m_vis;
  }
}





