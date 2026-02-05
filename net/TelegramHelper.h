#pragma once
#include <Arduino.h>

// ===============================
// TELEGRAM HELPER (STUB)
// ===============================

void configurarTelegram(const char* token);

// Envía mensaje de alerta
void enviarAlertaTelegram(const char* msg);
