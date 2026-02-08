#pragma once
#include <stdint.h>

// Inicialización
bool displayInit();
bool displayActivo();

// Uso en arranque
void displayBootPrint(const char* msg);

// Uso normal
void displayClear();
void displayLoop();

// Task FreeRTOS
void taskDisplay(void*);
