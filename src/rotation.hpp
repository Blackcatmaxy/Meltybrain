#pragma once

#include <Arduino.h>
#include "imus.hpp"
#include "bluetooth.hpp"
#include "defines.hpp"
#include "motors.hpp"

void manageRotation(double rpm);

unsigned long overflow_us = 0;
//int rot_throttle = 0;
Throttles throttles;

int rotationThrottle;
int forwardThrottle;

unsigned long start;
unsigned long thisRotationUs = 0;
long rotationIntervalUs;

unsigned long motorPhase1Start;
unsigned long motorPhase1Stop;
unsigned long motorPhase2Start;
unsigned long motorPhase2Stop;

unsigned long ledLength = rotationIntervalUs / 8;
unsigned long ledStart = (rotationIntervalUs / 2) - ledLength / 2;
unsigned long ledEnd = (rotationIntervalUs / 2) + ledLength / 2;

void manageRotation(double rpm, bool reset) {
    if (reset || thisRotationUs == 0)
    {
        rotationThrottle = map(throttles.rotation, 0, 255, 0, 999);
        forwardThrottle = map(abs(throttles.forward), 0, 130, 0, 999);

        start = micros();
        rotationIntervalUs = (getRotationIntervalMs(rpm) * 1000) - overflow_us;

        motorPhase1Start = rotationIntervalUs / 4;
        motorPhase1Stop = rotationIntervalUs - motorPhase1Start;
        motorPhase2Start = motorPhase1Stop;
        motorPhase2Stop = motorPhase1Start;

        ledLength = rotationIntervalUs / 8;
        ledStart = (rotationIntervalUs / 2) - ledLength / 2;
        ledEnd = (rotationIntervalUs / 2) + ledLength / 2;
    }

// TODO: This isn't right, motor 2 never starts since it's start time is at end of interval - but it works?

//    unsigned long motorPhase1Start = rotationIntervalUs / 2;
//    unsigned long motorPhase1Stop = rotationIntervalUs;
//    unsigned long motorPhase2Start = motorPhase1Stop;
//    unsigned long motorPhase2Stop = motorPhase1Start;

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

    // while (thisRotationUs < rotationIntervalUs) {
        // LED control
        if (thisRotationUs > ledStart && thisRotationUs < ledEnd) {
            digitalWrite(LEDT_PIN, HIGH);
            digitalWrite(LEDB_PIN, LOW);
        }
        else {
            digitalWrite(LEDT_PIN, LOW);
            digitalWrite(LEDB_PIN, HIGH);
        }
        if (abs(throttles.forward) - 10 < 0) {
            // no translation
            writeLeft(rotationThrottle);
            writeRight(rotationThrottle);
        } else {
            bool forwards = throttles.forward > 0;
            // Motor 1 control
            if (forwards) {
                if ((motorPhase1Start <= thisRotationUs && motorPhase1Stop >= thisRotationUs))
                    writeLeft(rotationThrottle); //on
                else
                    writeLeft(rotationThrottle - forwardThrottle);

                // Motor 2 control
                if ((motorPhase2Start <= thisRotationUs && motorPhase2Stop >= thisRotationUs))
                    writeRight(rotationThrottle); //on
                else
                    writeRight(rotationThrottle - forwardThrottle);
            }

            if (!forwards) {
                if ((motorPhase1Start <= thisRotationUs && motorPhase1Stop >= thisRotationUs))
                    writeRight(rotationThrottle); //on
                else
                    writeRight(rotationThrottle - forwardThrottle);

                // Motor 2 control
                if ((motorPhase2Start <= thisRotationUs && motorPhase2Stop >= thisRotationUs))
                    writeLeft(rotationThrottle); //on
                else
                    writeLeft(rotationThrottle - forwardThrottle);
            }
        }


        thisRotationUs = micros() - start;
    // }

    if (thisRotationUs > rotationIntervalUs)
    {
        
    //TODO: NEVER LET THIS UNDERFLOW
    overflow_us = thisRotationUs - rotationIntervalUs;
#ifdef SERIAL_TELEM
//       Serial.printf("rotation theory %ld, real %ld, overflow %ld\n", rotationIntervalUs, thisRotationUs, overflow_us);
#endif
        thisRotationUs = 0;
    }
}