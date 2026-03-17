bool escribirSignal(Signal& s, float value)
{
    if (!s.writable) return false;

    bool ok = false;
    uint32_t now = millis();

    switch (s.bus)
    {
        case BusType::BUS_GPIO:
        {
            digitalWrite(s.channel, value > 0.5f ? HIGH : LOW);
            ok = true;
            break;
        }

        case BusType::BUS_I2C:
        {
            // 1. Obtener contexto y driver
            ChipContext* ctx = i2cGetChipContext(s.chipId, s.address);
            const I2CChipDriver* drv = i2cGetDriver(s.chipId);
            
            if (!ctx || !drv || !drv->write) return false;

            // 2. Guardia de seguridad pre-lectura/escritura
            if (!i2cChipGuardBeforeRead(ctx, now)) return false;

            // 3. 🔒 Tomar control del bus I2C
            if (xSemaphoreTake(semI2C, pdMS_TO_TICKS(100)) == pdTRUE) 
            {
                ok = drv->write(s, value);
                xSemaphoreGive(semI2C);

                if (ok) {
                    i2cChipGuardOnSuccess(ctx, now);
                } else {
                    // --- AQUÍ INTEGRAMOS EL ChipMode ---
                    // Obtenemos el modo (OUTPUT_ONLY, MIXED, etc.) desde la metadata del driver
                    ChipMetadata meta;
                    drv->meta(meta); 
                    
                    // Ahora sí, llamamos a la guarda con el segundo parámetro corregido
                    i2cChipGuardOnError(ctx, meta.mode); 
                }
            }
            break;
        }

        case BusType::BUS_MODBUS:
        {
            ok = ModbusManager::writeHolding(s.address, s.channel, (uint16_t)value);
            break;
        }

        default:
            return false;
    }

    // Actualización de los flags de la señal (Idéntico a antes)
    if (ok) {
        s.prev = s.value;
        s.value = value;
        s.raw = value;
        s.valid = true;
        s.error = false;
        s.errorCount = 0;
        s.lastOkTs = now;
    } else {
        s.error = true;
        s.valid = false;
        s.errorCount++;
    }

    return ok;
}