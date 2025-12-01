#include <Arduino.h>
#include "UiCom.h"

void setup() {
  Serial.begin(115200);
  initUI();
}

void loop() {
  handleUI();
  delay(500); // Send telemetry every 5 seconds
}
