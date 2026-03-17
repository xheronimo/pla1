#pragma once

#include <FS.h>

namespace AlarmMgr {

    /**
     * @brief Carga las reglas de alarma desde un archivo JSON en el sistema de archivos.
     * * Esta función limpia el vector actual de reglas, lee el archivo especificado,
     * construye los árboles de expresiones recursivos y los añade al AlarmManager.
     * * @param fs Referencia al sistema de archivos (ej: SD o LittleFS)
     * @param path Ruta del archivo (ej: "/config/alarms.json")
     * @return true si la carga fue exitosa, false si hubo errores de parseo o lectura.
     */
    bool loadFromFS(FS &fs, const char* path);

    /**
     * @brief (Opcional) Guarda las reglas actuales del AlarmManager en un archivo JSON.
     * Útil si permites editar alarmas desde la interfaz Web y quieres persistir los cambios.
     */
    bool saveToFS(FS &fs, const char* path);

} // namespace AlarmMgr