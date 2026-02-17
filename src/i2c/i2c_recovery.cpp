#include <Wire.h>
#include "system/LogSystem.h"
#include "board/board_config.h"



void i2cRecoverBus()
{
    escribirLog("I2C: Recover bus (SCL pulses)");

    pinMode(I2C_SCL_PIN, OUTPUT_OPEN_DRAIN);
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);

    // 9 pulsos SCL
    for (int i = 0; i < 9; i++)
    {
        digitalWrite(I2C_SCL_PIN, HIGH);
        delayMicroseconds(5);
        digitalWrite(I2C_SDA_PIN, LOW);
        delayMicroseconds(5);
    }

    // STOP
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(5);

    // Reinit Wire
    Wire.end();
    delay(10);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    escribirLog("I2C: Bus recovered");
}
