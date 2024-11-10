#include <Arduino.h>
#include <Wire.h>
#include "defines.hpp"
#include "bluetooth.hpp"
#include <Preferences.h>
#include <imu.hpp>
#include "rotation.hpp"
#include "motors.hpp"

// float xMax = 0, yMax = 0, zMax = 0;
float maxRPM = 0;
bool connected = false;

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    prefs.begin("max", false);
    initImu();
    initMotors();

    delay(2000);
    // prefs.begin("maxG", false);
    // prefs.begin("yMax", false);
    // prefs.begin("zMax", false);

    maxRPM = prefs.getFloat("maxRPM", 0);

    printf("Initial maxRPM %f, maxG %f\n", maxRPM, maxG);
    // pinMode(LED_PIN, OUTPUT);
    // digitalWrite(LED_PIN, HIGH); // LED pin
    xTaskCreatePinnedToCore(
            bluetoothSetup,
            "bluetoothTask",
            200000,
            (void*)&throttles,
            0,
            NULL,
            0 // core being run on, wireless is core 1
    );
}

void loop()
{
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
    if (rpm < 60)
        rpm = 60;
    manageRotation(rpm);
//    Serial.println("Loop");
//    delay(10);
}
