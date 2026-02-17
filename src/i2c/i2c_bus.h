#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "i2c_chip_context.h"
#include "i2c_chip_registry.h"

// ========================================================
// INIT
// ========================================================

void i2cBusInit();
SemaphoreHandle_t i2cGetMutex();

// ========================================================
// LOW LEVEL WRITE
// ========================================================

bool i2cWrite8(uint8_t addr, uint8_t reg, uint8_t val);
bool i2cWrite16(uint8_t addr, uint8_t reg, uint16_t val);
bool i2cWriteBytes(uint8_t addr, uint8_t reg, const uint8_t* data, uint8_t len);
bool i2cWriteRaw(uint8_t addr, uint8_t value);

// ========================================================
// LOW LEVEL READ
// ========================================================

bool i2cRead8(uint8_t addr, uint8_t reg, uint8_t& out);
bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t& out);
bool i2cReadBytes(uint8_t addr, uint8_t reg, uint8_t* buf, uint8_t len);
bool i2cReadRaw(uint8_t addr, uint8_t* buf, uint8_t len);

bool i2cPing(uint8_t addr);
bool pcfInitShadow(ChipContext* ctx);

 ChipContext* i2cGetDriver(I2CDevice chip, uint8_t address);