#include "signal/signal_autogen.h"
#include "signal/signal_manager.h"
#include "chip/chip_manager.h"
#include "modbus/modbus_profiles.h"
#include <Arduino.h>

namespace SignalAutogen {

// Utilidad para crear nombres de señales consistentes
String buildTag(uint8_t id, const char* label) {
    char buf[32];
    snprintf(buf, sizeof(buf), "MB%u_%s", id, label);
    return String(buf);
}

void generateForDevice(const ModbusDevice& dev) {
    // 1. Aseguramos que el Chip exista en el ChipManager
    // Buscamos cuántos canales necesita según el fingerprint
    uint8_t channelsNeeded = 0;
    if (dev.fingerprint == 0x0630) channelsNeeded = 15; // SDM630
    else if (dev.fingerprint == 0x5172) channelsNeeded = 2;  // SHT20
    else if (dev.fingerprint == 0x0420) channelsNeeded = 4;  // ADC 4-20mA
    else {
        // Si es un perfil dinámico, sumamos las señales de sus bloques
        for (auto& prof : ModbusProfiles::g_dynamicProfiles) {
            if (prof.fp == dev.fingerprint) {
                for (auto& block : prof.blocks) channelsNeeded += block.signals.size();
            }
        }
    }

    if (channelsNeeded == 0) return;

    // Registramos el hardware en el ChipManager (si no existe)
    ChipMgr::add(dev.name, channelsNeeded);

    // 2. Generación de Señales Lógicas
    
    // --- CASO SDM630 ---
    if (dev.fingerprint == 0x0630) {
        const char* labels[] = {"V1", "V2", "V3", "I1", "I2", "I3", "P1", "P2", "P3", "Q1", "Q2", "Q3", "PTot", "QTot", "Freq"};
        for (int i = 0; i < 15; i++) {
            Signal s;
            s.name = buildTag(dev.id, labels[i]).c_str();
            s.chipName = dev.name;
            s.channelIdx = i;
            s.kind = SignalKind::ANALOG_IN;
            s.isReserved = true;
            SignalMgr::add(s);
        }
    }

    // --- CASO SHT20 ---
    else if (dev.fingerprint == 0x5172) {
        const char* labels[] = {"Temp", "Hum"};
        for (int i = 0; i < 2; i++) {
            Signal s;
            s.name = buildTag(dev.id, labels[i]).c_str();
            s.chipName = dev.name;
            s.channelIdx = i;
            s.kind = SignalKind::ANALOG_IN;
            s.isReserved = true;
            SignalMgr::add(s);
        }
    }

    // --- CASO PERFILES DINÁMICOS (JSON) ---
    for (auto& prof : ModbusProfiles::g_dynamicProfiles) {
        if (prof.fp == dev.fingerprint) {
            uint8_t currentIdx = 0;
            for (auto& block : prof.blocks) {
                for (auto& profSig : block.signals) {
                    Signal s;
                    // Usamos el nombre que viene definido en el bloque del JSON
                    s.name = buildTag(dev.id, profSig.name.c_str()).c_str(); 
                    s.chipName = dev.name;
                    s.channelIdx = currentIdx++;
                    s.kind = SignalKind::ANALOG_IN;
                    s.isReserved = true;
                    SignalMgr::add(s);
                }
            }
        }
    }
}

void clearForDevice(const char* deviceName) {
    // Implementar si se desea eliminar señales al borrar un dispositivo
}

} // namespace SignalAutogen