
#define SDCARD_CS_PIN 10
#define SDCARD_FILENAME "HAB2.txt"

Sd2Card sd_card;
SdVolume sd_volume;
SdFile sd_root;

// Initializes SD card. Returns true/false.
boolean sd_init() {
  pinMode(SDCARD_CS_PIN, OUTPUT);

  if (!sd_card.init(SPI_HALF_SPEED, SDCARD_CS_PIN)) {
    Serial.println("SD card initialization failed");
    return false;
  }

  if (!sd_volume.init(sd_card)) {
    Serial.println("Could not find FAT16/FAT32 partition on SD card");
    return false;
  }

  if (!sd_root.openRoot(sd_volume)) {
    Serial.println("Could not open root directory on SD card");
    return false;
  }

  return true;
}

// Writes a string into a file on SD card. Returns true/false.
boolean sd_write(char *s) {
  SdFile sd_file;
  size_t ret;
  
  if (!sd_file.open(sd_root, SDCARD_FILENAME, O_CREAT | O_APPEND | O_WRITE)) {
    Serial.println("Could not open file on SD card");
    return false;
  }

  ret = sd_file.write(s);
  sd_file.close();

  if (!ret) {
    Serial.println("Could not write to file on SD card");
    return false;
  }

  return true;
}





