#include "modbus_guard.h"
#include "system/LogSystem.h"
#include <Arduino.h>

extern bool g_modbusDegraded;

namespace ModbusGuard {

    // Configuración de tiempos de recuperación
    const uint32_t RECOVERY_ATTEMPT_MS = 300000; // 5 minutos (300.000 ms)
    const uint32_t BASE_RETRY_MS = 2000;         // 2 segundos base

    bool canRead(ModbusDevice& dev) {
        uint32_t now = millis();

        // 1. SI ESTÁ DESACTIVADO MANUALMENTE: No leer nunca.
        // (Asumimos que 'provisioning' o una orden externa es 'enabled = false')
        if (dev.provisioning) return false;

        // 2. LÓGICA DE AUTO-RECOVERY: 
        // Si el dispositivo fue auto-desactivado por errores (enabled=false e isDynamic=true/false)
        // le damos una oportunidad cada 5 minutos.
        if (!dev.enabled) {
            if (now - dev.lastPollMs >= RECOVERY_ATTEMPT_MS) {
                LOG_INF("MODBUS: Intentando auto-recuperación de ID %u...", dev.id);
                return true; // Intentamos una lectura de sonda
            }
            return false;
        }

        // 3. LÓGICA DE BACKOFF (Para dispositivos con errores temporales)
        if (dev.state == ModbusDeviceState::STATE_TIMEOUT || 
            dev.state == ModbusDeviceState::STATE_FAULT) {
            
            // Reintento exponencial: 2s, 4s, 8s... hasta un máximo de 30s
            uint32_t retryDelay = BASE_RETRY_MS * (dev.errorCount > 15 ? 15 : dev.errorCount);
            
            if (now - dev.lastPollMs >= retryDelay) {
                return true;
            }
            return false;
        }

        return true; // Estado OK, se puede leer normalmente
    }

    void reportSuccess(ModbusDevice& dev) {
        // Si el dispositivo estaba desactivado y logramos leerlo, lo reactivamos
        if (!dev.enabled) {
            dev.enabled = true;
            LOG_INF("MODBUS: Dispositivo ID %u RECUPERADO con éxito", dev.id);
        }

        dev.state = ModbusDeviceState::STATE_OK;
        dev.errorCount = 0;
        dev.lastOkMs = millis();
        
        // Aquí podrías añadir lógica para quitar g_modbusDegraded si todos los chips están OK
    }

    void reportError(ModbusDevice& dev) {
        dev.errorCount++;
        
        // A partir de 3 errores, marcamos como Timeout
        if (dev.errorCount >= 3) {
            dev.state = ModbusDeviceState::STATE_TIMEOUT;
            
            if (!g_modbusDegraded) {
                g_modbusDegraded = true;
                LOG_ERR("MODBUS: Bus degradado por ID %u", dev.id);
            }
        }

        // A los 20 errores, lo "dormimos" (Auto-Disable) para no ralentizar el bus
        if (dev.errorCount >= 20) {
            if (dev.enabled) {
                dev.enabled = false; 
                LOG_ERR("MODBUS: ID %u desactivado por errores críticos. Entrando en modo Auto-Recovery.", dev.id);
            }
        }
    }
}