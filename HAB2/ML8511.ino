
#define ML8511_PIN A6
#define ML8511_VOLT_MIN (4096 / 2048 * 990)    // 0.99V
#define ML8511_UV_MIN   (0)                   //  0 mW/cm2
#define ML8511_VOLT_MAX (4096 / 2048 * 2800)  // 2.8V
#define ML8511_UV_MAX   (1500)                // 15 mW/cm2


// Initializes ML8511 sensor
bool ml8511_init() {
  pinMode(ML8511_PIN, INPUT);
  return true;
}


int32_t adc_map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


// This function returns ML8511 UV measurement in 0.01 mW/cm2
boolean ml8511_measure(uint16_t *v, uint16_t *uv) {
  int32_t adc_reading = analogRead(ML8511_PIN);
  // Assume ADC in 12-bit preceision
  int32_t uv_calc = adc_map(adc_reading, ML8511_VOLT_MIN, ML8511_VOLT_MAX, 0, 1500);
  
  *v = adc_reading;
  *uv = uv_calc > 0 ? uv_calc : 0;
  
  return true;
}


