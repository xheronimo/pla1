#include "signal_parser.h"
#include <string.h>

SignalKind parseSignalKind(const char* str)
{
    if (!str) return SignalKind::SENSOR_DIGITAL;

    if (strcmp(str, "analog") == 0)
        return SignalKind::SENSOR_ANALOG;

    return SignalKind::SENSOR_DIGITAL;
}

BusType parseBusType(const char* str)
{
    if (!str) return BusType::BUS_GPIO;

    if (strcmp(str, "i2c") == 0)   return BusType::BUS_I2C;
    if (strcmp(str, "gpio") == 0)  return BusType::BUS_GPIO;
    if (strcmp(str, "modbus") == 0)return BusType::BUS_MODBUS;

    return BusType::BUS_GPIO;
}

I2CDevice parseI2CDevice(const char* str)
{
    if (!str) return I2CDevice::NONE;

    if (strcmp(str, "ads1115") == 0) return I2CDevice::ADS1115;
    if (strcmp(str, "ina219")  == 0) return I2CDevice::INA219;
    if (strcmp(str, "sht31")   == 0) return I2CDevice::SHT31;
    if (strcmp(str, "ens160")  == 0) return I2CDevice::ENS160;
    if (strcmp(str, "bme280")  == 0) return I2CDevice::BME280;
    if (strcmp(str, "bh1750")  == 0) return I2CDevice::BH1750;
    if (strcmp(str, "ccs811")  == 0) return I2CDevice::CCS811;
    if (strcmp(str, "pcf8574") == 0) return I2CDevice::PCF8574;
    if (strcmp(str, "pcf8575") == 0) return I2CDevice::PCF8575;

    return I2CDevice::NONE;
}

