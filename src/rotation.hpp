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
    int forwardThrottleUs = map(abs(throttles.forward), 0, 130, MIN_PULSE, MAX_PULSE) - MIN_PULSE;

//    Serial.printf("%d - %d\n", rotationThrottleUs, forwardThrottleUs);
//    delay(1);

    unsigned long start = micros();
    unsigned long thisRotationUs = 0;
    long rotationIntervalUs = (getRotationIntervalMs(rpm) * 1000) - overflow_us;

//    unsigned long motorPhase1Start = rotationIntervalUs / 4;
//    unsigned long motorPhase1Stop = rotationIntervalUs - motorPhase1Start;
//    unsigned long motorPhase2Start = motorPhase1Stop;
//    unsigned long motorPhase2Stop = motorPhase1Start;
// TODO: This isn't right, motor 2 never starts since it's start time is at end of interval - but it works?
    unsigned long motorPhase1Start = rotationIntervalUs / 2;
    unsigned long motorPhase1Stop = rotationIntervalUs;
    unsigned long motorPhase2Start = motorPhase1Stop;
    unsigned long motorPhase2Stop = motorPhase1Start;

    static unsigned long cycle_count = 0;
    cycle_count++;

    //hacky way to do this since I can't find how openmelt does this, likely to cause problems when translating at same time?
    //would be more reliable to add delay at the end to move with rotation but that can't counteract accumulating error
    //and can't do the opposite of a delay without time travel? Could do delay when > 10 but lets keep it symmetric
    // --- or this fine because we calculate intervals above?
    if (throttles.side > 10 || throttles.side < -10) {
        float multiplier = throttles.side / (128.0f * 16);
        rotationIntervalUs += rotationIntervalUs * multiplier;
    }

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
            if (forwards) {
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
    overflow_us = thisRotationUs - rotationIntervalUs;
//    Serial.printf("rotation theory %ld, real %ld, overflow %ld\n", rotationIntervalUs, thisRotationUs, overflow_us);
}