#pragma once

#include <ArduinoJson.h>
#include "config/config_global.h"

// ==================================================
// SERIALIZER DE CONFIGURACIÓN (RAM → JSON)
// ==================================================

bool saveConfigToJson(
    JsonDocument& doc,
    const Configuracion& cfg
);
