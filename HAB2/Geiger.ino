
#define GEIGER_PULSE_PIN  8
#define GEIGER_CPS_INVALID 65000 // max pulses per second
#define GEIGER_CPS_SIZE 100
volatile uint16_t interrupt_geiger_count = 0;
unsigned long geiger_count_timestamp = 0;
unsigned long geiger_count_duration = 0;
uint16_t geiger_count = 0;

struct geiger_cps_t {
  unsigned long timestamp;
  uint16_t count;
} geiger_cps[GEIGER_CPS_SIZE];
uint8_t geiger_cps_idx = 0;


boolean geiger_init() {
  for (uint8_t i = 0; i < GEIGER_CPS_SIZE; i++) geiger_cps[i].count = GEIGER_CPS_INVALID;
  geiger_count_timestamp = millis();
  
  pinMode(GEIGER_PULSE_PIN, INPUT);
  delay(50);
  attachInterrupt(GEIGER_PULSE_PIN, geiger_interrupt, RISING);

  return true;
}


void geiger_interrupt() {
  noInterrupts();
  if (interrupt_geiger_count < GEIGER_CPS_INVALID) interrupt_geiger_count++;
  interrupts();
}


void geiger_process() {
  unsigned long now = millis();
  
  if (geiger_count_timestamp + 1000UL > now) return; // measure once per second

  noInterrupts();
  geiger_count = interrupt_geiger_count;
  interrupt_geiger_count = 0;
  interrupts();

  geiger_cps[geiger_cps_idx].timestamp = geiger_count_timestamp;
  geiger_cps[geiger_cps_idx].count = geiger_count;
  geiger_cps_idx++;
  if (geiger_cps_idx == GEIGER_CPS_SIZE) geiger_cps_idx = 0;

  geiger_count_duration = now - geiger_count_timestamp;
  geiger_count_timestamp = now;
  
#ifdef DEBUG
  if (geiger_count_duration > 1200) {
    Serial.print("Warning: geiger counter duration = ");
    Serial.println(geiger_count_duration);
  }
#endif
}


boolean geiger_measure(uint16_t *cps, unsigned long *cps_duration, uint32_t *cpm, uint32_t *uShr) {
  unsigned long now = millis();
  unsigned long min_timstamp = millis();
  uint32_t cpm_count = 0;

  if (geiger_count_duration > 0) {
    *cps = geiger_count;
    *cps_duration = geiger_count_duration;
  } else {
    *cps = 0;
    *cps_duration = 0;
  }

  for (uint8_t i = 0; i < GEIGER_CPS_SIZE; i++) {
    if (geiger_cps[i].count == GEIGER_CPS_INVALID) continue;
    if (geiger_cps[i].timestamp + 60000UL < now) continue;
    cpm_count += geiger_cps[i].count;
    if (geiger_cps[i].timestamp < min_timstamp) min_timstamp = geiger_cps[i].timestamp;
  }

  if (min_timstamp + 59000 < now) {
    *cpm = cpm_count;
    *uShr = cpm_count * 57 / 100;
  } else {
    *cpm = 0;
    *uShr = 0;
  }

  return true;
}



