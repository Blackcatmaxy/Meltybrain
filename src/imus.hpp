#pragma once

#include <Arduino.h>
#include <defines.hpp>
#include <SparkFun_LIS331.h>
#include <math.h>
#include <MedianFilterLib.h>
#include "imu.hpp"

#define xComponent 0.7684834570
#define yComponent 0.6398696561

void initImu();
float readForce();
float getRPM();

float zeroGX = 0, zeroGY;
float maxG, maxRealG;
//Preferences prefs;

IMU imu0, imu1;
void initImu()
{
    imu0.init(CS0_PIN);
    imu1.init(CS1_PIN);
}

float getRPM()
{
    float accel = imu0.readAccel() + imu1.readAccel();
    accel *= 0.5f;

    float rpm = fabs(accel * 89445.0f);
    rpm = rpm / IMU_RADIUS;
    rpm = sqrt(rpm);
    return rpm;
}

double getRotationIntervalMs(double rpm)
{
    return 1.0f / rpm * 60 * 1000;
}