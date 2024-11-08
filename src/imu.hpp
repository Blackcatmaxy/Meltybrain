#pragma once

#include <Arduino.h>
#include <defines.hpp>
#include <SparkFun_LIS331.h>
#include <math.h>
#include <MedianFilterLib.h>

#define xComponent 0.7684834570
#define yComponent 0.6398696561

void initImu();
float readForce();
float getRPM();

float zeroGX = 0, zeroGY;
float maxG, maxRealG;
Preferences prefs;

LIS331 imu;
void initImu()
{
    imu.setI2CAddr(0x19);
    imu.begin(LIS331::USE_I2C);
    imu.setFullScale(IMU_RANGE);
    float sumX = 0, sumY = 0;
    int count = 1000;
    // MedianFilter<float> FilterBoi(10);
    
    delay(250);
    for (int i = 0; i < count; i++)
    {
        int16_t x, y, z;
        imu.readAxes(x, y, z);
        float xG = imu.convertToG(IMU_MAX, x);
        sumX += imu.convertToG(IMU_MAX, x);
        sumY += imu.convertToG(IMU_MAX, y);
        // Serial.printf("g:%f, filter%f\n", xG, FilterBoi.AddValue(xG));
    }
    zeroGX = sumX / count;
    zeroGY = sumY / count;
    // zeroGX = FilterBoi.GetFiltered();
    maxG = prefs.getFloat("maxG", 0);
    maxRealG = prefs.getFloat("maxRealG", 0);
}

float readForce()
{
    int16_t x, y, z;
    imu.readAxes(x, y, z);
    float xG, yG;
    // Serial.printf("zero gx%f", zeroGX);
    xG = imu.convertToG(IMU_MAX, x) - zeroGX;
    yG = imu.convertToG(IMU_MAX, y) - zeroGY;
    // Serial.printf("xg:%f zerog: %f xcos %f, yg:%f zerog: %f ysin%f\n", xG, zeroGX, xG / 0.771191440f, yG, zeroGY, yG/sin(39.539));
    // return xG * xComponent - yG * yComponent;
    xG = xG / 0.771191440;// x/cos(39.539)
    yG = yG / 0.6366033011f;
    // float G = (xG + yG)/2.0f; 
    float G = yG;

    // if (G > maxG)
    // {
    //     maxG = G;
    //     prefs.putFloat("maxG", G);
    //     prefs.end();
    //     prefs.begin("max");
    // }

    // if (xG > maxRealG)
    // {
    //     maxRealG = xG;
    //     prefs.putFloat("maxRealG", xG);
    //     prefs.end();
    //     prefs.begin("max");
    // }

    return G;
}

double getRPM(float G)
{
    double rpm = fabs(G * 89445.0f);
    rpm = rpm / IMU_RADIUS;
    rpm = sqrt(rpm);
    return rpm;
}

double getRotationIntervalMs(double rpm)
{
    return 1.0f / rpm * 60 * 1000;
}