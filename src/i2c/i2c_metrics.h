#pragma once
#include <stdint.h>

void     i2cMetricsInit();
void     i2cNotifySuccess();
void     i2cNotifyError();

uint32_t i2cGetTotalOps();
uint32_t i2cGetTotalErrors();
uint32_t i2cGetTotalResets();
uint32_t i2cGetCurrentSpeed();
float    i2cGetBusHealth();

void     i2cSetSpeed(uint32_t hz);
