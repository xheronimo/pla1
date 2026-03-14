#pragma once
#include <stdint.h>

#define MAX_DIGITALES   16
#define MAX_ANALOGICOS   8
#define MAX_MODBUS      16

struct DatosReales
{
    // ---------------- DIGITALES ----------------
    bool entradasDigitales[MAX_DIGITALES];
    bool errDigitales[MAX_DIGITALES];

    // ---------------- ANALÓGICOS ----------------
    float analogicos[MAX_ANALOGICOS];
    bool  errAnalogicos[MAX_ANALOGICOS];

    // ---------------- MODBUS ----------------
    float modbus[MAX_MODBUS];
    bool  errModbus[MAX_MODBUS];
};

extern DatosReales datosReales;
