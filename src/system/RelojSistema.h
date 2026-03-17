#ifndef RELOJ_SISTEMA_H
#define RELOJ_SISTEMA_H

#include <Arduino.h>
#include <time.h>

namespace RelojSistema {
    void inicializar();
    void cargarHoraInicial();
    void iniciarTaskNTP();
    
    // Getters de tiempo formateado
    void obtenerISO8601(char* buffer, size_t size);
    void obtenerHoraHHMM(char* buffer);
    
    // Utilidades
    bool horaValida(time_t t);
    void ajustarRelojInterno(uint16_t year, uint8_t month, uint8_t day, 
                            uint8_t hour, uint8_t minute, uint8_t second);
}

#endif