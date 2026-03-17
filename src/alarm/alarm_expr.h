#pragma once
#include <string>
#include <vector>

enum class AlarmExprType { EXPR_COND, EXPR_AND, EXPR_OR, EXPR_NOT };
enum class Op { GT, LT, GE, LE, EQ, NE, RISE, FALL };

struct AlarmExpr {
    AlarmExprType type;
    Op op;
    
    // Para condiciones (Leaf)
    std::string signalId;
    float value;
    float hysteresis;
    bool useHys;

    // Para lógica (Tree)
    AlarmExpr* left = nullptr;
    AlarmExpr* right = nullptr;
    AlarmExpr* child = nullptr; // Para el NOT

    // Constructor simple para facilitar la creación
    AlarmExpr(AlarmExprType t) : type(t), left(nullptr), right(nullptr), child(nullptr), useHys(false), hysteresis(0) {}
};