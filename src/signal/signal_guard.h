#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>

/**
 * @brief Tipos de señales soportadas por el PLC
 */
enum class SignalKind {
    ANALOG_IN,   // Solo lectura (Sensores)
    ANALOG_OUT,  // Lectura/Escritura (Dimmers, VFDs)
    DIGITAL_IN,  // Binario lectura
    DIGITAL_OUT, // Binario escritura (Relés)
    SYSTEM       // Internas (Uptime, Estado Bus)
};

/**
 * @brief Resultado de las operaciones de validación (Guard)
 */
enum class SignalUpdateResult {
    OK,
    ERR_RESERVED,      // No se puede modificar una señal del sistema
    ERR_INVALID_KIND,  // El tipo de señal no coincide
    ERR_CHIP_NOT_FOUND,// El chip de hardware no existe
    ERR_INVALID_CHANNEL,// El canal solicitado no existe en el chip
    ERR_DUPLICATE      // Ya existe una señal con ese nombre
};

/**
 * @brief Estructura de una Señal Lógica.
 * Representa un dato procesado listo para el usuario/interfaz.
 */
struct Signal {
    std::string name;        // Nombre único (ID lógico)
    std::string chipName;    // Vínculo al Hardware (Nombre del Chip)
    uint8_t     channelIdx;  // Canal dentro de ese chip
    SignalKind  kind;
    
    float    value;          // Valor procesado (escalado y filtrado)
    bool     valid;          // ¿Es el dato fiable?
    bool     isReserved;     // Si es true, la estructura está blindada
    
    // Procesamiento
    float    multiplier = 1.0f; // Escala (y = mx + b)
    float    offset     = 0.0f;
    float    emaAlpha   = 1.0f; // Filtro (1.0 = sin filtro, 0.1 = muy filtrado)
    
    Signal() : name(""), chipName(""), channelIdx(0), kind(SignalKind::ANALOG_IN), 
               value(0.0f), valid(false), isReserved(false) {}
};

/**
 * @brief Gestor Central de Señales
 */
namespace SignalMgr {

    // --- Ciclo de Vida ---
    void init();
    bool add(const Signal& sig);
    void clear();

    // --- Procesamiento ---
    /**
     * @brief Sincroniza las señales con los datos de los Chips.
     * Aplica filtros EMA y escalado lineal.
     */
    void updateAll();

    // --- Acceso ---
    Signal* getByName(const std::string& name);
    const std::vector<Signal>& getSignals();
    
    // --- API / JSON ---
    SignalUpdateResult updateFromJson(const std::string& name, JsonObjectConst root);
    void serializeSignal(const std::string& name, JsonDocument& doc);

} // namespace SignalMgr