// En modbus_config.h, solo dejamos parámetros de comportamiento
namespace ModbusCfg {
    const uint32_t DEFAULT_TIMEOUT_MS = 200;
    const uint32_t DEFAULT_POLL_MS    = 1000;
    const uint8_t  MAX_RETRYS         = 3;
    const uint32_t BACKOFF_MAX_MS     = 60000; // 1 minuto de pausa si falla mucho
}