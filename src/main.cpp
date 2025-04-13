#include <Arduino.h>
#include <Wire.h>
#include "defines.hpp"
#include "bluetooth.hpp"
#include <Preferences.h>
#include <imu.hpp>
#include <SPI.h>

#include "rotation.hpp"
#include "motors.hpp"

// float xMax = 0, yMax = 0, zMax = 0;
float maxRPM = 0;
bool connected = false;

[[noreturn]] void controlLoop(void *pvParams);
void setup()
{
    Serial.begin(115200);

    pinMode(LEDT_PIN, OUTPUT);
    pinMode(LEDB_PIN, OUTPUT);
    digitalWrite(LEDT_PIN, HIGH);

    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS0_PIN);
    // Wire.begin(SDA_PIN, SCL_PIN);
    // Wire.setClock(400000);

    prefs.begin("max", false);
    initImu();
    initMotors();

    delay(2000);
    // prefs.begin("maxG", false);
    // prefs.begin("yMax", false);
    // prefs.begin("zMax", false);

    // maxRPM = prefs.getFloat("maxRPM", 0);
    //
    // printf("Initial maxRPM %f, maxG %f\n", maxRPM, maxG);
    digitalWrite(LEDT_PIN, HIGH); // LED pin
    xTaskCreatePinnedToCore(
            bluetoothSetup,
            "bluetoothTask",
            200000,
            (void*)&throttles,
            0,
            nullptr,
            1 // core being run on, wireless is core 1
    );

    xTaskCreatePinnedToCore(
            controlLoop,
            "controlTask999",
            50000,
            nullptr,
            0,
            nullptr,
            0 // core being run on, wireless is core 1
    );
    Serial.print("Setup: Executing on core ");
    Serial.println(xPortGetCoreID());
}

int count = 0;
bool reset = false;

[[noreturn]]
void controlLoop(void *pvParams)
{
    Serial.print("Control: Executing on core ");
    Serial.println(xPortGetCoreID());
    while (true) {
        double rpm;
        count++;
        float G;
        if (count > 1000) {
            // G = readForce();

            // rpm = getRPM(G);
            rpm = getDoubleRPM();
            Serial.printf("RPM: %f\n", rpm);
            count = 0;
            // Serial.printf("G: %f, max G:%f, max Real G:%f RPM: %f, maxRPM:%f\n", G, maxG, maxRealG, rpm, maxRPM);
        }
        // if (rpm > maxRPM)
        // {
        //   maxRPM = rpm;
        //   prefs.putFloat("maxRPM", rpm);
        //   prefs.end();
        //   prefs.begin("max");
        // }


        // if (rpm > 00) {
        // digitalWrite(LED_PIN, HIGH);
        if (throttles.rotation) {
            // Serial.println("melty");
            if (rpm < 60)
                rpm = 60;
            manageRotation(rpm, reset);
            reset = false;
        } else {
            reset = true;
            int rightThrottle = map(throttles.rightForward, -128, 128, -999, 999) * -1;
            int leftThrottle = map(throttles.forward, -128, 128, -999, 999);
    #ifdef SERIAL_TELEM
    //        Serial.printf("left: %d right: %d\n", leftThrottle, rightThrottle);
    #endif
            tankDrive(leftThrottle, rightThrottle);
            delayMicroseconds(50);
        }
    //    Serial.println("Loop");
    #ifdef SERIAL_TELEM
        delay(1);

    #endif
    }
}

void loop()
{
    delay(1000);
}
