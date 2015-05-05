
#define CAMERA_SHUTTER_PIN 9
#define CAMERA_TRIGGER_INTERVAL 5000 // trigger camera every 5 seconds
#define CAMERA_SHUTTER_HOLD 250      // press shutter button for 1/4 of a second
unsigned long camera_last_trigger_timestamp = 3000;
boolean camera_on = false;

boolean camera_init() {
  pinMode(CAMERA_SHUTTER_PIN, OUTPUT);
  delay(50);
  digitalWrite(CAMERA_SHUTTER_PIN, LOW);
  return true;
}


void camera_process() {
  unsigned long now = millis();
  
  if (camera_on == false) {
    if (camera_last_trigger_timestamp + CAMERA_TRIGGER_INTERVAL < now) {
      digitalWrite(CAMERA_SHUTTER_PIN, HIGH);
      camera_on = true;
      camera_last_trigger_timestamp = now;
      Serial.println("Camera ON");
    }

  } else { // camera_on = true
    if (camera_last_trigger_timestamp + CAMERA_SHUTTER_HOLD < now) {
      digitalWrite(CAMERA_SHUTTER_PIN, LOW);
      camera_on = false;
      Serial.println("Camera OFF");
    }
  }

}


