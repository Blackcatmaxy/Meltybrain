#pragma once

#include <Arduino.h>
#include <defines.hpp>
#include <SparkFun_LIS331.h>
#include <math.h>

class IMU
{
public:
    IMU(int csPin);
    float readForce();
    double getRPM(float G);
private:
    LIS331 lis331;
    float zeroGX = 0, zeroGY;
    float maxG, maxRealG;
};

inline IMU::IMU(int csPin) {
    pinMode(csPin, OUTPUT);
    lis331.setSPICSPin(csPin);
    lis331.begin(LIS331::USE_SPI);
    lis331.setFullScale(IMU_RANGE);

    float sumX = 0, sumY = 0;
    int count = 1000;
    // MedianFilter<float> FilterBoi(10);

    //    delay(250);
    for (int i = 0; i < count; i++)
    {
        int16_t x, y, z;
        lis331.readAxes(x, y, z);
        //        float xG = imu.convertToG(IMU_MAX, x);
        sumX += lis331.convertToG(IMU_MAX, x);
        sumY += lis331.convertToG(IMU_MAX, y);
        //        delay(10);
        // Serial.printf("g:%f, filter%f\n", xG, FilterBoi.AddValue(xG));
    }
    zeroGX = sumX / count;
    zeroGY = sumY / count;
}

float IMU::readForce() {
    int16_t x, y, z;
    lis331.readAxes(x, y, z);
    float xG, yG;
    // Serial.printf("zero gx%f", zeroGX);
    xG = lis331.convertToG(IMU_MAX, x) - zeroGX;
    yG = lis331.convertToG(IMU_MAX, y) - zeroGY;
    // Serial.printf("xg:%f zerog: %f xcos %f, yg:%f zerog: %f ysin%f\n", xG, zeroGX, xG / 0.771191440f, yG, zeroGY, yG/sin(39.539));
    // return xG * xComponent - yG * yComponent;
    xG = xG / 0.771191440f;// x/cos(39.539)
    yG = yG / 0.6366033011f;
    float G = (xG + yG)/2.0f;
    return G;
}
