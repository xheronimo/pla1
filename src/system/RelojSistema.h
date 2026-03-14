#pragma once
#include <stdint.h>
#include <stddef.h>

void inicializarRTC();
void cargarHoraInicial();
void iniciarTaskNtp();

void obtenerISO8601(char* buffer, size_t size);
void obtenerHoraHHMM(char* buf);

void ajustarRelojInterno(
    uint16_t year,
    uint8_t  month,
    uint8_t  day,
    uint8_t  hour,
    uint8_t  minute,
    uint8_t  second
);
