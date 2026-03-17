#include <Wire.h>
#include "system/LogSystem.h"
#include "board/board_config.h"



void i2cRecoverBus()
{
    escribirLog("I2C: Instando recuperación física (9 pulsos SCL)");

    // 1. Apagar el periférico hardware para tomar control manual
    Wire.end(); 
    
    // 2. Configurar pines. SCL como salida, SDA como entrada para "escuchar"
    pinMode(I2C_SCL_PIN, OUTPUT);
    pinMode(I2C_SDA_PIN, INPUT_PULLUP); 

    // 3. 9 pulsos de reloj. Esto hace que el esclavo avance en su máquina de estados
    // interna hasta que suelte la línea SDA.
    for (int i = 0; i < 9; i++)
    {
        digitalWrite(I2C_SCL_PIN, LOW);
        delayMicroseconds(5);
        digitalWrite(I2C_SCL_PIN, HIGH);
        delayMicroseconds(5);

        // Si SDA ya está en HIGH, el esclavo ya soltó el bus
        if (digitalRead(I2C_SDA_PIN) == HIGH) break;
    }

    // 4. Generar condición de STOP manual
    pinMode(I2C_SDA_PIN, OUTPUT);
    digitalWrite(I2C_SDA_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(I2C_SDA_PIN, HIGH); // SDA sube mientras SCL está alto = STOP

    // 5. Reiniciar el bus hardware
    delay(10);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    escribirLog("I2C: Bus re-inicializado");
}