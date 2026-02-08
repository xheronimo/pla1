#include "alarm_evaluator.h"

#include "alarm/alarm_expr.h"
#include "alarm/alarm_runtime.h"
#include "alarm/alarm_queue.h"
#include "signal/signal_manager.h"
#include "alarm_registry.h"

// --------------------------------------------------
// Evaluar condición hoja (COND)
// --------------------------------------------------
static bool evalCond(const AlarmExpr &e)
{
    float value = 0.0f;
    bool hasValue = signalManagerGetValueById(e.signalId, value);

    bool error = false;
    signalManagerGetErrorById(e.signalId, error);

    Signal *s = signalManagerGet(e.signalId);
    float prev = s ? s->prev : 0.0f;

    bool now = (value != 0.0f);
    bool prevb = (prev != 0.0f);

    switch (e.op)
    {
    case OP_GT:
        if (!hasValue)
            return false;
        return !e.useHys
                   ? value > e.value
                   : (value > e.value || (value > e.value - e.hysteresis));

    case OP_LT:
        if (!hasValue)
            return false;
        return !e.useHys
                   ? value < e.value
                   : (value < e.value || (value < e.value + e.hysteresis));

    case OP_GE:
        return hasValue && value >= e.value;

    case OP_LE:
        return hasValue && value <= e.value;

    case OP_EQ:
        return hasValue && value == e.value;

    case OP_NE:
        return hasValue && value != e.value;

    case OP_RISE:
        return hasValue && (!prevb && now);

    case OP_FALL:
        return hasValue && (prevb && !now);
    }

    return false;
}

// --------------------------------------------------
// Evaluar expresión completa (recursivo)
// --------------------------------------------------
static bool evalExpr(const AlarmExpr &e)
{
    switch (e.type)
    {
    case AlarmExprType::EXPR_COND:
        return evalCond(e);

    case AlarmExprType::EXPR_AND:
        return evalExpr(*e.left) && evalExpr(*e.right);

    case AlarmExprType::EXPR_OR:
        return evalExpr(*e.left) || evalExpr(*e.right);

    case AlarmExprType::EXPR_NOT:
        return !evalExpr(*e.child);
    }
    return false;
}

// --------------------------------------------------
// Evaluador principal de alarmas
// --------------------------------------------------
void alarmEvaluate()
{
    size_t count;

    const AlarmRule *rules = alarmRegistryAll(count);

    for (size_t i = 0; i < count; i++)
    {
        const AlarmRule &r = rules[i];

        bool conditionActive = evalExpr(*r.expr);

        bool wasActive = alarmRuntimeIsActive(r.alarmId);
        bool wasAcked = alarmRuntimeIsAcked(r.alarmId);

        bool newActive;

        // -----------------------------
        // LATCH / ACK
        // -----------------------------
        if (!r.latch)
        {
            newActive = conditionActive;
        }
        else
        {

            if (conditionActive)
                newActive = true;
            else if (wasAcked)
                newActive = false;
        }

        // -----------------------------
        // Cambio de estado → evento
        // -----------------------------
        if (newActive != wasActive)
        {
            alarmRuntimeSetActive(r.alarmId, newActive);

            AlarmEvent ev{};
            ev.alarmId = r.alarmId;
            ev.active = newActive;
            ev.kind = AlarmEventKind::STATE_CHANGE;

            alarm_queuePush(ev);
        }
    }
}
