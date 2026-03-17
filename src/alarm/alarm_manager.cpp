#include "alarm/alarm_manager.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_evaluator.h"
#include "system/LogSystem.h"
#include <Arduino.h>
#include <vector>

namespace AlarmMgr {

    // Contenedor dinámico de reglas cargadas desde el JSON
    static std::vector<AlarmRule> _rules;

    /**
     * @brief Inicializa el motor de alarmas y la cola de eventos de FreeRTOS
     */
    void init() {
        _rules.clear();
        _rules.reserve(20); // Reservamos espacio inicial para evitar realocaciones
        alarm_queueInit(); 
        escribirLog("ALARM: Motor y Cola de eventos listos.");
    }

    void addRule(const AlarmRule& rule) {
        // Evitar duplicados por ID
        for (const auto& r : _rules) {
            if (r.alarmId == rule.alarmId) return;
        }
        _rules.push_back(rule);
    }

    void clearRules() {
        // Liberar memoria de los árboles de expresiones (árbol binario de la lógica)
        for (auto& rule : _rules) {
            if (rule.expr) {
                freeExpr(rule.expr); 
                rule.expr = nullptr;
            }
        }
        _rules.clear();
        escribirLog("ALARM: Reglas de memoria liberadas.");
    }

    /**
     * @brief Ciclo de evaluación de reglas (Motor Lógico).
     * Se llama desde taskAlarmRules en el Core 1.
     */
    void update() {
        for (auto& rule : _rules) {
            if (!rule.expr) continue;

            // 1. EVALUAR EXPRESIÓN (Llamada al evaluador con soporte de histéresis)
            // evalExpr recorre el árbol de la regla (ej: PT_01 > 10.5)
            bool conditionMet = evalExpr(rule.expr); 
            bool wasActive = rule.isActive;
            bool newActive = wasActive;

            // 2. LÓGICA DE ESTADO (Latch / Enclavamiento)
            if (conditionMet) {
                newActive = true;
            } else {
                // Si la condición física ya no se cumple:
                // Si NO tiene latch, se libera sola.
                // Si TIENE latch, solo se libera si el operario dio ACK.
                if (!rule.latch || rule.isAcknowledged) {
                    newActive = false;
                }
            }

            // 3. DETECCIÓN DE FLANCOS (Cambios de estado)
            if (newActive != wasActive) {
                rule.isActive = newActive;
                
                if (newActive) {
                    // --- ALARMA RECIÉN DISPARADA ---
                    rule.isAcknowledged = false;
                    rule.lastTriggerMs = millis();

                    // ACCIÓN FÍSICA INMEDIATA (Interlock de Seguridad)
                    if (rule.hasAction) {
                        bool res = ChipMgr::setRawValue(rule.actionChipId, rule.actionChannel, rule.actionValue);
                        escribirLog("INTERLOCK: %s CH:%d -> %.2f [%s]", 
                                   rule.actionChipId, rule.actionChannel, rule.actionValue, res ? "OK" : "FAIL");
                    }
                    
                    escribirLog("ALARM: ACTIVADA [%s] ID:%d", rule.name, rule.alarmId);
                } else {
                    // --- ALARMA RECUPERADA ---
                    escribirLog("ALARM: RECUPERADA [%s] ID:%d", rule.name, rule.alarmId);
                }

                // 4. NOTIFICACIÓN AL DISPATCHER (Cola asíncrona hacia MQTT/Web)
                AlarmEvent ev;
                ev.alarmId = rule.alarmId;
                ev.active = newActive;
                ev.kind = AlarmEventKind::STATE_CHANGE;
                
                if (!alarm_queuePush(ev)) {
                    escribirLog("ALARM: ERR - Cola de eventos llena. ID:%d perdido", rule.alarmId);
                }
            }
        }
    }

    void acknowledge(uint32_t alarmId) {
        for (auto& rule : _rules) {
            if (rule.alarmId == alarmId) {
                rule.isAcknowledged = true;
                escribirLog("ALARM: ACK recibido para ID:%d", alarmId);
                
                // Si el fallo físico ya no existe, el ACK la desactiva inmediatamente
                if (!evalExpr(rule.expr)) {
                    rule.isActive = false;
                    AlarmEvent ev = { rule.alarmId, false, AlarmEventKind::STATE_CHANGE };
                    alarm_queuePush(ev);
                }
                break;
            }
        }
    }

    AlarmRule* getById(uint32_t id) {
        for (auto& rule : _rules) {
            if (rule.alarmId == id) return &rule;
        }
        return nullptr;
    }

    std::vector<AlarmRule>& getRules() {
        return _rules;
    }

    uint32_t getActiveCount() {
        uint32_t count = 0;
        for (const auto& rule : _rules) {
            if (rule.isActive) count++;
        }
        return count;
    }

} // namespace AlarmMgr