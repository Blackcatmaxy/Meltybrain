#pragma once
#include <Arduino.h>
#include "imu.hpp"
#include "bluetooth.hpp"
#include "defines.hpp"
#include "motors.hpp"

void manageRotation(double rpm);

unsigned long overflow_us = 0;
int rot_throttle = 0;

void manageRotation(double rpm)
{
    if (true)
    {
        int us = map(rot_throttle, 0, 255, MIN_PULSE, MAX_PULSE);
        writeMotorLeft(us);
        writeMotorRight(us);
    }

    unsigned long start = micros();
    unsigned long thisRotationUs = 0;
    unsigned long rotationIntervalUs = getRotationIntervalMs(rpm) * 1000 - overflow_us;
    while (thisRotationUs < rotationIntervalUs)
    {

        thisRotationUs = micros() - start;
        if (thisRotationUs < rotationIntervalUs / 8)
            digitalWrite(LED_PIN, HIGH);
        else
            digitalWrite(LED_PIN, LOW);
        // delayMicroseconds(10);
    }

    //TODO: NEVER LET THIS UNDERFLOW
    // overflow_us = rotationIntervalUs - thisRotationUs;Q
}