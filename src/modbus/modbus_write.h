#pragma once 
#include <stdint.h>

bool modbusWriteBlock(uint8_t slave,
                      uint16_t start,
                      uint8_t len,
                      uint16_t* values);