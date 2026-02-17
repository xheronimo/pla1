#pragma once
#include <stdint.h>
#include <stddef.h>

// Estado en tiempo de ejecución de una alarma
struct AlarmRuntime
{
    uint32_t alarmId;
    bool active;
    bool acked;
    bool blocked;
};

// --- API ---
AlarmRuntime* alarmRuntimeGet(uint32_t alarmId);


// Cambia estado activo (si pasa de OFF→ON, acked = false)
void alarmRuntimeSetActive(uint32_t alarmId, bool active);

// Restaurar desde persistencia

void alarmRuntimeRestore(uint32_t alarmId, bool active, bool acked);

// Consultas
bool alarmRuntimeIsActive(uint32_t alarmId);
bool alarmRuntimeIsAcked(uint32_t alarmId);

// Ack manual (para web / botón / etc.)
void alarmRuntimeAck(uint32_t alarmId);

const AlarmRuntime* alarmRuntimeAll(size_t& outCount);
void alarmRuntimeClearAll();

void alarmRuntimeSetBlocked(uint32_t alarmId, bool blocked);
bool alarmRuntimeIsBlocked(uint32_t alarmId);
