#include "modbus_decode.h"
#include "signal/signal_struct.h"
#include "modbus_utils.h"
#include <stdint.h>
#include <string.h>

static uint32_t mergeWords(uint16_t w1, uint16_t w2, ModbusWordOrder order)
{
    uint32_t value = 0;

    switch (order)
    {
        case ModbusWordOrder::AB:
            value = ((uint32_t)w1 << 16) | w2;
            break;

        case ModbusWordOrder::BA:
            value = ((uint32_t)w2 << 16) | w1;
            break;

        case ModbusWordOrder::CDAB:
            value = ((uint32_t)(w1 & 0xFF) << 24) |
                    ((uint32_t)(w1 >> 8) << 16) |
                    ((uint32_t)(w2 & 0xFF) << 8) |
                    (w2 >> 8);
            break;
    }

    return value;
}





bool modbusDecodeValue(const uint16_t* data,
                       const Signal& s,
                       float& out)
{
    uint8_t expected = modbusExpectedWordCount(s.dataType);

    if (s.wordCount != expected)
        return false;

    if (expected == 1)
    {
        uint16_t v = data[0];

        if (s.dataType == ModbusDataType::INT16)
            out = (int16_t)v;
        else
            out = v;

        return true;
    }

    // -------- 32 bits --------

    uint32_t value = 0;

    if (s.wordOrder == ModbusWordOrder::AB)
        value = ((uint32_t)data[0] << 16) | data[1];

    else if (s.wordOrder == ModbusWordOrder::BA)
        value = ((uint32_t)data[1] << 16) | data[0];

    else // CDAB
        value = ((uint32_t)data[1] << 16) | data[0];

    if (s.dataType == ModbusDataType::FLOAT32)
    {
        float f;
        memcpy(&f, &value, sizeof(float));
        out = f;
        return true;
    }

    if (s.dataType == ModbusDataType::INT32)
        out = (int32_t)value;
    else
        out = value;

    return true;
}
