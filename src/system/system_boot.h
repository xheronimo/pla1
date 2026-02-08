#pragma once
#include <stdint.h>

#include "config/config_struct.h"

// Detección
SystemMode detectSystemMode();

// Marcas de arranque
void markBootSuccess();

// Flags persistentes (opcional)
void setSafeModeFlag(bool enable);
bool systemFlagSafeMode();
