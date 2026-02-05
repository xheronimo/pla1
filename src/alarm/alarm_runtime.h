#pragma once

#include <stdint.h>

struct AlarmRuntime {
    bool activaAhora = false;
    bool latched     = false;
    bool acked       = false;
    uint32_t lastTs  = 0;
    uint32_t ultimoValor =0:
    uint32_t valor  = 0;
    char tipo[32];
    char estado[32];

};
