//LogSystem.h
#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>


// Definir el pin CS para la SD
#define SD_CS 5



// Solo debe existir esta declaración para escribir logs
void escribirLog(const char* formato, ...);

bool inicializarSD();
bool leerLog(void (*callback)(File&));
void borrarLog();
void leerLogConsola();

String obtenerLogSistema();
void limpiarLogSistema();


#endif
