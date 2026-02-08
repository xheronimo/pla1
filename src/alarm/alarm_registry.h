#pragma once
#include <stdint.h>
#include <stddef.h>
#include "alarm_struct.h"

// Inicializar / limpiar
void alarmRegistryInit();

// Añadir regla
bool alarmRegistryAdd(const AlarmRule& rule);

// Obtener regla por id
const AlarmRule* alarmRegistryGet(uint32_t alarmId);

// Obtener todas (para evaluator / snapshot)
const AlarmRule* alarmRegistryAll(size_t& count);
void alarmRegistryClear();

size_t alarmRegistryCount();