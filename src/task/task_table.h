#pragma once
#include <freertos/FreeRTOS.h>   // SIEMPRE EL PRIMERO
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "config/config_struct.h"

void arrancarTareasSistema(const Configuracion& cfg);
