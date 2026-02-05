#pragma once
#include <Arduino.h>

struct DigitalSensorConfig
{
    String   id;
    String   nombre;

    enum Tipo {
        GPIO,
        PCF,
        MODBUS
    } tipo;

    uint8_t  chip;      // PCF index
    uint8_t  pin;

    bool     invertido;
    uint32_t histeresisMs;
    bool     latch;
};

class DigitalSensor
{
public:
    explicit DigitalSensor(const DigitalSensorConfig& cfg);

    void update();
    bool estado() const;
    bool flancoOn() const;
    bool flancoOff() const;
    void resetLatch();

    const String& getId() const { return config.id; }

private:
    DigitalSensorConfig config;

    bool estadoActual = false;
    bool estadoPrevio = false;
    bool latched      = false;

    uint32_t tCambio  = 0;

    bool leerEntrada() const;
};
