#define DEBUG 9
#undef DEBUG_LCD

#include <SPI.h>
#include <SD.h>
#include <i2c_t3.h>

//#ifdef DEBUG_LCD
//#include <LiquidCrystal.h>
//LiquidCrystal lcd(0, 1, 2, 5, 6, 7);
//uint8_t lcd_disp = 0;
//#endif

#define INVALID_INT -32000
#define INVALID_FLOAT -1000000.0
#define MEASUREMENT_INTERVAL 1000

unsigned long now = 0;
unsigned long last_measurement = 0;
boolean health = true;

#define STR_LEN 1024
char str[STR_LEN];

void scan_i2c() {
  
  Serial.println("Scanning i2c bus...");
  for(byte target = 1; target <= 0x7F; target++) {
    Wire.beginTransmission(target);
    Wire.endTransmission();
    if (Wire.status() == I2C_WAITING) {
      Serial.print("Addr 0x");
      Serial.print(target, HEX);
      Serial.println(" OK");
    }
  }
}

// Initializes Teensy ADC
void adc_init() {
  // Use external voltage reference
  analogReference(EXTERNAL);
  // ADC readings will be in range 0-4096
  analogReadRes(12);
  // Average over 3 analog readings
  analogReadAveraging(4);
  delay(50);
}


void setup() {
  
#ifdef DEBUG_LCD
  lcd.begin(16, 2);
#endif
  
  led_init();
  adc_init();
  
  health &= i2c_init();
  health &= sd_init();
  health &= si1145_init();
  health &= ms5607_init();
  health &= ml8511_init();
  health &= guvc_init();
  health &= geiger_init();
  health &= camera_init();
  health &= volt_init();
}


void loop() {
  int16_t uvi; // UV index in 0.01
  int16_t ir;  // IR reading
  int16_t vis; // Visual light
  uint32_t raw_temp;  // Raw temperature from MS5607
  uint32_t raw_press; // Raw pressure from MS5607
  int32_t temp;  // Temperature in 0.01C
  int32_t press; // Pressure in Pa
  int32_t alt;   // Altitude in meters
  uint16_t uv_adc; // Analog UV reading from ML8511
  uint16_t uv_int; // UV intensity in 0.01 mW/cm2
  uint16_t cps;  // Geiger pulses per second
  unsigned long cps_int; // Length of last measurement interval
  uint32_t cpm;  // Geiger pulses per minute
  uint32_t uSv_hr; // In 0.01 uSv/hr
  uint16_t vb;   // Battery voltage in 0.01V
  uint16_t vr;   // Rail voltage in 0.01V
  uint16_t uvc_adc;  // Analog UV reading from GUVC-T21GH
  uint16_t uvc_int;  // UV-C intensity in 0.01 mW/cm2

  now = millis();

  led_process(health);
  geiger_process();
  camera_process();

  if (now < last_measurement + MEASUREMENT_INTERVAL) return;
  last_measurement = now;

  si1145_measure(&uvi, &ir, &vis);
#ifdef DEBUG
  Serial.print("SI1145 UV=");
  Serial.print(uvi);
  Serial.print(" IR=");
  Serial.print(ir);
  Serial.print(" VIS=");
  Serial.println(vis);
#endif

  ms5607_measure(&raw_temp, &raw_press, &temp, &press, &alt);
#ifdef DEBUG
  Serial.print("MS5607 TEMP=");
  Serial.print(temp);
  Serial.print(" PRESS=");
  Serial.print(press);
  Serial.print(" ALT=");
  Serial.println(alt);
#endif

  ml8511_measure(&uv_adc, &uv_int);
#ifdef DEBUG
  Serial.print("ML8511 ADC=");
  Serial.print(uv_adc);
  Serial.print(" UV=");
  Serial.println(uv_int);
#endif

  geiger_measure(&cps, &cps_int, &cpm, &uSv_hr);
#ifdef DEBUG
  Serial.print("GEIGER CPS=");
  Serial.print(cps);
  Serial.print(" INT=");
  Serial.print(cps_int);
  Serial.print(" CPM=");
  Serial.print(cpm);
  Serial.print(" uSv/hr=");
  Serial.println(uSv_hr);
#endif

  guvc_measure(&uvc_adc, &uvc_int);
#ifdef DEBUG
  Serial.print("GUVC ADC=");
  Serial.print(uvc_adc);
  Serial.print(" UVC=");
  Serial.println(uvc_int);
#endif

  volt_measure(&vb, &vr);
#ifdef DEBUG
  Serial.print("BATT=");
  Serial.print(vb);
  Serial.print(" RAIL=");
  Serial.println(vr);
#endif


  snprintf(str, STR_LEN, "%lu,SI1145,%d,%d,%d,MS5607,%lu,%lu,%ld,%ld,%ld,ML8511,%u,%u,GUVC,%u,%u,GEIGER,%u,%lu,%lu,%lu,VOLT,%u,%u\r\n", now, uvi, ir, vis, raw_temp, raw_press, temp, press, alt, uv_adc, uv_int, uvc_adc, uvc_int, cps, cps_int, cpm, uSv_hr, vb, vr);
  sd_write(str);

#ifdef DEBUG
  Serial.print(str);
#endif


#ifdef DEBUG_LCD

  switch (lcd_disp) {

    case 1:
      snprintf(str, STR_LEN, "UV-AB %u.%02u mW/cm", uv_int/100, uv_int%100);
      break; 
  

    case 2:
      snprintf(str, STR_LEN, "UV-C %u.%02u mW/cm", uvc_int/100, uvc_int%100);
      break; 
  
    case 3:
      snprintf(str, STR_LEN, "UV-Index %d.%02d", uvi/100, uvi%100);
      break; 

    case 4:
      snprintf(str, STR_LEN, "IR %d", ir);
      break; 

    case 5:
      snprintf(str, STR_LEN, "Visible %d", vis);
      break; 

    case 6:
      snprintf(str, STR_LEN, "X-ray %u CPS", cps);
      break; 
  
    case 7:
      snprintf(str, STR_LEN, "X-ray %lu CPM", cpm);
      break; 
  
    case 8:
      snprintf(str, STR_LEN, "Temp %ld.%02ld C", temp/100, temp%100);
      break; 

    case 9:
      snprintf(str, STR_LEN, "Alt %ld m", alt);
      break; 

    case 10:
      snprintf(str, STR_LEN, "Batt %u.%02u V", vb/100, vb%100);
      break; 

    case 11:
      snprintf(str, STR_LEN, "Rail %u.%02u V", vr/100, vr%100);
      break; 

  default:
      lcd_disp = 0;
      snprintf(str, STR_LEN, "HAB-2 test");
      break; 
  }

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(str);

  lcd_disp++;
  
#endif

}



