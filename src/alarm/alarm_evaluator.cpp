#include "alarm/alarm_evaluator.h"
#include "alarm/alarm_manager.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include <Arduino.h>

namespace AlarmMgr {

    // --------------------------------------------------
    // Evaluar condición hoja (Comparación de señal)
    // --------------------------------------------------
    static bool evalCond(const AlarmExpr &e) {
        float value = 0.0f;
        // Obtenemos la señal directamente del SignalManager
        Signal* s = SignalMgr::getById(e.signalId);

        // 🛑 BLOQUEO POR CALIDAD: Si no existe, tiene error o no es válida
        if (!s || !s->valid || s->error || s->quality != SignalQuality::GOOD) {
            return false; // ⛔ SAFE → No dispara la alarma si la señal es dudosa
        }

        value = s->value;
        float prev = s->prev;
        bool now = (value != 0.0f);
        bool prevb = (prev != 0.0f);

        switch (e.op) {
            case Op::GT: // Mayor que con Histéresis mejorada (Schmitt Trigger)
                return !e.useHys ? (value > e.value) : (value > (e.value + e.hysteresis));

            case Op::LT: // Menor que con Histéresis
                return !e.useHys ? (value < e.value) : (value < (e.value - e.hysteresis));

            case Op::GE: return value >= e.value;
            case Op::LE: return value <= e.value;
            case Op::EQ: return abs(value - e.value) < 0.001f;
            case Op::NE: return abs(value - e.value) > 0.001f;

            case Op::RISE: return (!prevb && now);
            case Op::FALL: return (prevb && !now);
            
            default: return false;
        }
    }

    // --------------------------------------------------
    // Evaluar expresión completa (Recursivo optimizado)
    // --------------------------------------------------
    bool evalExpr(const AlarmExpr* e) {
        if (!e) return false;

        switch (e->type) {
            case AlarmExprType::EXPR_COND:
                return evalCond(*e);

            case AlarmExprType::EXPR_AND:
                // Cortocircuito: si el primero es falso, no evalúa el segundo
                return evalExpr(e->left) && evalExpr(e->right);

            case AlarmExprType::EXPR_OR:
                // Cortocircuito: si el primero es verdadero, ya es verdadero
                return evalExpr(e->left) || evalExpr(e->right);

            case AlarmExprType::EXPR_NOT:
                return !evalExpr(e->child);
                
            default: return false;
        }
    }

    // --------------------------------------------------
    // Motor Principal de Evaluación (Reemplaza alarmEvaluate)
    // --------------------------------------------------
    void update() {
        // Obtenemos el vector dinámico de reglas
        auto& rules = getRules();

        // 🚀 OPTIMIZACIÓN: for-each sobre vector dinámico (Sin límites)
        for (auto& r : rules) {
            if (!r.expr) continue;

            // 1. Evaluar la condición lógica
            bool conditionActive = evalExpr(r.expr);
            
            bool wasActive = r.isActive;
            bool wasAcked = r.isAcknowledged;
            bool newActive = false;

            // 2. Lógica de LATCH / RETENCIÓN
            if (!r.latch) {
                newActive = conditionActive;
            } else {
                // Si es latch: se activa con la condición, 
                // pero solo se apaga si la condición cesa Y el operador dio ACK
                if (conditionActive) 
                    newActive = true;
                else if (wasAcked) 
                    newActive = false;
                else 
                    newActive = wasActive;
            }

            // 3. Cambio de estado detectado
            if (newActive != wasActive) {
                r.isActive = newActive;
                r.isAcknowledged = false; // Reset ACK al cambiar de estado
                r.lastTriggerMs = millis();

                // 🔥 ACCIÓN FÍSICA INMEDIATA (INTERLOCK)
                if (newActive && r.hasAction) {
                    ChipMgr::setRawValue(r.actionChipId, r.actionChannel, r.actionValue);
                }

                // 4. Registrar Evento (Para Logs, MQTT, etc.)
                // AlarmEvent ev = { r.alarmId, newActive, ... };
                // alarm_queuePush(ev);
                
                Serial.printf("[Alarm] ID:%d -> %s\n", r.alarmId, newActive ? "ON" : "OFF");
            }
        }
    }

} // namespace AlarmMgr