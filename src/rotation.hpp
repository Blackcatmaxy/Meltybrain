#pragma once
#include <Arduino.h>
#include <imu.hpp>
#include <bluetooth.hpp>

void manageRotation(double rpm);

void manageRotation(double rpm) {
  
  unsigned long start = micros();
  unsigned long thisRotationUs = 0;
  unsigned long rotationIntervalUs = getRotationIntervalMs(rpm) * 1000;
  while (thisRotationUs < rotationIntervalUs) {
    thisRotationUs = micros() - start;
    if (thisRotationUs < rotationIntervalUs / 8) {
      digitalWrite(LED_PIN, HIGH);
    } else
      digitalWrite(LED_PIN, LOW);
    // delayMicroseconds(10);
  }
}