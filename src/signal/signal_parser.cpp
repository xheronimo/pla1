#include "signal/signal_parser.h"
#include <string.h>
#include <strings.h>

// ======================================================
// STRING -> ENUM
// ======================================================

SignalKind parseSignalKind(const char* str) {
    if (!str) return SignalKind::SENSOR_DIGITAL;

    if (strcasecmp(str, "analog") == 0)   return SignalKind::SENSOR_ANALOG;
    if (strcasecmp(str, "digital") == 0)  return SignalKind::SENSOR_DIGITAL;
    if (strcasecmp(str, "actuator_di") == 0 || strcasecmp(str, "relay") == 0) 
        return SignalKind::ACTUATOR_DIGITAL;
    if (strcasecmp(str, "actuator_an") == 0 || strcasecmp(str, "vfd") == 0) 
        return SignalKind::ACTUATOR_ANALOG;
    
    return SignalKind::SENSOR_DIGITAL;
}

BusType parseBusType(const char* str) {
    if (!str) return BusType::BUS_NONE;

    if (strcasecmp(str, "gpio") == 0)      return BusType::BUS_GPIO;
    if (strcasecmp(str, "i2c") == 0)       return BusType::BUS_I2C;
    if (strcasecmp(str, "onewire") == 0)   return BusType::BUS_ONEWIRE;
    if (strcasecmp(str, "dht11") == 0)     return BusType::BUS_DHT11;
    if (strcasecmp(str, "dht22") == 0)     return BusType::BUS_DHT22;
    if (strcasecmp(str, "modbus") == 0)    return BusType::BUS_MODBUS;
    if (strcasecmp(str, "virtual") == 0)   return BusType::BUS_VIRTUAL;
    if (strcasecmp(str, "mqtt") == 0)      return BusType::BUS_MQTT;
    if (strcasecmp(str, "telegram") == 0)  return BusType::BUS_TELEGRAM;
    if (strcasecmp(str, "sms") == 0)       return BusType::BUS_SMS;
    if (strcasecmp(str, "broadcast") == 0) return BusType::BUS_BROADCAST;
    if (strcasecmp(str, "can") == 0)       return BusType::BUS_CAN;

    return BusType::BUS_NONE;
}

SignalMode parseSignalMode(const char* str) {
    if (!str) return SignalMode::ACTIVE;

    if (strcasecmp(str, "active") == 0)         return SignalMode::ACTIVE;
    if (strcasecmp(str, "test") == 0)           return SignalMode::TEST;
    if (strcasecmp(str, "mantenimiento") == 0)  return SignalMode::MANTENIMIENTO; // <--- Añadido
    if (strcasecmp(str, "offline") == 0)        return SignalMode::OFFLINE;

    return SignalMode::ACTIVE;
}

ModbusRegType parseModbusRegType(const char* str) {
    if (!str) return ModbusRegType::MODBUS_HOLDING;

    if (strcasecmp(str, "coil") == 0)     return ModbusRegType::MODBUS_COIL;
    if (strcasecmp(str, "discrete") == 0) return ModbusRegType::MODBUS_DISCRETE_INPUT;
    if (strcasecmp(str, "holding") == 0)  return ModbusRegType::MODBUS_HOLDING;
    if (strcasecmp(str, "input") == 0)    return ModbusRegType::MODBUS_INPUT;

    return ModbusRegType::MODBUS_HOLDING;
}

// ======================================================
// ENUM -> STRING
// ======================================================

const char* signalModeToString(SignalMode mode) {
    switch (mode) {
        case SignalMode::ACTIVE:         return "active";
        case SignalMode::TEST:           return "test";
        case SignalMode::MANTENIMIENTO: return "mantenimiento";
        case SignalMode::OFFLINE:        return "offline";
        default: return "active";
    }
}

const char* busTypeToString(BusType bus) {
    switch (bus) {
        case BusType::BUS_GPIO:      return "gpio";
        case BusType::BUS_I2C:       return "i2c";
        case BusType::BUS_ONEWIRE:   return "onewire";
        case BusType::BUS_DHT11:     return "dht11";
        case BusType::BUS_DHT22:     return "dht22";
        case BusType::BUS_MODBUS:    return "modbus";
        case BusType::BUS_VIRTUAL:   return "virtual";
        case BusType::BUS_MQTT:      return "mqtt";
        case BusType::BUS_TELEGRAM:  return "telegram";
        case BusType::BUS_SMS:       return "sms";
        case BusType::BUS_BROADCAST: return "broadcast";
        case BusType::BUS_CAN:       return "can";
        default: return "none";
    }
}