#pragma once

//toggle defines
#define BLUETOOTH
//#define SERIAL_TELEM
#define WATCHDOG

#define ESC1_PIN 13 // TELEM 14
#define ESC2_PIN 15 // TELEM 16
#define LED_PIN 34
#define SDA_PIN 39
#define SCL_PIN 40
#define MAX_PULSE 1750 //2000
#define MIN_PULSE 1000
#define IMU_RANGE LIS331::HIGH_RANGE
#define IMU_MAX 400
 #define IMU_RADIUS 1.094 //CM bot
//#define IMU_RADIUS 2.54 //CM test jig
// #define EEPROM_SIZE 3