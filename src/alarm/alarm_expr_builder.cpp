#include "alarm_expr_builder.h"
#include <stdlib.h>
#include <string.h>

// ----------------------------
// Parsers
// ----------------------------
static AlarmExprType parseExprType(const char* s)
{
    if (!s) return AlarmExprType::EXPR_COND;
    if (strcasecmp(s, "and") == 0) return AlarmExprType::EXPR_AND;
    if (strcasecmp(s, "or")  == 0) return AlarmExprType::EXPR_OR;
    if (strcasecmp(s, "not") == 0) return AlarmExprType::EXPR_NOT;
    return AlarmExprType::EXPR_COND;
}

static AlarmOp parseOp(const char* s)
{
    if (!s) return OP_EQ;

    if (strcasecmp(s, "eq") == 0)   return OP_EQ;
    if (strcasecmp(s, "ne") == 0)   return OP_NE;
    if (strcasecmp(s, "gt") == 0)   return OP_GT;
    if (strcasecmp(s, "lt") == 0)   return OP_LT;
    if (strcasecmp(s, "ge") == 0)   return OP_GE;
    if (strcasecmp(s, "le") == 0)   return OP_LE;
    if (strcasecmp(s, "rise") == 0) return OP_RISE;
    if (strcasecmp(s, "fall") == 0) return OP_FALL;
    return OP_EQ;
}

// ----------------------------
// Builder recursivo
// ----------------------------
AlarmExpr* buildExpr(const JsonObjectConst& o)
{
    if (o.isNull()) return nullptr;

    AlarmExpr* e = new AlarmExpr{};
    const char* type = o["type"] | "cond";

    if (!strcasecmp(type, "cond"))
    {
        e->type     = AlarmExprType::EXPR_COND;
        e->signalId = o["signal"] | 0;
        e->op       = parseOp(o["op"] | "eq");
        e->value    = o["value"] | 0.0f;
        e->hysteresis = o["hys"] | 0.0f;
        e->useHys   = o.containsKey("hys");
    }
    else if (!strcasecmp(type, "and"))
    {
        e->type  = AlarmExprType::EXPR_AND;
        e->left  = buildExpr(o["left"]);
        e->right = buildExpr(o["right"]);
    }
    else if (!strcasecmp(type, "or"))
    {
        e->type  = AlarmExprType::EXPR_OR;
        e->left  = buildExpr(o["left"]);
        e->right = buildExpr(o["right"]);
    }
    else if (!strcasecmp(type, "not"))
    {
        e->type  = AlarmExprType::EXPR_NOT;
        e->child = buildExpr(o["expr"]);
    }
    else
    {
        delete e;
        return nullptr;
    }

    return e;
}

