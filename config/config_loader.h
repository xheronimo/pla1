#pragma once
#include "config_struct.h"
#include <ArduinoJson.h>

// Tu parser existente
bool loadConfigFromJson(Configuracion& cfg, const JsonDocument& doc);

// Nuevo loader robusto
bool cargarConfiguracion(Configuracion& cfg);
