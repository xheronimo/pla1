#pragma once

#include "config_struct.h"

// ==================================================
// CONFIGURACIÓN GLOBAL DEL SISTEMA
// ==================================================

// Variable global única de configuración
extern Configuracion cfg;


SystemMode parseSystemMode(const char* s);
