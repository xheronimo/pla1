#pragma once

#include <stdint.h>
#include "signal/signal_struct.h"
#include "i2c_chip_registry.h"
#include "i2c_bus.h"




// =====================================================
// CONFIGURACIÓN
// =====================================================
#define ADS1115_MAX_ADDR_SLOTS  8
#define ADS1115_CACHE_MS        300

// =====================================================
// API ESTÁNDAR DRIVER
// =====================================================

/**
 * Inicializa un chip ADS1115 en la dirección I2C indicada.
 * options:
 *   bits 7..4 → Gain
 *   bits 3..0 → Data rate
 */
bool ads1115Init(uint8_t addr, uint8_t options);

/**
 * Lee una señal ADS1115.
 * Usa cache por chip y lectura multi-canal.
 */
bool ads1115ReadSignal(const Signal& s, float& out);

/**
 * Devuelve metadata para el frontend.
 */
void ads1115GetMetadata(ChipMetadata& meta);

/**
 * Resetea cache interna (no toca ChipContext global).
 */
void ads1115Reset();

bool ads1115Detect(uint8_t addr);