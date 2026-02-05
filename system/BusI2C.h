#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ==================================================
// API I2C
// ==================================================

bool BusI2C_Init();

bool BusI2C_Write(uint8_t addr, uint8_t data);
bool BusI2C_WriteBytes(uint8_t addr, uint8_t* data, uint8_t len);
bool BusI2C_Read(uint8_t addr, uint8_t* data, uint8_t len);

uint32_t BusI2C_GetErrors();

// ==================================================
// MUTEX GLOBAL I2C
// (ÚNICO en todo el sistema)
// ==================================================

extern SemaphoreHandle_t semI2C;
