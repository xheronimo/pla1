#pragma once
#include "config/config_struct.h"


// Hardware físico (no depende de config)
void initHardwareBasico();

// Hardware lógico (depende de config)
void initHardwareConfigurado(const Configuracion& cfg);
