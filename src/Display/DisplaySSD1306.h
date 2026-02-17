#pragma once
#include <stdint.h>


enum DisplayPage
{
    PAGE_GENERAL = 0,
    PAGE_SYSTEM,
    PAGE_ALARMS,
    PAGE_MODBUS,
    PAGE_FAULT,     // SOLO si hubo reset WDT
    PAGE_MAX_NORMAL = PAGE_MODBUS + 1
};

// Inicialización
bool displayInit();
bool displayActivo();

// Uso en arranque
void displayBootPrint(const char* msg);

// Uso normal
void displayClear();
void displayLoop();




