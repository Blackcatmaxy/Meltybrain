#pragma once
#include <Arduino.h>
#include <defines.hpp>
#include <DShotESC.h>

DShotESC esc1, esc2;

void initMotors();
void tankDrive(int left, int right);
void writeLeft(int throttle);
void writeRight(int throttle);

void initMotors()
{
    pinMode(ESC1_PIN, OUTPUT);
    pinMode(ESC2_PIN, OUTPUT);

    esc1.install((gpio_num_t)ESC1_PIN, RMT_CHANNEL_0);
    esc2.install((gpio_num_t)ESC2_PIN, RMT_CHANNEL_1);
    esc1.init();
    esc2.init();
}

void tankDrive(int left, int right) {
    writeLeft(left);
    writeRight(right);
}

void writeLeft(int throttle) {
    if (throttle == 0) 
        esc1.sendMotorStop();

    esc1.sendThrottle3D(throttle * MOTOR_MULT);
}

void writeRight(int throttle) {
    if (throttle == 0) 
        esc2.sendMotorStop();

    esc2.sendThrottle3D(throttle * MOTOR_MULT);
}