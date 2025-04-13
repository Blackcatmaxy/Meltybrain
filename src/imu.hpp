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

LIS331 imu0, imu1;
void initImu()
{
    pinMode(CS0_PIN, OUTPUT);
    imu0.setSPICSPin(CS0_PIN);
    imu0.begin(LIS331::USE_SPI);
    imu0.setFullScale(IMU_RANGE);

    pinMode(CS1_PIN, OUTPUT);
    imu1.setSPICSPin(CS1_PIN);
    imu1.begin(LIS331::USE_SPI);
    imu1.setFullScale(IMU_RANGE);
    double sumX = 0, sumY = 0;
    int count = 1000;
    // MedianFilter<float> FilterBoi(10);
    
//    delay(250);
    for (int i = 0; i < count; i++)
    {
        int16_t x, y, z;
        imu0.readAxes(x, y, z);
//        float xG = imu.convertToG(IMU_MAX, x);
        sumX += imu0.convertToG(IMU_MAX, x);
        sumY += imu0.convertToG(IMU_MAX, y);
//        delay(10);
        // Serial.printf("g:%f, filter%f\n", xG, FilterBoi.AddValue(xG));
    }
    zeroGX = sumX / count;
    zeroGY = sumY / count;
    Serial.printf("zero x: %f, zero y: %f\n", zeroGX, zeroGY);
    maxG = prefs.getFloat("maxG", 0);
    maxRealG = prefs.getFloat("maxRealG", 0);
}

float readForce()
{
    int16_t x, y, z;
    imu0.readAxes(x, y, z);
    float xG, yG;
    // Serial.printf("zero gx%f", zeroGX);
    xG = imu0.convertToG(IMU_MAX, x) - zeroGX;
    yG = imu0.convertToG(IMU_MAX, y) - zeroGY;
     // Serial.printf("xg:%f zerog: %f xcos %f, yg:%f zerog: %f ysin%f\n", xG, zeroGX, xG / 0.771191440f, yG, zeroGY, yG/sin(39.539));
    // return xG * xComponent - yG * yComponent;
    xG = xG / 0.771191440f;// x/cos(39.539)
    yG = yG / 0.6366033011f;
     float G = (xG + yG)/2.0f;
//    float G = yG;

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

double getDoubleRPM() {
    int16_t x, y, z;
    imu0.readAxes(x, y, z);
    float x0G = imu0.convertToG(IMU_MAX, y);
    imu1.readAxes(x, y, z);
    float x1G = imu0.convertToG(IMU_MAX, y);
    Serial.printf("x0g: %f, x1g: %f\n", x0G, x1G);
    double deltaA = abs(x0G - x1G);
    double omega = sqrt(deltaA/IMU_DISTANCE); // rad/s
    return omega * 60 / 3.14159 ;
}

double getRotationIntervalMs(double rpm)
{
    return 1.0f / rpm * 60 * 1000;
}