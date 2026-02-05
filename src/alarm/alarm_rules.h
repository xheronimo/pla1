#pragma once
#include <stdint.h>
#include <vector>

enum AlarmOp : uint8_t {
    OP_EQ,
    OP_NE,
    OP_GT,
    OP_LT,
    OP_GE,
    OP_LE
};

struct AlarmCondition {
    const char* signalId;
    AlarmOp op;
    float value;
};

struct AlarmRule {
    const char* id;
    const char* nombre;

    bool activa;
    uint64_t grupos;

    AlarmCondition cond;

    // Runtime
    bool activaAhora;
    bool latched;
};
