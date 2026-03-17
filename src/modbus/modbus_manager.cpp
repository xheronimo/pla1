#include "modbus/modbus_manager.h"
#include "signal/signal_manager.h"
#include "system/LogSystem.h"
#include <algorithm>
#include <map>

ModbusRTU ModbusManager::_mb;
std::vector<ModbusGroup> ModbusManager::_groups;
unsigned long ModbusManager::_lastPollTime = 0;

void ModbusManager::init(HardwareSerial& serial, uint32_t baud, int8_t txPin) {
    serial.begin(baud, SERIAL_8N1);
    _mb.begin(&serial, txPin);
    _mb.master();
    escribirLog("MODBUS: Master iniciado a %u baud. Motor de decodificación 32-bit OK.", baud);
}

// --- DECODIFICADOR INDUSTRIAL ---
float ModbusManager::decodeValue(uint16_t* regs, SignalKind kind, bool swapped) {
    uint32_t combined = 0;

    // Si la señal es analógica, tratamos como 32 bits (Float o Int32)
    if (kind == SignalKind::SENSOR_ANALOG || kind == SignalKind::ACTUATOR_ANALOG) {
        if (!swapped) {
            combined = ((uint32_t)regs[0] << 16) | regs[1]; // Big Endian
        } else {
            combined = ((uint32_t)regs[1] << 16) | regs[0]; // Word Swap / Little Endian
        }

        // Si es un sensor analógico, interpretamos como Float IEEE 754
        float f;
        memcpy(&f, &combined, sizeof(float));
        return f;
    }

    // Si es digital, solo devolvemos el primer registro (16 bits)
    return (float)regs[0];
}

void ModbusManager::pollAll() {
    if (millis() - _lastPollTime < 100) return;
    if (_mb.slave()) return;

    for (auto& group : _groups) {
        // Reintento inteligente para esclavos caídos
        if (!group.isAlive && (millis() % 5000 > 200)) continue;

        _mb.readHreg(group.slaveId, group.startReg, group.buffer, group.count, 
            [&group](Modbus::ResultCode event, uint16_t transactionId, void* data) {
                if (event == Modbus::EX_SUCCESS) {
                    ModbusManager::reportSuccess(group.slaveId);
                    
                    for (const String& id : group.signalIds) {
                        Signal* sig = SignalMgr::getById(id.c_str());
                        if (sig) {
                            int offset = sig->channel - group.startReg;
                            if (offset >= 0 && offset < group.count) {
                                // Llamamos al decodificador con los registros necesarios
                                float val = ModbusManager::decodeValue(&group.buffer[offset], sig->kind, sig->invertido);
                                SignalMgr::updateRemote(id.c_str(), val, SignalQuality::GOOD);
                            }
                        }
                    }
                } else {
                    ModbusManager::reportError(group.slaveId);
                }
                return true;
        });
    }
    _lastPollTime = millis();
    _mb.task();
}

void ModbusManager::autoBuildGroups() {
    escribirLog("MODBUS: Generando mapa de optimización...");
    for (auto& g : _groups) { delete[] g.buffer; }
    _groups.clear();

    auto& allSignals = SignalMgr::getAll();
    std::vector<Signal*> mbSignals;
    for (auto& s : allSignals) {
        if (s.bus == BusType::BUS_MODBUS) mbSignals.push_back(&s);
    }

    if (mbSignals.empty()) return;

    std::sort(mbSignals.begin(), mbSignals.end(), [](Signal* a, Signal* b) {
        if (a->chipId != b->chipId) return a->chipId < b->chipId;
        return a->channel < b->channel;
    });

    std::vector<String> currentIds;
    uint8_t currentSlave = mbSignals[0]->chipId;
    uint16_t currentStart = mbSignals[0]->channel;
    uint16_t currentCount = 0;
    const int MAX_GAP = 6; 

    for (size_t i = 0; i < mbSignals.size(); i++) {
        bool sameSlave = (mbSignals[i]->chipId == currentSlave);
        int gap = mbSignals[i]->channel - (currentStart + currentCount);

        if (sameSlave && gap <= MAX_GAP && (mbSignals[i]->channel - currentStart) < 120) {
            currentIds.push_back(String(mbSignals[i]->id));
            // Si es analógica, reservamos 2 registros para el bloque
            int regsNeeded = (mbSignals[i]->kind == SignalKind::SENSOR_ANALOG) ? 2 : 1;
            currentCount = (mbSignals[i]->channel - currentStart) + regsNeeded;
        } else {
            addGroup(currentSlave, currentStart, currentCount, currentIds);
            currentSlave = mbSignals[i]->chipId;
            currentStart = mbSignals[i]->channel;
            currentIds.clear();
            currentIds.push_back(String(mbSignals[i]->id));
            currentCount = (mbSignals[i]->kind == SignalKind::SENSOR_ANALOG) ? 2 : 1;
        }
    }
    addGroup(currentSlave, currentStart, currentCount, currentIds);
}

void ModbusManager::addGroup(uint8_t slave, uint16_t start, uint16_t count, std::vector<String> ids) {
    ModbusGroup newGroup;
    newGroup.slaveId = slave;
    newGroup.startReg = start;
    newGroup.count = count;
    newGroup.buffer = new uint16_t[count];
    newGroup.errorCount = 0;
    newGroup.isAlive = true;
    newGroup.signalIds = ids;
    _groups.push_back(newGroup);
}

void ModbusManager::reportSuccess(uint8_t slaveId) {
    for (auto& g : _groups) {
        if (g.slaveId == slaveId) {
            g.errorCount = 0;
            g.isAlive = true;
        }
    }
}

void ModbusManager::reportError(uint8_t slaveId) {
    for (auto& g : _groups) {
        if (g.slaveId == slaveId) {
            g.errorCount++;
            if (g.errorCount >= MAX_ERRORS) {
                g.isAlive = false;
                escribirLog("MODBUS: Timeout en Esclavo %d", slaveId);
            }
        }
    }
}

void ModbusManager::updateWatchdog() {
    for (auto& g : _groups) {
        if (!g.isAlive) {
            for (const String& id : g.signalIds) {
                SignalMgr::updateRemote(id.c_str(), 0, SignalQuality::ERROR);
            }
        }
    }
}