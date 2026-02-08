#include "sensor_manager.h"
#include "datos_reales.h"

#include <Arduino.h>

#include "system/WatchdogManager.h"
#include "system/LogSystem.h"
#include "net/ModbusManager.h"

// ==================================================
// INSTANCIA GLOBAL
// ==================================================
DatosReales datosReales;

// ==================================================
// INIT
// ==================================================
void sensorManagerInit()
{
    memset(&datosReales, 0, sizeof(datosReales));
    escribirLog("SENS:INIT");
}

// ==================================================
// LECTURA DIGITAL
// ==================================================
static void leerDigitales()
{
    for (int i = 0; i < MAX_DIGITALES; i++)
    {
        datosReales.entradasDigitales[i] = digitalRead(i);
        datosReales.errDigitales[i] = false; // aquí luego validaciones
    }
}

// ==================================================
// LECTURA ANALÓGICA
// ==================================================
static void leerAnalogicos()
{
    for (int i = 0; i < MAX_ANALOGICOS; i++)
    {
        int raw = analogRead(i);
        datosReales.analogicos[i] = raw * (3.3f / 4095.0f);
        datosReales.errAnalogicos[i] = false;
    }
}

// ==================================================
// MODBUS
// ==================================================
static void leerModbus()
{
    for (int i = 0; i < MAX_MODBUS; i++)
    {
        if (ModbusManager::leerRegistro(i, datosReales.modbus[i]))
            datosReales.errModbus[i] = false;
        else
            datosReales.errModbus[i] = true;
    }
}

// ==================================================
// UPDATE CÍCLICO
// ==================================================
void sensorManagerUpdate()
{
    leerDigitales();
    leerAnalogicos();
    leerModbus();

    watchdogKick(WDT_SENSORS);
}
