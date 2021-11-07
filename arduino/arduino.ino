#define ARDUINO

#define DEBUG        // TODO
#define BYTE_BY_BYTE 1

#include "EEPROM.h"
#include "Test.h"

using namespace MemBlockTool;

void setup()
{
  Serial.begin(BAUDRATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Инициализируем как ведущее устройство
  Wire.begin();

#ifdef DEBUG
  Serial.println("/// DEBUG");
  // __TestRead();
  // __TestWrite();
  // __Test();
  // __Test2();
  __Test3();
#endif // DEBUG
}

void loop()
{
#ifdef DEBUG
  return;
#endif // DEBUG

  Request  req;
  Response ans;

  if (Serial.available() >= sizeof(req)) {
    Serial.readBytes((char*)&req.code, sizeof(req.code));
    Serial.readBytes((char*)&req.param, sizeof(req.param));

    switch (req.code) {
      case CommandCode::CHIP_SELECT:
        // Serial.print("CHIP_SELECT ");
        // Serial.print(req.param);

        ans = ChipSelect(req);
        break;
      case CommandCode::ADDRESS:
        // Serial.print("ADDRESS ");
        // Serial.print(req.param);

        ans = SetAddress(req);
        break;
      case CommandCode::READ:
        // Serial.print("READ ");
        // Serial.print(req.param);

        ans = ReadEEPROM(req);
        break;
      case CommandCode::WRITE:
        // Serial.print("WRITE ");
        // Serial.print(req.param);

        ans = WriteEEPROM(req);
        break;
      case CommandCode::UNKNOWN:
        // Serial.print("UNKNOWN ");
        // Serial.print(req.param);

      default:
        ans.code  = StatusCode::UNDEF;
        ans.param = req.code;
        break;
    }

    Serial.write((uint8_t*)&ans, sizeof(ans));

    if (req.code == CommandCode::READ) Serial.write(g_buffer.Get(), g_buffer.Size());
  }
}
