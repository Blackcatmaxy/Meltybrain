#pragma once

//toggle defines
#define BLUETOOTH
#define SERIAL_TELEM
//#define WATCHDOG

#define ESC1_PIN 40 // TELEM 14
#define ESC2_PIN 41 // TELEM 16
#define LEDT_PIN 45
#define LEDB_PIN 46
#define CS0_PIN 35
#define CS1_PIN 36
// few mistakes made on PCB
#define MOSI_PIN 39 //Labelled "SDO" on board
#define MISO_PIN 38 //Labelled "SAO" on board
#define SCK_PIN 37  //Labelled "SPC" on board

#define IMU_RANGE LIS331::HIGH_RANGE
#define IMU_MAX 400
#define IMU_RADIUS 0.582 //CM bot
#define IMU_DISTANCE (9.2 / 1000)// mm to m
#define MOTOR_MULT 0.5
//#define IMU_RADIUS 2.54 //CM test jig
// #define EEPROM_SIZE 3