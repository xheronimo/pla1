#pragma once
#include "config_struct.h"
#include <ArduinoJson.h>

// Tu parser existente
bool loadConfigFromJson(Configuracion& cfg, const JsonObjectConst& root);

// Nuevo loader robusto
bool cargarConfiguracion(Configuracion& cfg, SystemMode mode);
