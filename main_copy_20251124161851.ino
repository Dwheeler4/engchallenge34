#include <Arduino.h>
#include "UiCom.h"
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  initUI();
}

void loop() {  
  sendSetpoints();
  delay(10);
  readMeasurements();

  // update UI
  handleUI();

  delay(500);
}
