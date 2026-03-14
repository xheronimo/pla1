#include "alarm/alarm_manager.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "alarm/alarm_queue.h"
#include "alarm/alarm_evaluator.h" // Importante para la evaluación con histéresis
#include <Arduino.h>

namespace AlarmMgr {

    // Contenedor dinámico: Memoria eficiente y sin límites fijos
    static std::vector<AlarmRule> _rules;

    /**
     * @brief Inicializa el motor de alarmas y la cola de eventos
     */
    void init() {
        _rules.clear();
        alarm_queueInit(); // Inicializamos la cola de FreeRTOS para el Dispatcher
        Serial.println("[AlarmMgr] Motor y Cola de eventos inicializados.");
    }

    void addRule(const AlarmRule& rule) {
        // Evitar duplicados por ID
        for (const auto& r : _rules) {
            if (r.alarmId == rule.alarmId) return;
        }
        _rules.push_back(rule);
    }

    void clearRules() {
        // Liberar memoria de los árboles de expresiones antes de limpiar el vector
        for (auto& rule : _rules) {
        if (rule.expr) {
            freeExpr(rule.expr); // Usamos la utilidad rescatada
            rule.expr = nullptr;
        }
    }
    _rules.clear();
    }

    /**
     * @brief Ciclo de evaluación de reglas.
     * Se llama desde la tarea PLC_Scan después de actualizar las señales.
     */
    void update() {
        for (auto& rule : _rules) {
            if (!rule.expr) continue;

            // 1. Evaluar expresión usando el Evaluador (gestiona histéresis internamente)
            bool conditionMet = evalExpr(rule.expr); 
            bool wasActive = rule.isActive;
            bool newActive = wasActive;

            // 2. Lógica de ESTADO (Gestión de Latch y Activación)
            if (conditionMet) {
                newActive = true;
            } else {
                // Si la condición ya no se cumple:
                // Si NO tiene latch, se desactiva. 
                // Si TIENE latch, solo se desactiva si el usuario le dio a ACK.
                if (!rule.latch || rule.isAcknowledged) {
                    newActive = false;
                }
            }

            // 3. DETECCIÓN DE CAMBIOS (Disparo o Recuperación)
            if (newActive != wasActive) {
                rule.isActive = newActive;
                
                if (newActive) {
                    // --- ALARMA RECIÉN ACTIVADA ---
                    rule.isAcknowledged = false;
                    rule.lastTriggerMs = millis();

                    // ACCIÓN FÍSICA INMEDIATA (Interlock)
                    if (rule.hasAction) {
                        ChipMgr::setRawValue(rule.actionChipId, rule.actionChannel, rule.actionValue);
                        Serial.printf("[AlarmMgr] INTERLOCK: %s CH:%d -> %.2f\n", 
                                      rule.actionChipId, rule.actionChannel, rule.actionValue);
                    }
                    Serial.printf("[AlarmMgr] ACTIVADA: ID %d [%s]\n", rule.alarmId, rule.name.c_str());
                } else {
                    // --- ALARMA RECUPERADA ---
                    Serial.printf("[AlarmMgr] RECUPERADA: ID %d [%s]\n", rule.alarmId, rule.name.c_str());
                }

                // 4. NOTIFICACIÓN AL DISPATCHER (Cola de eventos)
                AlarmEvent ev;
                ev.alarmId = rule.alarmId;
                ev.active = newActive;
                ev.kind = AlarmEventKind::STATE_CHANGE;
                
                if (!alarm_queuePush(ev)) {
                    Serial.printf("[AlarmMgr] ERROR: Cola llena, evento ID %d perdido\n", rule.alarmId);
                }
            }
        }
    }

    void acknowledge(uint32_t alarmId) {
        for (auto& rule : _rules) {
            if (rule.alarmId == alarmId) {
                rule.isAcknowledged = true;
                // Si el problema ya desapareció físicamente, la desactivamos de inmediato
                if (!evalExpr(rule.expr)) {
                    rule.isActive = false;
                    
                    // Notificamos la recuperación por ACK al dispatcher
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