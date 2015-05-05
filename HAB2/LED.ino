
#define PIN_LED A7
#define LED_MAXTIME 60000 // Turn of LED after 1 minute

// Setup LED indicator
void led_init() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
}

// Blink LED indicator if system is healthy
void led_process(boolean status) {
  
  if (!status || now > LED_MAXTIME) {
    digitalWrite(PIN_LED, LOW);
  } else {
    digitalWrite(PIN_LED, (now % 1000) < 250 ? HIGH : LOW);
  }
}


