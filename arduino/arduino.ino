#include "Wire.h"

#define EEPROM_ADDRESS 0x50

int  address = 0;
byte val     = 16;
byte data;

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    //запись
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    Wire.write(val);
    Wire.endTransmission();
    delay(5);

    //чтение
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDRESS, 1);
    data = Wire.read();

    Serial.print("Address : Data - ");
    Serial.print(address);
    Serial.print(" : ");
    Serial.println(data);
}

void loop()
{
}
