#include "signal_read_i2cc.h"

#include "i2c/i2c_chip.h"
#include "i2c/pcf_driver.h"
#include "i2c/ads1115_driver.h"
#include "i2c/driver_bh1750.h"
#include "i2c/ina219_driver.h"
#include "i2c/sht31_driver.h"
#include "i2c/ccs811_driver.h"


bool leerSignalI2C(const Signal &s, float &out)
{
    switch (s.chip)
    {
    case I2CDevice::PCF8574:      
    case I2CDevice::PCF8575:
      return leerSignalPCF_User(s, out);
    case I2CDevice::ADS1115:  return leerSignalADS1115(s, out);
    case I2CDevice::INA219:   return leerSignalINA219(s, out);
    case I2CDevice::SHT31:    return leerSignalSHT31(s, out);
    case I2CDevice::CCS811:   return leerSignalCCS811(s, out);        
    case I2CDevice::ENS160: return leerSignalENS160(s, out);
    case I2CDevice::BMe280: return leerSignalBME280(s, out);
    default:
        return false;
    }
}

