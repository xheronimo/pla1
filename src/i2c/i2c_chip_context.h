#pragma once
#include <stdint.h>

enum class ChipState : uint8_t {
    UNINITIALIZED,
    INITIALIZING,
    WARMUP,
    READY,
    ERROR
};

struct ChipContext {
    ChipState state = ChipState::UNINITIALIZED;

    uint32_t initTs;        // cuándo empezó INIT/WARMUP
    uint32_t lastReadTs;    // última lectura OK

    uint32_t warmupMs;      // tiempo mínimo de calentamiento
    uint32_t retryMs;       // tiempo entre reintentos

    uint8_t  errorCount;        // total
    uint8_t  consecutiveErrors;

    bool busy = false;              // chip ocupado (no bloquear)
    bool fatal;          // 👈 NUEVO: no reintentar más
};
