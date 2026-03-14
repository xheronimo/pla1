#pragma once

#include "chip_struct.h"
#include <string>
#include <vector>

/**
 * @brief Gestor de Chips Físicos (Capa de Abstracción de Hardware)
 * * Este namespace organiza todos los dispositivos (Chips) conectados a los distintos
 * buses y actúa como una memoria compartida (caché) entre los drivers de hardware
 * y la lógica de señales del PLC.
 */
namespace ChipMgr {

    // ======================================================
    // GESTIÓN DE CHIPS (Inventario)
    // ======================================================

    /** * @brief Busca un chip en el registro por su ID único.
     * @param id Identificador de texto del chip (ej: "MCP23017_0").
     * @return Puntero al chip o nullptr si no existe.
     */
    Chip* getById(const std::string& id);
    
    /** * @brief Añade un nuevo chip al registro o lo actualiza si ya existe.
     */
    void add(const Chip& c);

    // ======================================================
    // GESTIÓN DE CANALES (Asignación)
    // ======================================================

    /** * @brief Reserva un canal físico para una señal lógica.
     * @return true si el canal estaba libre y se pudo reservar.
     */
    bool markChannelUsed(const std::string& chipId, int channel, const std::string& signalId);
    
    /** * @brief Libera un canal, permitiendo que otras señales lo utilicen.
     */
    void markChannelFree(const std::string& chipId, int channel);
    
    /** * @brief Obtiene una lista de los índices de canales disponibles en un chip.
     */
    std::vector<int> getFreeChannels(const std::string& chipId);

    // ======================================================
    // ACCESO A DATOS (Runtime / Performance)
    // ======================================================

    /** * @brief Lee el valor de la caché de hardware.
     * @param outValue Variable donde se volcará el dato.
     * @return true si el chip existe y el dato es válido (ha sido actualizado).
     */
    bool getRawValue(const std::string& chipId, int channel, float& outValue);
    
    /** * @brief Escribe un valor en la caché para que el bus lo transmita.
     * @note Valida si el modo del chip permite escritura (ChipMode).
     */
    bool setRawValue(const std::string& chipId, int channel, float value);

} // namespace ChipMgr

// ======================================================
// WRAPPERS DE COMPATIBILIDAD (Nombres Antiguos)
// ======================================================
// Estos wrappers aseguran que tu código actual siga funcionando sin cambios.

inline Chip* chipManagerGet(const std::string& id) { 
    return ChipMgr::getById(id); 
}

inline void chipManagerAdd(const Chip& c) { 
    ChipMgr::add(c); 
}

inline bool chipMarkChannelUsed(const std::string& chipId, int channel, const std::string& signalId) { 
    return ChipMgr::markChannelUsed(chipId, channel, signalId); 
}

inline void chipMarkChannelFree(const std::string& chipId, int channel) { 
    ChipMgr::markChannelFree(chipId, channel); 
}

inline std::vector<int> chipGetFreeChannels(const std::string& chipId) { 
    return ChipMgr::getFreeChannels(chipId); 
}