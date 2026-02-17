#include "system/i2c_health.h"

uint8_t  g_lastI2CChip   = 0;
uint8_t  g_lastI2CAddr   = 0;
uint32_t g_lastI2CErrTs  = 0;
