//hardware_defs.h
#pragma once
#include <Arduino.h>






#define ANALOG_A1  36
#define ANALOG_A2  34
#define ANALOG_A3  35
#define ANALOG_A4  39

#define I2C_SDA 4
#define I2C_SCL 5

#define Relay_IIC_address1 0x24
#define Relay_IIC_address2 0x25

#define Input_IIC_address1 0x21
#define Input_IIC_address2 0x22
// =====================
// Pines OneWire
// =====================
#define ONEWIRE1_PIN 32
#define ONEWIRE2_PIN 33
#define ONEWIRE3_PIN 14

// RF433MHz wireless 
#define receiver 2
#define sender 15

//Ethernet (LAN8720) I/O define:

#define ETH_ADDR        0
#define ETH_POWER_PIN  -1
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN  18
#define ETH_TYPE      ETH_PHY_LAN8720
#ifdef ETH_CLK_MODE
  #undef ETH_CLK_MODE
#endif
#define ETH_CLK_MODE  ETH_CLOCK_GPIO17_OUT

//RS485:
#define RS485_RX 16
#define RS485_TX 13 


// =====================
// Archivos de configuración
// =====================
#define CFG_FILE "/config.json"


// =====================
// Límites globales
// =====================
#define DI_MAX 16
#define NUM_SENSORES 3

extern volatile uint16_t EstadosEntradas;
extern volatile uint16_t EstadosSalidas;

