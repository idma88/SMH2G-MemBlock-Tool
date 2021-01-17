#include "Wire.h"
#include "protocol.h"

using namespace MemBlockTool;

/*
    #define EEPROM_ADDRESS 0x50

    byte val = 16;
    byte data;
*/

#define CHIP_BASE 0x50 // b:1010aaa

byte  chipId  = 0;       // Номер микросхемы
int   address = 0;       // Адрес в EEPROM
char* pBuffer = nullptr; // Указатель на буфер для данных

char* NewBuffer(int16_t length)
{
    DeleteBuffer();

    if (length) pBuffer = new char[length];
    return pBuffer;
}

void DeleteBuffer()
{
    if (pBuffer != nullptr) delete[] pBuffer;
}

SResponse ChipSelect(SRequest req)
{
    SResponse ans = { EStatus::ERR, 0 };

    if (req.param < CHIP_COUNT)
    {
        chipId   = CHIP_BASE + req.param;
        ans.code = EStatus::OK;
    }

    return ans;
}

SResponse SetAddress(SRequest req)
{
    SResponse ans = { EStatus::ERR, 0 };

    if (req.param < CHIP_MAX_ADDR)
    {
        address  = req.param;
        ans.code = EStatus::OK;
    }

    return ans;
}

SResponse ReadEEPROM(SRequest req)
{
    SResponse ans      = { EStatus::ERR, 0 };
    int       dataSize = req.param;
    char*     pData;

    Wire.beginTransmission(chipId);
    Wire.write((int)(address >> 8));
    Wire.write((int)(address & 0xFF));
    if (Wire.endTransmission() != 0) return ans; // Error

    if (dataSize + address > CHIP_MAX_ADDR) dataSize = CHIP_MAX_ADDR - address;

    dataSize = Wire.requestFrom(address, dataSize);
    address += dataSize;

    if (dataSize == 0) return ans; // Error

    pBuffer = NewBuffer(dataSize);
    if (pBuffer == nullptr) return ans; // Error

    pData = pBuffer;
    while (Wire.available()) // slave may send less than requested
    {
        *pData = Wire.read(); // receive a byte as character
        ++pData;
    }

    ans.code   = EStatus::OK;
    ans.length = dataSize;

    return ans;
}

SResponse WriteEEPROM(SRequest req)
{
    /*
        Wire.beginTransmission(EEPROM_ADDRESS);
        Wire.write((int)(address >> 8));
        Wire.write((int)(address & 0xFF));
        Wire.write(val);
        Wire.endTransmission();
        delay(5);
    */

    SResponse ans = { EStatus::ERR, 0 };

    // TODO

    return ans;
}

void setup()
{
    Serial.begin(BAUDRATE);
    // while (!Serial) // Ожидаем подключения к порту
    // {
    // }

    Serial.write("Ready!");

    // Инициализируем как ведущее устройство
    Wire.begin();
}

void loop()
{
    SRequest  req;
    SResponse ans;

    if (Serial.available() >= sizeof(req.code) + sizeof(req.param))
    {
        Serial.readBytes((uint8_t*)&req.code, sizeof(req.code));
        Serial.readBytes((uint8_t*)&req.param, sizeof(req.param));

        switch (req.code)
        {
            case ECommand::CHIP_SELECT:
                ans = ChipSelect(req);
                break;
            case ECommand::ADDRESS:
                ans = SetAddress(req);
                break;
            case ECommand::READ:
                ans = ReadEEPROM(req);
                break;
            case ECommand::WRITE:
                ans = WriteEEPROM(req);
                break;
            case ECommand::UNKNOWN:
            default:
                ans.code   = EStatus::UNDEF;
                ans.length =  0;
                break;
        }

        Serial.write((uint8_t*)&ans, sizeof(ans));


        // TODO Send answer
    }
}
