#ifndef ALARM_DESTINATIONS_SERIALIZER_H
#define ALARM_DESTINATIONS_SERIALIZER_H

#include <ArduinoJson.h>

/**
 * @brief Carga los destinos de alarma desde un array JSON (leído de la SD).
 */
bool alarmDestinationsLoadFromJson(const JsonArray& arr);

/**
 * @brief Vuelca los destinos actuales a un documento JSON para persistencia.
 */
bool alarmDestinationsSaveToJson(JsonDocument& doc);

#endif