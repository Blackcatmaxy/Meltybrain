#pragma once
#include <Arduino.h>
#include <defines.hpp>
#include <ESP32Servo.h>
#include "driver/rmt.h"

Servo esc1, esc2;

void initMotors();
void writeMotorLeft(int us);
void writeMotorRight(int us);

void initMotors()
{
    pinMode(ESC1_PIN, OUTPUT);
    pinMode(ESC2_PIN, OUTPUT);

    // TODO: can we raise this?
    // Since we're sending max 2000µs pulses theoredically max should be 500hz but can also adjust
    // throttle on ESC side to read smaller pulses as max throttle, allowing even higher frequency?
    esc1.setPeriodHertz(400);
    esc1.attach(ESC1_PIN, MID_PULSE, MAX_PULSE);
    esc2.setPeriodHertz(400);
    esc2.attach(ESC2_PIN, MID_PULSE, MAX_PULSE);
    esc1.write(0);
    esc2.write(0);
}

void writeMotorLeft(int us)
{
    esc1.writeMicroseconds(us);
}

void writeMotorRight(int us)
{
    esc2.writeMicroseconds(us);
}