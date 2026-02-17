#pragma once

// =======================================================
// FEATURES SEGÚN PLACA
// =======================================================

#if defined(BOARD_V1)

// -------- MICRO --------
#define MCU_ESP32_CLASSIC 1
#define HAS_PSRAM         0

// -------- ETHERNET --------
#define HAS_ETHERNET      1
#define ETH_LAN8720       1
#define ETH_W5500         0

// -------- STORAGE --------
#define HAS_SD_CARD       0

// -------- DISPLAY --------
#define HAS_OLED          1

// -------- RTC --------
#define HAS_RTC_DS3231    1

// -------- EEPROM --------
#define HAS_EEPROM_24C02  1

// -------- RS485 --------
#define HAS_RS485         1

// -------- RF --------
#define HAS_RF433         1


#elif defined(BOARD_V3)

// -------- MICRO --------
#define MCU_ESP32_S3      1
#define HAS_PSRAM         1

// -------- ETHERNET --------
#define HAS_ETHERNET      1
#define ETH_LAN8720       0
#define ETH_W5500         1

// -------- STORAGE --------
#define HAS_SD_CARD       1

// -------- DISPLAY --------
#define HAS_OLED          1

// -------- RTC --------
#define HAS_RTC_DS3231    1

// -------- EEPROM --------
#define HAS_EEPROM_24C02  1

// -------- RS485 --------
#define HAS_RS485         1

// -------- RF --------
#define HAS_RF433         1

#else
    #error "Board no definida"
#endif
