#pragma once

#include <vector>
#include <Arduino.h>

// =====================================================
// OPERADORES
// =====================================================

enum AlarmOp : uint8_t
{
    OP_EQ,   // ==
    OP_NE,   // !=
    OP_LT,   // <
    OP_GT,   // >
    OP_LE,   // <=
    OP_GE    // >=
};

// =====================================================
// TIPOS DE EXPRESIÓN
// =====================================================

enum AlarmExprType : uint8_t
{
    EXPR_COND,
    EXPR_AND,
    EXPR_OR,
    EXPR_NOT
};

const char* alarmOpToString(AlarmOp op);

// =====================================================
// EXPRESIÓN
// =====================================================

struct AlarmExpr
{
    AlarmExprType type;

    // --- condición simple ---
    String src;      // ED.D5, P.P1, T, etc.
    AlarmOp op;
    float value = 0;

    // --- nodos hijos ---
    std::vector<AlarmExpr*> children;

    // --- constructores ---
    AlarmExpr(AlarmExprType t);
    AlarmExpr(const String& src, AlarmOp op, float value);

    // --- evaluación ---
    bool eval() const;
};

