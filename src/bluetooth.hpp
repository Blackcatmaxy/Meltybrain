#pragma once

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <ESP32Servo.h>
#include "defines.hpp"
#include <Adafruit_SleepyDog.h>

Servo esc1, esc2;
int rot_throttle = 0;

void bluetoothSetup();
void monitor(BLEDevice device);
void HIDReportWritten(BLEDevice central, BLECharacteristic characteristic);

void bluetoothSetup(void *pvParams)
{
    if (!BLE.begin())
    {
        Serial.println("BLE start failed");
        return;
    }

    Serial.println("Started Scanning");
    BLE.scan(true);

    while (1)
    {
        BLEDevice availableDevice = BLE.available();

        // Serial.println(availableDevice);
        if (availableDevice)
        {
            // Serial.printf("Found %s '%s' - %s\n", availableDevice.address().c_str(), availableDevice.localName().c_str(), availableDevice.advertisedServiceUuid().c_str());

            if (availableDevice.advertisedServiceUuid() == "1812")
            {
                // stop scanning
                // connected = true;
                Serial.println("Connected");
                BLE.stopScan();
                digitalWrite(LED_PIN, LOW);
                monitor(availableDevice);
                delay(100);
                // peripheral disconnected, we are done
                digitalWrite(LED_PIN, HIGH);
                esc1.writeMicroseconds(0);
                esc2.writeMicroseconds(0);
                Watchdog.disable();
                delay(100);
                digitalWrite(LED_PIN, LOW);
                BLE.scan(true);
            }
        }
        delay(250);
    }
}

void monitor(BLEDevice device)
{
    Serial.println("Connecting...");
    if (device.connect())
    {
        Serial.println("Connected");
    }
    else
    {
        Serial.println("Failed to connect!");
        return;
    }

    // discover peripheral attributes
    //  Serial.println("Discovering service 0x1812 ...");
    if (device.discoverService("1812"))
    {
        Serial.println("0x1812 discovered");
    }
    else
    {
        Serial.println("0x1812 disc failed");
        device.disconnect();
        return;
    }

    BLEService service = device.service("1812"); // get the HID service

    int characteristicCount = service.characteristicCount();
    Serial.printf("Service has %d chars\n", characteristicCount);
    for (int i = 0; i < characteristicCount; i++)
    {
        BLECharacteristic characteristic = service.characteristic(i);
        //    Serial.print("characteristic "); Serial.print(i, DEC);
        //    Serial.print(" = "); Serial.println(bc.uuid());
        //    Serial.print("Descriptor count = "); Serial.println(bc.descriptorCount(), DEC);
        if (strcasecmp(characteristic.uuid(), "2a4D") == 0) // enable notify
        {
            characteristic.subscribe();
            characteristic.setEventHandler(BLEWritten, HIDReportWritten);
        }
    }

    BLECharacteristic protmodChar = service.characteristic("2A4E"); // get protocol mode characteristic
    if (protmodChar != NULL)
    {
        //    Serial.println("Setting Protocol mode to REPORT_MODE");
        protmodChar.writeValue((uint8_t)0x01); // set protocol report mode (we want reports)
    }

    // int countdownMS = Watchdog.enable(10000);
    while (device.connected())
        ;
}

void HIDReportWritten(BLEDevice central, BLECharacteristic characteristic)
{
    // Watchdog.reset();
    int iLen, i;
    uint8_t ucData[128];
    char buffer[64];

    // central wrote new HID report info
    iLen = characteristic.readValue(ucData, sizeof(ucData));

    Serial.printf("HID Report recieved %d bytes -", iLen);
    for (int j = 0; j < iLen; j++)
    {
        Serial.printf(" '%d:%d' ", j, ucData[j]);
    }

    // ucData[4] is left joystick vertical, ucData[6] is right vertical
    int pwm1 = ucData[6], pwm2 = ucData[4];
    // ranges from 1-255 with stick resting in the middle ~128, we don't want motors moving on their own
    // so we make 128 the new 0 point, going from 0-128
    if (pwm1 > 128)
        pwm1 = 0;
    else
        pwm1 = 128 - pwm1;
    if (pwm2 > 128)
        pwm2 = 0;
    else
        pwm2 = 128 - pwm2;

    // Allow 'A' button on the controller to toggle LED
    digitalWrite(LED_PIN, ucData[2] / 64);

    // ESC sees 1000 as stop and 2000 as full throttle
    pwm1 = map(pwm1, 0, 128, MIN_PULSE, MAX_PULSE);
    pwm2 = map(pwm2, 0, 128, MIN_PULSE, MAX_PULSE);
    esc1.writeMicroseconds(pwm1);
    esc2.writeMicroseconds(pwm2);

    Serial.printf(" pwm %d:%d", ucData[4], pwm2);
    Serial.printf(" pwm %d:%d\n", ucData[6], pwm1);
}
