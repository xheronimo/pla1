#pragma once
#include "signal/signal_reader.h"
#include "Wire.h"

#define BH1750_POWER_ON       0x01
#define BH1750_RESET          0x07
#define BH1750_CONT_HIGH_RES  0x10   // 1 lx resolution




static bool bh1750ReadLux(uint8_t addr, float& lux);