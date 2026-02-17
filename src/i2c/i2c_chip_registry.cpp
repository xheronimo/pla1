#include "i2c_chip_registry.h"
#include "i2c_bus.h"
#include "signal/signal_manager.h"
#include "system/LogSystem.h"

#include <cstddef>

extern Signal signalTable[];
extern size_t signalCount;

// =====================================================
// DETECT FUNCTIONS (implementadas en cada driver)
// =====================================================

bool ads1115Detect(uint8_t addr);
bool ina219Detect(uint8_t addr);
bool sht31Detect(uint8_t addr);
bool ens160Detect(uint8_t addr);
bool bme280Detect(uint8_t addr);
bool bh1750Detect(uint8_t addr);
bool ccs811Detect(uint8_t addr);
bool pcfDetect8574(uint8_t addr);
bool pcfDetect8575(uint8_t addr);

// =====================================================
// DRIVER FUNCTIONS
// =====================================================

bool ads1115Init(uint8_t addr, uint8_t options);
bool ads1115ReadSignal(const Signal& s, float& out);
void ads1115GetMetadata(ChipMetadata& meta);

bool ina219Init(uint8_t addr, uint8_t options);
bool ina219ReadSignal(const Signal& s, float& out);
void ina219GetMetadata(ChipMetadata& meta);

bool sht31Init(uint8_t addr, uint8_t options);
bool sht31ReadSignal(const Signal& s, float& out);
void sht31GetMetadata(ChipMetadata& meta);

bool ens160Init(uint8_t addr, uint8_t options);
bool ens160ReadSignal(const Signal& s, float& out);
void ens160GetMetadata(ChipMetadata& meta);

bool bme280Init(uint8_t addr, uint8_t options);
bool bme280ReadSignal(const Signal& s, float& out);
void bme280GetMetadata(ChipMetadata& meta);

bool bh1750Init(uint8_t addr, uint8_t options);
bool bh1750ReadSignal(const Signal& s, float& out);
void bh1750GetMetadata(ChipMetadata& meta);

bool ccs811Init(uint8_t addr, uint8_t options);
bool ccs811ReadSignal(const Signal& s, float& out);
void ccs811GetMetadata(ChipMetadata& meta);


bool pcfInit(uint8_t address, uint8_t options);
bool pcfReadSignal(const Signal& s, float& out);
bool pcfWriteSignal(const Signal& s, float value);
void pcf8574GetMetadata(ChipMetadata& meta);
void pcf8575GetMetadata(ChipMetadata& meta);

// =====================================================
// TABLA CENTRAL DE DRIVERS
// =====================================================

static const I2CChipDriver drivers[] =
{
    { I2CDevice::ADS1115, ads1115Detect, ads1115Init, ads1115ReadSignal,nullptr ,ads1115GetMetadata, true },
    { I2CDevice::INA219,  ina219Detect,  ina219Init,  ina219ReadSignal,nullptr  ,ina219GetMetadata,  true },
    { I2CDevice::SHT31,   sht31Detect,   sht31Init,   sht31ReadSignal, nullptr  ,sht31GetMetadata,   true },
    { I2CDevice::ENS160,  ens160Detect,  ens160Init,  ens160ReadSignal,nullptr  ,ens160GetMetadata,  true },
    { I2CDevice::BME280,  bme280Detect,  bme280Init,  bme280ReadSignal,nullptr  ,bme280GetMetadata,  true },
    { I2CDevice::BH1750,  bh1750Detect,  bh1750Init,  bh1750ReadSignal,nullptr  ,bh1750GetMetadata,  true },
    { I2CDevice::CCS811,  ccs811Detect,  ccs811Init,  ccs811ReadSignal,nullptr  ,ccs811GetMetadata,  true },

    // Expansores fijos de placa → NO autodetect
    { I2CDevice::PCF8574, pcfDetect8574, pcfInit, pcfReadSignal, pcfWriteSignal, pcf8574GetMetadata, false },
    { I2CDevice::PCF8575, pcfDetect8575, pcfInit, pcfReadSignal, pcfWriteSignal, pcf8575GetMetadata, false },
};

static const size_t driverCount =
    sizeof(drivers) / sizeof(drivers[0]);

// =====================================================
// GET DRIVER POR TIPO
// =====================================================

const I2CChipDriver* i2cGetDriver(I2CDevice type)
{
    for (size_t i = 0; i < driverCount; i++)
    {
        if (drivers[i].type == type)
            return &drivers[i];
    }

    return nullptr;
}

// =====================================================
// GET DRIVER NAME
// =====================================================

const char* i2cGetDriverName(I2CDevice type)
{
    switch (type)
    {
        case I2CDevice::ADS1115: return "ADS1115";
        case I2CDevice::INA219:  return "INA219";
        case I2CDevice::SHT31:   return "SHT31";
        case I2CDevice::ENS160:  return "ENS160";
        case I2CDevice::BME280:  return "BME280";
        case I2CDevice::BH1750:  return "BH1750";
        case I2CDevice::CCS811:  return "CCS811";
        case I2CDevice::PCF8574: return "PCF8574";
        case I2CDevice::PCF8575: return "PCF8575";
        default: return "UNKNOWN";
    }
}



// =====================================================
// ¿DIRECCIÓN YA CONFIGURADA?
// Evita duplicados al autodetectar
// =====================================================

bool i2cIsAddressConfigured(uint8_t addr)
{
    for (size_t i = 0; i < signalCount; i++)
    {
        const Signal& s = signalTable[i];

        if ((s.bus == BusType::BUS_I2C ||
             s.bus == BusType::BUS_PCF) &&
            s.address == addr)
        {
            return true;
        }
    }

    return false;
}
size_t i2cGetDriverCount()
{
    return driverCount;
}

const I2CChipDriver* i2cGetDriverByIndex(size_t index)
{
    if (index >= driverCount)
        return nullptr;

    return &drivers[index];
}



