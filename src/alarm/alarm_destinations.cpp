#include "alarm/alarm_destinations.h"
#include "system/LogSystem.h"
#include <string.h>

// Instanciación de las variables globales
AlarmDestination alarmDestinations[MAX_ALARM_DESTINATIONS];
uint8_t alarmDestinationCount = 0;

namespace AlarmDestMgr {

    void init() {
        clearAll();
        escribirLog("ALARM: Gestor de destinos inicializado.");
    }

    void clearAll() {
        memset(alarmDestinations, 0, sizeof(alarmDestinations));
        alarmDestinationCount = 0;
    }

    bool addDestination(const AlarmDestination& dest) {
        if (alarmDestinationCount >= MAX_ALARM_DESTINATIONS) {
            escribirLog("ALARM: Error - Máximo de destinos alcanzado.");
            return false;
        }

        // Copiamos la estructura al array global
        alarmDestinations[alarmDestinationCount] = dest;
        
        escribirLog("ALARM: Destino añadido [%s] - Canales: %d", 
                    dest.nombre, dest.canales);
        
        alarmDestinationCount++;
        return true;
    }
}