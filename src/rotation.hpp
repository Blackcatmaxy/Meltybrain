#pragma once

#include <Arduino.h>
#include "imu.hpp"
#include "bluetooth.hpp"
#include "defines.hpp"
#include "motors.hpp"

void manageRotation(double rpm);

unsigned long overflow_us = 0;
//int rot_throttle = 0;
Throttles throttles;

void manageRotation(double rpm) {
    int rotationThrottleUs = map(throttles.rotation, 0, 255, MIN_PULSE, MAX_PULSE);
    int forwardThrottleUs = map(abs(throttles.forward), 0, 127, MIN_PULSE, MAX_PULSE * 3 / 4) - MIN_PULSE;

//    Serial.printf("%d - %d\n", rotationThrottleUs, forwardThrottleUs);
//    delay(1);

    unsigned long start = micros();
    unsigned long thisRotationUs = 0;
    long rotationIntervalUs = getRotationIntervalMs(rpm) * 1000 - overflow_us;

    unsigned long motorPhase1Start = rotationIntervalUs / 4;
    unsigned long motorPhase1Stop = rotationIntervalUs - motorPhase1Start;
    unsigned long motorPhase2Start = motorPhase1Stop;
    unsigned long motorPhase2Stop = motorPhase1Start;

    static unsigned long cycle_count = 0;
    cycle_count++;

    while (thisRotationUs < rotationIntervalUs) {
        // LED control
        if (thisRotationUs < rotationIntervalUs / 8)
            digitalWrite(LED_PIN, HIGH);
        else
            digitalWrite(LED_PIN, LOW);


        if (abs(throttles.forward) - 10 < 0) {
            // no translation
            writeMotorLeft(rotationThrottleUs);
            writeMotorRight(rotationThrottleUs);
        } else {
            bool forwards = throttles.forward > 0;
            // Motor 1 control
            if (forwards && cycle_count % 2 == 0) {
                if ((motorPhase1Start <= thisRotationUs && motorPhase1Stop >= thisRotationUs))
                    writeMotorLeft(rotationThrottleUs); //on
                else
                    writeMotorLeft(rotationThrottleUs - forwardThrottleUs);

                // Motor 2 control
                if ((motorPhase2Start <= thisRotationUs && motorPhase2Stop >= thisRotationUs))
                    writeMotorRight(rotationThrottleUs); //on
                else
                    writeMotorRight(rotationThrottleUs - forwardThrottleUs);
            }

            if (!forwards) {
                if ((motorPhase1Start <= thisRotationUs && motorPhase1Stop >= thisRotationUs))
                    writeMotorRight(rotationThrottleUs); //on
                else
                    writeMotorRight(rotationThrottleUs - forwardThrottleUs);

                // Motor 2 control
                if ((motorPhase2Start <= thisRotationUs && motorPhase2Stop >= thisRotationUs))
                    writeMotorLeft(rotationThrottleUs); //on
                else
                    writeMotorLeft(rotationThrottleUs - forwardThrottleUs);
            }
        }


        thisRotationUs = micros() - start;
    }

    //TODO: NEVER LET THIS UNDERFLOW
//     overflow_us = rotationIntervalUs - thisRotationUs;


}