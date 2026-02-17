#pragma once
#include "signal/signal_struct.h"
#include <stdint.h>
#include <string.h>

static uint32_t mergeWords(uint16_t w1, uint16_t w2, ModbusWordOrder order);



bool modbusDecodeValue(const uint16_t* data,
                       const Signal& s,
                       float& out);
