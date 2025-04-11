// written by Daniel Bereza aka Blackcatmaxy
// based on https://github.com/bitbank2/Nano_33_Gamepad with permission
// credit is due for being the best reference on the dark magic of BLE I could find
#pragma once

#include <Arduino.h>
#include <ArduinoBLE.h>
#include "defines.hpp"
#include <Adafruit_SleepyDog.h>

struct Throttles {
    int rotation = 0;
    int forward = 0;
    int side = 0;
    int rightForward = 0;
};

Throttles* throttle_ptr;
ulong lastBle = 0;

// init code snippet, need to use concurrency or multithreading since we block in order to know when disconnect, this is for ESP32
// xTaskCreatePinnedToCore(
//            bluetoothSetup,
//            "bluetoothTask",
//            200000,
//            (void*)&throttles,
//            0,
//            nullptr,
//            1 // core being run on, wireless is core 1
//    );

void bluetoothSetup(void *pvParams);
void monitor(BLEDevice device);
void dataReceived(BLEDevice device, BLECharacteristic characteristic);

void bluetoothSetup(void *pvParams)
{
    throttle_ptr = (Throttles*)pvParams;

    if (!BLE.begin())
    {
        Serial.println("BLE start failed");
        return;
    }

    Serial.println("Started Scanning");
    BLE.scan(true);

    while (true)
    {
        BLEDevice availableDevice = BLE.available();

        if (availableDevice)
        {
            // uncomment to see all available devices
            // Serial.printf("Found %s '%s' - %s\n", availableDevice.address().c_str(), availableDevice.localName().c_str(), availableDevice.advertisedServiceUuid().c_str());

            if (availableDevice.advertisedServiceUuid() == "1812")
            {
#ifdef SERIAL_TELEM
                Serial.println("Connected");
#endif
                BLE.stopScan();

                monitor(availableDevice); // blocks until disconnect

                // we are now disconnected
                throttle_ptr->rotation = 0;
                throttle_ptr->forward = 0;
                throttle_ptr->rightForward = 0;
#ifdef WATCHDOG
                Watchdog.disable();
#endif
                BLE.scan(true); // restart scan
            }
        }
        delay(250);
    }
}

void monitor(BLEDevice device)
{
    Serial.print("Connection");
    if (device.connect())
    {
        Serial.println("- success");
    }
    else
    {
        Serial.println("- failure!");
        return;
    }

    if (device.discoverService("1812")) // Device is HID
    {
        Serial.println("Device is HID");
    }
    else
    {
        Serial.println("Device is not HID - abort");
        device.disconnect();
        return;
    }

    BLEService service = device.service("1812");

    int characteristicCount = service.characteristicCount();
    Serial.printf("Service has %d chars\n", characteristicCount);
    for (int i = 0; i < characteristicCount; i++)
    {
        BLECharacteristic characteristic = service.characteristic(i);
        if (strcasecmp(characteristic.uuid(), "2a4D") == 0)
        {
            characteristic.subscribe();
            characteristic.setEventHandler(BLEWritten, dataReceived);
        }
    }

    // Required for data reporting?
    BLECharacteristic protocol = service.characteristic("2A4E");
    if (protocol != NULL)
    {
        //    Serial.println("Set device to report data to us");
        protocol.writeValue((uint8_t)0x01);
    }
#ifdef WATCHDOG
    int countdownMS = Watchdog.enable(10000);
#endif
    while (device.connected())
        ; // block until disconnect
}

void dataReceived(BLEDevice device, BLECharacteristic characteristic)
{
#ifdef WATCHDOG
    Watchdog.reset();
#endif
    uint8_t data[128];
    int i = characteristic.readValue(data, sizeof(data));;

    // define to see all values of data received, tested for Stadia controller but might vary between different controllers?
#ifdef SERIAL_TELEM
    // bool print = millis() - lastBle > 100;
    bool print = false;

    if (print) {
        lastBle = millis();
        Serial.printf("Data received %d bytes -", i);
        for (int j = 0; j < i; j++) {
            Serial.printf(" '%d:%d' ", j, data[j]);
        }
    }
#endif
    // data[4] is left joystick y, data[6] is right y
    // 8 is right trigger 3 left joy x 5 right joy x
    // ranges from 1-255 with stick resting in the middle ~128

    // Allow 'A' button on the controller to toggle LED
    digitalWrite(LED_PIN, data[2] & 64);
    if (data[2] & 16) {
        throttle_ptr->rotation = 1;
#ifdef SERIAL_TELEM
        if (print)
            Serial.println("Enable rotation");
#endif
    } else if (data[2] & 32) {
        throttle_ptr->rotation = 0;
#ifdef SERIAL_TELEM
        if (print)
            Serial.println("Disable rotation");
#endif
    }
    throttle_ptr->forward = map(data[4], 0, 255, 128, -128);
    throttle_ptr->side = map(data[5], 0, 255, -128, 128);
    throttle_ptr->rotation = data[8];
    throttle_ptr->rightForward = map(data[6], 0, 255, 128, -128);

#ifdef SERIAL_TELEM
    if (print) {
        Serial.printf(" forward %d:%d", data[4], throttle_ptr->forward);
        Serial.printf(" side %d:%d", data[5], throttle_ptr->side);
        Serial.printf(" rotation %d:%d\n", data[8], throttle_ptr->rotation);
    }
#endif
}