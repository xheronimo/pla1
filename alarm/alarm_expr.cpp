#include "alarm_expr.h"

#include <math.h>

// Esta función debe existir en tu sistema real.
// El loader solo construye expresiones; el runtime evalúa valores reales.
extern bool getSignalValue(const String& src, float& outValue);

// =====================================================
// CONSTRUCTORES
// =====================================================



AlarmExpr::AlarmExpr(const String& s, AlarmOp o, float v)
    : type(EXPR_COND),
      src(s),
      op(o),
      value(v)
{
}

const char* alarmOpToString(AlarmOp op)
{
    switch (op)
    {
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        case OP_GT: return ">";
        case OP_LT: return "<";
        case OP_GE: return ">=";
        case OP_LE: return "<=";
        default:    return "?";
    }
}


// =====================================================
// EVALUACIÓN
// =====================================================

bool AlarmExpr::eval() const
{
    switch (type)
    {
        case EXPR_COND:
        {
            float v;
            if (!getSignalValue(src, v))
                return false;

            switch (op)
            {
                case OP_EQ: return v == value;
                case OP_NE: return v != value;
                case OP_LT: return v <  value;
                case OP_GT: return v >  value;
                case OP_LE: return v <= value;
                case OP_GE: return v >= value;
            }
            return false;
        }

        case EXPR_AND:
            for (auto* c : children)
                if (!c->eval()) return false;
            return true;

        case EXPR_OR:
            for (auto* c : children)
                if (c->eval()) return true;
            return false;

        case EXPR_NOT:
            if (children.empty()) return false;
            return !children[0]->eval();
    }

    return false;
}
