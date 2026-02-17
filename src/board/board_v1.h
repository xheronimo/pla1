#pragma once

// =====================================================
// BOARD V1 - ESP32 + LAN8720
// =====================================================

// ----------------------------
// ANALOG INPUTS
// ----------------------------
#define ANALOG_A1  36
#define ANALOG_A2  34
#define ANALOG_A3  35
#define ANALOG_A4  39

// ----------------------------
// I2C
// ----------------------------
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

#define SD_CS   -1  // No hay SD en V1

// ----------------------------
// PCF I2C ADDRESSES
// ----------------------------
#define RELAY_I2C_ADDR_1  0x24
#define RELAY_I2C_ADDR_2  0x25

#define INPUT_I2C_ADDR_1  0x21
#define INPUT_I2C_ADDR_2  0x22

// ----------------------------
// 1-WIRE / DHT / LED STRIP
// ----------------------------
#define ONEWIRE_1 32
#define ONEWIRE_2 33
#define ONEWIRE_3 14

// ----------------------------
// RF433
// ----------------------------
#define RF_RX_PIN 2
#define RF_TX_PIN 15

// ----------------------------
// ETHERNET LAN8720
// ----------------------------
#define ETH_ADDR        0
#define ETH_POWER_PIN  -1
#define ETH_MDC_PIN     23
#define ETH_MDIO_PIN    18
#define ETH_TYPE        ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT

// ----------------------------
// RS485
// ----------------------------
#define RS485_RX_PIN 16
#define RS485_TX_PIN 13

// ----------------------------
// RTC
// ----------------------------
#define RTC_I2C_ADDR 0x68

// ----------------------------
// DISPLAY
// ----------------------------
#define DISPLAY_I2C_ADDR 0x3C

// ----------------------------
// EEPROM
// ----------------------------
#define EEPROM_I2C_ADDR 0x50






