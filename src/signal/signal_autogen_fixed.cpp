#include "signal/signal_autogen_fixed.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "board/board_config.h"
#include <stdio.h>

void signalAutogenFixedGenerateAll() {
    char idBuf[32];
    char nameBuf[32];

    // ==================================================
    // 1. LOS 4 PCF8574 (32 Señales Digitales)
    // ==================================================
    ChipMgr::add("BOARD_IN_1", 8);  
    ChipMgr::add("BOARD_IN_2", 8);  
    ChipMgr::add("BOARD_OUT_1", 8); 
    ChipMgr::add("BOARD_OUT_2", 8); 

    for (uint8_t i = 0; i < 8; i++) {
        // ENTRADAS DIGITALES
        Signal si;
        si.isReserved = true;
        si.kind = SignalKind::DIGITAL_IN;
        
        snprintf(idBuf, sizeof(idBuf), "SYS_DI_%u", i + 1);
        si.name = idBuf;
        si.chipName = "BOARD_IN_1";
        si.channelIdx = i;
        SignalMgr::add(si);

        snprintf(idBuf, sizeof(idBuf), "SYS_DI_%u", i + 9);
        si.name = idBuf;
        si.chipName = "BOARD_IN_2";
        si.channelIdx = i;
        SignalMgr::add(si);

        // SALIDAS / RELÉS
        Signal so;
        so.isReserved = true;
        so.kind = SignalKind::DIGITAL_OUT;

        snprintf(idBuf, sizeof(idBuf), "SYS_RELAY_%u", i + 1);
        so.name = idBuf;
        so.chipName = "BOARD_OUT_1";
        so.channelIdx = i;
        SignalMgr::add(so);

        snprintf(idBuf, sizeof(idBuf), "SYS_RELAY_%u", i + 9);
        so.name = idBuf;
        so.chipName = "BOARD_OUT_2";
        so.channelIdx = i;
        SignalMgr::add(so);
    }

    // ==================================================
    // 2. LOS 4 ANALÓGICOS (ADC ESP32)
    // ==================================================
    ChipMgr::add("BOARD_ADC", 4);
    
    for (uint8_t i = 0; i < 4; i++) {
        Signal sa;
        snprintf(idBuf, sizeof(idBuf), "SYS_ADC_CH%u", i + 1);
        snprintf(nameBuf, sizeof(nameBuf), "Analog In %u", i + 1);
        sa.name = idBuf;
        sa.chipName = "BOARD_ADC";
        sa.channelIdx = i;
        sa.kind = SignalKind::ANALOG_IN;
        sa.isReserved = true;
        SignalMgr::add(sa);
    }

    // ==================================================
    // 3. LOS 3 GPIO MULTIPROPÓSITO (DHT11/22, OneWire, etc.)
    // ==================================================
    // Los llamamos CH1, CH2, CH3 porque no sabemos qué habrá conectado
    ChipMgr::add("BOARD_GPIO_BUS", 3);
    
    for (uint8_t i = 0; i < 3; i++) {
        Signal sw;
        snprintf(idBuf, sizeof(idBuf), "SYS_GPIO_CH%u", i + 1);
        snprintf(nameBuf, sizeof(nameBuf), "GPIO Channel %u", i + 1);
        sw.name = idBuf;
        sw.chipName = "BOARD_GPIO_BUS";
        sw.channelIdx = i;
        // Por defecto ANALOG_IN (sirve para Temp/Hum del DHT), 
        // pero el usuario podrá cambiar el 'kind' si conecta un sensor digital.
        sw.kind = SignalKind::ANALOG_IN; 
        sw.isReserved = true; 
        SignalMgr::add(sw);
    }
}