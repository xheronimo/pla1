#pragma once
#include <stdint.h>
#include <stddef.h>

enum class AlarmExprType : uint8_t {
    EXPR_COND,   // hoja
    EXPR_AND,
    EXPR_OR,
    EXPR_NOT
};

enum AlarmOp : uint8_t {
    OP_EQ,
    OP_NE,
    OP_GT,
    OP_LT,
    OP_GE,
    OP_LE,
    OP_RISE,
    OP_FALL
};

struct AlarmExpr
{
    AlarmExprType type;

    // --- Si type == COND ---
    uint32_t signalId;
    AlarmOp  op;
    float    value;
    float    hysteresis;   // opcional
    bool     useHys;

    // --- Si type == AND / OR ---
    AlarmExpr* left;
    AlarmExpr* right;

    // --- Si type == NOT ---
    AlarmExpr* child;
};

