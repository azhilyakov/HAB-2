
#define VOLT_PIN_BATTERY A8
#define VOLT_PIN_RAIL A9

boolean volt_init() {
  pinMode(VOLT_PIN_BATTERY, INPUT);
  pinMode(VOLT_PIN_RAIL, INPUT);
  return true;
}


// This function measures battery and 3.3V rail voltages in 0.01V
boolean volt_measure(uint16_t *v_battery, uint16_t *v_rail) {

  uint32_t vb = analogRead(VOLT_PIN_BATTERY);
  uint32_t vr = analogRead(VOLT_PIN_RAIL);
  
  *v_battery = vb * 10 / 45; // coeff=450
  *v_rail = vr * 10 / 43; // coeff=430
  
  return true;
}



