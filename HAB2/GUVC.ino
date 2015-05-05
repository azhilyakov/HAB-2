
#define GUVC_PIN A0

boolean guvc_init() {
  pinMode(GUVC_PIN, INPUT);
  return true; 
}


boolean guvc_measure(uint16_t *v, uint16_t *uv) {
  uint32_t adc_reading = analogRead(GUVC_PIN);
  uint32_t uvc_calc = adc_reading * 73 / 4096;
  
  *v = adc_reading;
  *uv = uvc_calc;
  
  return true;
}


