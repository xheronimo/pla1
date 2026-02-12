#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "i2c_chip_context.h"
#include "i2c_chip.h"
#include "Wire.h"

// Mutex global del bus
void i2cBusInit();
SemaphoreHandle_t i2cGetMutex();

// Low level comunes
bool i2cWrite8(uint8_t addr, uint8_t reg, uint8_t val);
bool i2cWrite16(uint8_t addr, uint8_t reg, uint16_t val);
bool i2cRead8(uint8_t addr, uint8_t reg, uint8_t& out);
bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t& out);
bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len);
bool i2cDetect(uint8_t addr);
bool i2cWriteRaw(uint8_t addr, uint8_t value);
bool i2cReadRaw(uint8_t addr, uint8_t* buf, uint8_t len);
bool i2cWriteBytes(uint8_t addr, uint8_t reg, const uint8_t* data, uint8_t len);


// Contexto global
ChipContext* i2cGetContext(I2CDevice type, uint8_t addr);
void i2cBusReset();