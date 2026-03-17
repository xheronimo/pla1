#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ESPAsyncWebServer.h>

// 1. Infraestructura y Configuración de Pines
#include "config/pines.h" // Asegúrate de que este archivo tenga los defines que pasaste
#include "storage/SDManager.h"
#include "system/LogSystem.h"
#include "system/WatchdogManager.h"

// 2. Hardware y Red
#include "chip/chip_manager.h"
#include "net/NetworkLoader.h" // La clase que refactorizamos
#include "net/NetworkManager.h"
#include "modbus/modbus_manager.h"

// 3. Orquestación
#include "task/task_boot.h"

// Instancias Globales
AsyncWebServer server(80);
// Definimos buses SPI explícitos para el ESP32-S3
SPIClass spiSD(FSPI);
SPIClass spiEth(HSPI);

void setup() {
    // --- PASO 1: Diagnóstico temprano ---
    Serial.begin(115200);
    delay(500); 

    // --- PASO 2: Inicialización de Buses (Crítico para V3) ---
    // I2C para RTC, Pantalla y Expansores PCF
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    // SPI para SD (Pines 11, 12, 13, 14)
    spiSD.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    
    // SPI para Ethernet W5500 (Pines 15, 42, 43, 44) + Reset físico
    pinMode(W5500_RST, OUTPUT);
    digitalWrite(W5500_RST, LOW);
    delay(100);
    digitalWrite(W5500_RST, HIGH);
    delay(200);
    spiEth.begin(W5500_SCK, W5500_MISO, W5500_MOSI, W5500_CS);

    // --- PASO 3: Almacenamiento y Seguridad ---
    // Pasamos el pin CS y nuestro objeto SPI específico
    if (SDMgr::begin(SD_CS)) { 
        SDMgr::logEvent("SYS", "SD Montada. Bus SPI_SD activo.");
    }

    // Inicializar I2C/Relés y ver por qué reiniciamos
    ChipMgr::init(); 
    // Aquí podrías leer el RTC_I2C_ADDR para sincronizar la hora antes del log
    
    // --- PASO 4: Modo Seguro y Configuración ---
    // Verificamos si el usuario fuerza modo rescate (ej: jumper en FREE_GPIO_1)
    bool safeMode = (digitalRead(FREE_GPIO_1) == LOW); 
    
    // Carga de red usando nuestra clase optimizada
    // Importante: NetworkLoader debe usar 'spiEth' internamente
   if (!NetworkLoader::loadAndSetup(spiEth)) { 
    // Si falla, ya se encarga loadAndSetup de lanzar el modo rescate
    Serial.println("NetworkLoader finalizado con avisos.");
}

    // --- PASO 5: Watchdog y Servicios ---
    // 30 segundos de gracia para que el W5500 negocie el link
    watchdogInit(30000);

    // Servidor Web y API
    NetworkMgr::init(&server);
    server.begin();
    SDMgr::logEvent("SYS", "Servidor HTTP puerto 80 activo.");

    // --- PASO 6: Bus de Campo (RS485) ---
    if (!safeMode) {
        // Usamos los pines de tu tabla: RX 17, TX 16 (V3)
        ModbusManager::init(Serial1, 9600, RS485_RX_PIN, RS485_TX_PIN); 
        SDMgr::logEvent("SYS", "Modbus RTU iniciado en UART1.");
    }

    // --- PASO 7: Lanzamiento de Tareas FreeRTOS ---
    arrancarTareasMinimas(); 
    
    if (!safeMode) {
        arrancarTareasNormales(); 
    }

    SDMgr::logEvent("SYS", "SETUP COMPLETADO. PLC EN LINEA.");
}

void loop() {
    // El loop no hace nada, las tareas de FreeRTOS tienen el control.
    // Matamos la tarea de setup para liberar memoria.
    vTaskDelete(NULL); 
}