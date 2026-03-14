#pragma once
#include "signal/signal_struct.h"
#include "i2c/i2c_chip_context.h"

// Inicialización y detección
bool ds3231Init(uint8_t addr, uint8_t options);
bool ds3231Detect(uint8_t addr);

// Lectura de señales (Hora, Temperatura, etc.)
bool ds3231ReadSignal(const Signal& s, float& out);

// Metadatos para la interfaz
void ds3231GetMetadata(ChipMetadata& meta);

// Limpieza de cache
void ds3231Reset();

bool ds3231WriteTime(uint16_t year, uint8_t month, uint8_t day, 
                     uint8_t hour, uint8_t minute, uint8_t second) ;
bool ds3231ReadTime(uint16_t &year, uint8_t &month, uint8_t &day, 
                    uint8_t &hour, uint8_t &minute, uint8_t &second);