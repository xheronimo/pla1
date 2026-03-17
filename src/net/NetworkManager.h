#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <vector>

/**
 * @brief Enumeración de interfaces de red disponibles para el PLC.
 */
enum class NetInterface : uint8_t {
    NONE = 0,
    ETHERNET,
    WIFI,
    CELLULAR_4G
};

/**
 * @brief Gestor de Red Industrial.
 * Controla el failover entre interfaces, la seguridad del Punto de Acceso (AP),
 * y la comunicación en tiempo real vía WebSockets para el Dashboard.
 */
class NetworkMgr {
public:
    // --- Inicialización y Servicios Web ---
    
    /**
     * @brief Configura WebSockets, rutas del servidor y lanza tareas de monitoreo.
     * @param server Puntero al servidor web asíncrono.
     */
    static void init(AsyncWebServer* server);

    /**
     * @brief Envía el estado de todas las señales del PLC a los clientes WebSockets.
     */
    static void broadcastSignals();

    /**
     * @brief Activa un temporizador para cerrar el AP automáticamente.
     * @param minutes Tiempo en minutos antes del cierre por seguridad.
     */
    static void activateAPTimer(int minutes);

    /**
     * @brief Verifica si el AP debe cerrarse por inactividad o tiempo cumplido.
     */
    static void manageAPSecurity();

    // --- Gestión de Failover e Interfaces ---

    /**
     * @brief Tarea de RTOS: Monitorea la salud de la conexión activa (5s).
     */
    static void monitorTask(void* pv);

    /**
     * @brief Tarea de RTOS: Intenta recuperar interfaces de mayor prioridad (30s).
     */
    static void recoveryTask(void* pv);

    /**
     * @brief Evalúa todas las interfaces y selecciona la mejor disponible.
     */
    static void handleFailover();

    /**
     * @brief Cambia físicamente la ruta de datos a una nueva interfaz.
     */
    static void switchInterface(NetInterface newIface);

    /**
     * @brief Realiza pruebas de conectividad (Ping) hacia servidores externos.
     */
    static bool checkInternetAccess();

    /**
     * @brief Evalúa la salud de una interfaz específica (Local vs Internet).
     */
    static void checkInterfaceHealth(NetInterface iface);

    // --- Getters de Estado ---
    static NetInterface getActiveInterface() { return _currentActiveInterface; }
    static bool isTransitioning() { return _isTransitioning; }

private:
    // Callback para eventos de WebSocket
    static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                          AwsEventType type, void *arg, uint8_t *data, size_t len);

    // Miembros estáticos para servicios web
    static AsyncWebSocket _ws;
    static unsigned long _apStartTime;
    static unsigned long _apDuration;
    static bool _apTimerEnabled;

    // Miembros estáticos para gestión de ruta
    static NetInterface _currentActiveInterface;
    static bool _isTransitioning;
    static unsigned long _lastRouteCheck;
};

#endif