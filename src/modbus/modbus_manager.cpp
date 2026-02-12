#include "modbus_manager.h"

static HardwareSerial* serial = nullptr;
static uint8_t dePin;

static uint16_t crc16(const uint8_t* buf, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
    }
    return crc;
}

void ModbusManager::init(HardwareSerial* port, uint8_t pin)
{
    serial = port;
    dePin = pin;
}

static bool sendRequest(
    uint8_t slave,
    uint8_t func,
    uint16_t reg,
    uint16_t count,
    uint16_t& out
)
{
    if (!serial) return false;

    uint8_t frame[8];
    frame[0] = slave;
    frame[1] = func;
    frame[2] = reg >> 8;
    frame[3] = reg & 0xFF;
    frame[4] = count >> 8;
    frame[5] = count & 0xFF;

    uint16_t crc = crc16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;

    // TX
    digitalWrite(dePin, HIGH);
    delayMicroseconds(10);
    serial->write(frame, 8);
    serial->flush();
    digitalWrite(dePin, LOW);

    // RX
    uint8_t resp[7];
    uint32_t t0 = millis();
    uint8_t idx = 0;

    while (millis() - t0 < 200) {
        if (serial->available()) {
            resp[idx++] = serial->read();
            if (idx >= 7) break;
        }
    }

    if (idx < 7) return false;

    uint16_t crcRx = (resp[6] << 8) | resp[5];
    if (crc16(resp, 5) != crcRx) return false;

    out = (resp[3] << 8) | resp[4];
    return true;
}

bool ModbusManager::readHolding(uint8_t slave, uint16_t reg, uint16_t& out)
{
    return sendRequest(slave, 0x03, reg, 1, out);
}

bool ModbusManager::readInput(uint8_t slave, uint16_t reg, uint16_t& out)
{
    return sendRequest(slave, 0x04, reg, 1, out);
}
