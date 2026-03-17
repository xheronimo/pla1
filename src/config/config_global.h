#pragma once

#include "config_struct.h"

#define I2C_SIMULATION 1
 // cambiar a 0 cuando este el hardware

// ==================================================
// CONFIGURACIÓN GLOBAL DEL SISTEMA
// ==================================================

// Variable global única de configuración
extern Configuracion cfg;


SystemMode parseSystemMode(const char* s);
