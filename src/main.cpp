#include <Arduino.h>
#include <ArduinoBLE.h>
#include <ESP32Servo.h>
#include "SparkFun_LIS331.h"
#include <Wire.h>
#include "defines.hpp"
#include "bluetooth.hpp"
#include <EEPROM.h>
#include <Preferences.h>
#include <imu.hpp>
#include <rotation.hpp>

u_int16_t PWM_width_us = 0; //PWM high width in Microseconds (us) (amount of time the signal is high)
u_int16_t DutyCycle = 0;

// float xMax = 0, yMax = 0, zMax = 0;
float maxRPM = 0;

int var = 0;

bool connected = false;

void setup() {
  Serial.begin(115200);
  
  pinMode(ESC1_PIN, OUTPUT);
  pinMode(ESC2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);


  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  prefs.begin("max", false);
  initImu();

  // set ground pins
  // pinMode(8, OUTPUT);
  // pinMode(20, OUTPUT);
  // digitalWrite(8, LOW);
  // digitalWrite(20, LOW);

  // TODO: can we raise this? 
  // Since we're sending max 2000µs pulses theoredically max should be 500hz but can also adjust
  // throttle on ESC side to read smaller pulses as max throttle, allowing even higher frequency?
  esc1.setPeriodHertz(50);
  esc1.attach(ESC1_PIN, MIN_PULSE, MAX_PULSE);
  esc2.setPeriodHertz(50);
  esc2.attach(ESC2_PIN, MIN_PULSE, MAX_PULSE);
  esc1.write(0);
  esc2.write(0);
  delay(2000);
  Serial.printf("%d, Started Stuff", var);
  // prefs.begin("maxG", false);
  // prefs.begin("yMax", false);
  // prefs.begin("zMax", false);
  
  maxRPM = prefs.getFloat("maxRPM", 0);
  
  printf("Initial maxRPM %f, maxG %f\n", maxRPM, maxG);
  // pinMode(LED_PIN, OUTPUT);
  // digitalWrite(LED_PIN, HIGH); // LED pin 

  // xTaskCreatePinnedToCore(
  //   bluetoothSetup,
  //   "bluetoothTask",
  //   20000,
  //   NULL,
  //   0,
  //   NULL,
  //   1     // core being run on, wireless is core 1
  // );
}

unsigned long rotationStart;

void loop() {
  var = 1;
  float G = readForce();
  
  double rpm = getRPM(G);
  // if (rpm > maxRPM)
  // {
  //   maxRPM = rpm;
  //   prefs.putFloat("maxRPM", rpm);
  //   prefs.end();
  //   prefs.begin("max");
  // }
  // Serial.printf("G: %f, max G:%f, max Real G:%f RPM: %f, maxRPM:%f\n", G, maxG, maxRealG, rpm, maxRPM);

  // if (rpm > 00) {
    // digitalWrite(LED_PIN, HIGH);
    manageRotation(rpm);
    // delay(4);
  // } else {
    // digitalWrite(LED_PIN, LOW);
    // delay(4);
  // }
}

