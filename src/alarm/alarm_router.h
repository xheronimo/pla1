#pragma once
#include "alarm/alarm_struct.h"

namespace AlarmRouter {
    /**
     * @brief Procesa el evento y lo distribuye a los canales activos.
     * Reemplaza a 'emitirEvento'.
     */
    bool dispatch(const AlarmEvent& ev);

    // Funciones de envío (Stubs para completar según tus librerías)
    void enviarMQTT(const AlarmEvent& ev, const AlarmRule& rule);
    void enviarWeb(const AlarmEvent& ev, const AlarmRule& rule);
    void enviarTelegram(const AlarmEvent& ev, const AlarmRule& rule);
    void enviarSMS(const AlarmEvent& ev, const AlarmRule& rule);
}