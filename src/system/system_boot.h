#pragma once
#include <stdint.h>
#include <esp_system.h>

#include "config/config_struct.h"

// Detección
SystemMode detectSystemMode();

// Marcas de arranque
void markBootSuccess();

// Flags persistentes (opcional)
void setSafeModeFlag(bool enable);
bool systemFlagSafeMode();

esp_reset_reason_t systemGetLastReset();

void systemBootInit();
