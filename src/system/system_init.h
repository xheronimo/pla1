#pragma once

#include "config/config_struct.h"

// Inicialización de hardware base (no depende de config)
void systemInitBase();

// Inicialización de servicios dependientes de config
void systemInitFromConfig(const Configuracion& cfg);
