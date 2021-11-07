#pragma once

#include <Wire.h>
#include <stdint.h> // uint8_t, uint16_t

#include "EEPROM.h"

#define SWAP_ADDR 0 // Правильный порядок при SWAP_ADDR == 0
#define CHIP_ID   0
#define ERASE     0
#define ERASE_B2B 0

using namespace MemBlockTool;

#ifdef DEBUG
/***************************************************************************
 * ! DEBUG
 **************************************************************************/

uint8_t  __chip    = 0; // Номер микросхемы
uint16_t __address = 0; // Адрес в EEPROM

void __CS(uint8_t chip)
{
  __chip = CHIP_BASE + chip;
}

bool __ADDR(uint16_t address)
{
  if (address > CHIP_MAX_ADDR) return false;

  __address = address;
  return true;
}

uint8_t AddrH()
{
#if SWAP_ADDR == 0
  return (__address >> 8) & 0xFF;
#else
  return __address & 0xFF;
#endif
}
uint8_t AddrL()
{
#if SWAP_ADDR == 0
  return __address & 0xFF;
#else
  return (__address >> 8) & 0xFF;
#endif
}

uint8_t __READ()
{
  Wire.beginTransmission(__chip);
  Wire.write(AddrH());
  Wire.write(AddrL());
  Wire.endTransmission();

  Wire.requestFrom(__chip, (uint8_t)1);
  return Wire.read();
}

void __WRITE(uint8_t data)
{
  Wire.beginTransmission(__chip);
  Wire.write(AddrH());
  Wire.write(AddrL());
  Wire.write(data);

  delay(WRITE_TIMEOUT);
  Wire.endTransmission();
}

void __WRITE(uint8_t* pData, uint8_t size)
{
  Wire.beginTransmission(__chip);
  Wire.write(AddrH());
  Wire.write(AddrL());
#if (1)
  Wire.write(pData, size);
#else
  // STABLE
  for (size_t i = 0; i < size; ++i) {
    Wire.write(*(pData + i));
  }
#endif

  delay(WRITE_TIMEOUT);
  Wire.endTransmission();
}

/**************************************************************************/

void __TestRead()
{
  Request  req;
  Response ans;

  uint32_t _totalData = 0;

  Serial.println("/// READ");

  for (uint8_t _chip = 0; _chip < CHIP_COUNT; _chip++) {
    Serial.print("Select chip #");
    Serial.println(_chip, DEC);

    req.param = _chip;
    ans       = ChipSelect(req);

    for (uint32_t _addr = 0; _addr <= CHIP_MAX_ADDR; _addr += BUFF_SIZE) {
      req.param = _addr;
      ans       = SetAddress(req);

      req.param = BUFF_SIZE;
      ans       = ReadEEPROM(req);

      _totalData += ans.code == StatusCode::OK ? ans.param : 0;

      for (uint16_t i = 0; i < g_buffer.Size(); i++) {
        Serial.print((uint8_t)g_buffer.Get()[i], HEX);
        if ((i + 1) % 16 == 0)
          Serial.println(" ");
        else
          Serial.print(" ");
      }
    }
  }

  Serial.print("Total: ");
  Serial.print(_totalData, HEX);
}

void __TestWrite()
{
  Request  req;
  Response ans;

  uint32_t _totalData = 0;
  uint8_t* pData      = (uint8_t*)g_buffer.Alloc(BUFF_SIZE);
  uint32_t counter    = 0;

  Serial.println("/// WRITE");

  for (uint8_t _chip = 0; _chip < CHIP_COUNT; _chip++) {
    Serial.print("Select chip #");
    Serial.println(_chip, DEC);
    req.param = _chip;
    ans       = ChipSelect(req);

    for (uint32_t _addr = 0; _addr <= CHIP_MAX_ADDR; _addr += BUFF_SIZE) {
      req.param = _addr;
      ans       = SetAddress(req);

      for (uint16_t i = 0; i < g_buffer.Size() / sizeof(uint32_t); i++) {
        ((uint32_t*)g_buffer.Get())[i] = counter;
        counter += 1;
      }

      for (uint16_t i = 0; i < g_buffer.Size(); i++) {
        // g_buffer.Get()[i] = counter % 256;
        // counter += 1;

        Serial.print((uint8_t)g_buffer.Get()[i], HEX);
        if ((i + 1) % 16 == 0)
          Serial.println(" ");
        else
          Serial.print(" ");
      }

      req.param = BUFF_SIZE;
      ans       = WriteEEPROM(req);

      if (ans.code != StatusCode::OK) break;

      _totalData += ans.code == StatusCode::OK ? ans.param : 0;
    }

    if (ans.code != StatusCode::OK) break;
  }

  Serial.print("Total: ");
  Serial.print(_totalData, HEX);
}

void __EraseEEPROM()
{
  for (uint16_t i = 0; i < 256 /*CHIP_MAX_ADDR*/; ++i) {
    Wire.beginTransmission(g_chipId);
    Wire.write((uint8_t)((i >> 8) & 0xFF));
    Wire.write((uint8_t)(i & 0xFF));
    Wire.write(0);

    delay(WRITE_TIMEOUT);

    Wire.endTransmission();

    if (i % 256 == 0) {
      Serial.print("* ");
      Serial.println(i);
    }
  }
}

void __Test()
{
  Serial.println("/// Test");

  Request  req;
  Response ans;

  uint8_t* pData   = (uint8_t*)g_buffer.Alloc(BUFF_SIZE);
  uint32_t addr    = 0;
  uint8_t  offset  = 0xA0;
  uint8_t  colSize = 32;

  // Serial.print("/// OFFSET: ");
  // Serial.println(offset, HEX);

  // Очищаем чип
  Serial.println("/// ERASE");
  __EraseEEPROM();

#if (0)
  // Генерируем тестовые данные
  Serial.println("/// SAMPLE");
  for (uint16_t row = 0; row <= 0xF; ++row) {
    for (uint16_t col = 0; col <= 0xF; ++col) {
      *pData = ((row << 4) + col + 0x12) % 0x100;

      ++pData;
    }
  }

  // тестовый вывод буфера
  pData = (uint8_t*)g_buffer.Get();
  for (uint16_t i = 0; i <= 0xFF; ++i) {
    uint8_t value = *pData;

    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);

    if ((i + 1) % 16 == 0)
      Serial.println("");
    else
      Serial.print(" ");

    ++pData;
  }
#endif

#if (0)
  // Запись на чип
  Serial.println("/// WRITE");

  req.param = 0;
  ans       = ChipSelect(req);

  req.param = 0;
  ans       = SetAddress(req);

  req.param = 0x100; // g_buffer.Size();
  ans       = WriteEEPROM(req);
  Serial.println(ans.code == StatusCode::OK ? "OK" : "ERROR");
#endif

  // Чтение с чипа
  Serial.println("/// READ");

  memset(g_buffer.Get(), 0x0, g_buffer.Size());

  req.param = 0;
  ans       = ChipSelect(req);

  req.param = 0;
  ans       = SetAddress(req);

  req.param = 0x100;
  ans       = ReadEEPROM(req);

  pData = (uint8_t*)g_buffer.Get();
  for (uint16_t i = 0; i <= 0xFF; ++i) {
    uint8_t value = *pData;

    if (value < 0x10) Serial.print("0");
    Serial.print(value, HEX);

    if ((i + 1) % 16 == 0)
      Serial.println("");
    else
      Serial.print(" ");

    ++pData;
  }

  return;

  for (uint16_t row = 0; row < PAGE_SIZE / colSize; ++row) {
    for (uint16_t col = 0; col < colSize; ++col) {
      g_buffer.Get()[col] = (row * colSize + col + offset) % 256;

      Serial.print((uint8_t)g_buffer.Get()[col], HEX);
      if ((col + 1) % 16 == 0)
        Serial.println(" ");
      else
        Serial.print(" ");
    }

    Serial.print("/// WRITE @ ");

    req.param = 0;
    ans       = ChipSelect(req);

    req.param = row * colSize;
    ans       = SetAddress(req);
    Serial.println(req.param);

    req.param = g_buffer.Size();
    ans       = WriteEEPROM(req);
  }

  // Serial.println("/// WRITE");

  // req.param = 0;
  // ans       = ChipSelect(req);

  // req.param = addr;
  // ans       = SetAddress(req);

  // req.param = g_buffer.Size();
  // ans       = WriteEEPROM(req);
  // Serial.print("Answer: ");
  // Serial.println(ans.code != StatusCode::OK);

  /******************************************/

  Serial.println("/// READ");

  memset(g_buffer.Get(), 0x0, g_buffer.Size());

  req.param = 0;
  ans       = ChipSelect(req);

  req.param = 0x00;
  ans       = SetAddress(req);

  req.param = g_buffer.Size();
  ans       = ReadEEPROM(req);

  for (uint16_t i = 0; i < PAGE_SIZE; i++) {
    Serial.print((uint8_t)g_buffer.Get()[i], HEX);
    if ((i + 1) % 16 == 0)
      Serial.println(" ");
    else
      Serial.print(" ");
  }
}

void __Test2()
{
  Serial.println("/// Test2");

  uint8_t               data;
  static const uint16_t BUFF_SIZE = PAGE_SIZE; // min(PAGE_SIZE, TWI_BUFFER_LENGTH - 3);
  uint8_t               buff[BUFF_SIZE];

  // Serial.print("/// TWI_BUFFER_LENGTH = ");
  // Serial.println(TWI_BUFFER_LENGTH, DEC);

  // Serial.print("/// BUFFER_LENGTH = ");
  // Serial.println(BUFFER_LENGTH, DEC);

  Serial.print("/// SWAP_ADDR = ");
  Serial.println(SWAP_ADDR, DEC);

  Serial.print("/// CHIP_ID = ");
  Serial.println(CHIP_ID, DEC);

  Serial.print("/// ERASE = ");
  Serial.println(ERASE, DEC);

  Serial.print("/// ERASE_B2B = ");
  Serial.println(ERASE_B2B, DEC);

  Serial.print("/// BUFF_SIZE = ");
  Serial.println(BUFF_SIZE, DEC);

  __CS(CHIP_ID);

  for (uint8_t i = 0; i < BUFF_SIZE; i++) {
    buff[i] = 0xFF; // i % 256;

    if (buff[i] < 0x10) Serial.print("0");
    Serial.print(buff[i], HEX);

    if ((i + 1) % 16 == 0)
      Serial.println("");
    else
      Serial.print(" ");
  }
  Serial.println("");

#if (ERASE)
  Serial.println("/// ERASE");
#if (ERASE_B2B)
  for (uint16_t addr = 0; addr < CHIP_MAX_ADDR; addr++) {
    __ADDR(addr);
    //__WRITE(0);
    __WRITE(buff[addr % BUFF_SIZE]);

    if ((addr + 1) % 256 == 0) {
      if (addr < 0x10) Serial.print("0");
      if (addr < 0x100) Serial.print("0");
      if (addr < 0x1000) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
#else
  for (uint32_t addr = 0; addr < CHIP_MAX_ADDR; addr += BUFF_SIZE) {
    __ADDR(addr);
    __WRITE(&buff[0], BUFF_SIZE);

    // if ((addr + 1) % 256 == 0)
    {
      if (addr < 0x10) Serial.print("0");
      if (addr < 0x100) Serial.print("0");
      if (addr < 0x1000) Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
#endif
#endif

  Serial.println("/// READ");
  for (uint16_t addr = 0; addr < CHIP_MAX_ADDR; addr++) {
    __ADDR(addr);
    data = __READ();

#if (0)
    if (data < 0x10) Serial.print("0");
    Serial.print(data, HEX);

    // if ((addr + 1) % 16 == 0)
    //     Serial.println("");
    // else
    //     Serial.print(" ");

    if ((addr + 1) % 16 == 0) Serial.print("   ");
    if ((addr + 1) % 32 == 0) Serial.println("");
    Serial.print(" ");
#else
    if ((addr) % 16 == 0) {
      if (addr < 0x10) Serial.print("0");
      if (addr < 0x100) Serial.print("0");
      if (addr < 0x1000) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print(": ");
    }

    if (data < 0x10) Serial.print("0");
    Serial.print(data, HEX);
    Serial.print(" ");

    if ((addr + 1) % 16 == 0) Serial.println("");
#endif
  }
}

void __Test3()
{
  Serial.println("/// Test3");

  uint8_t               data;
  static const uint16_t BUFF_SIZE = PAGE_SIZE; // min(PAGE_SIZE, TWI_BUFFER_LENGTH - 3);
  uint8_t               buff[BUFF_SIZE];

  Serial.print("/// SWAP_ADDR = ");
  Serial.println(SWAP_ADDR, DEC);

  Serial.print("/// CHIP_ID = ");
  Serial.println(CHIP_ID, DEC);

  Serial.print("/// BUFF_SIZE = ");
  Serial.println(BUFF_SIZE, DEC);

  __CS(CHIP_ID);

  buff[0] = 1 ? 0x0A : 0x01;
  buff[1] = 1 ? 0x0B : 0x02;
  buff[2] = 1 ? 0x0C : 0x03;
  buff[3] = 1 ? 0x0D : 0x04;

  __ADDR(0x0102); // 258 or 513
  __WRITE(&buff[0], 4);
  /*
      __WRITE(0x0A);
      __WRITE(0x0B);
      __WRITE(0x0C);
      __WRITE(0x0D);
  */

  Serial.println("/// READ");
  for (uint16_t addr = 0; addr < 0x0300; addr++) {
    __ADDR(addr);
    data = __READ();

#if (0)
    if (data < 0x10) Serial.print("0");
    Serial.print(data, HEX);

    // if ((addr + 1) % 16 == 0)
    //     Serial.println("");
    // else
    //     Serial.print(" ");

    if ((addr + 1) % 16 == 0) Serial.print("   ");
    if ((addr + 1) % 32 == 0) Serial.println("");
    Serial.print(" ");
#else
    if ((addr) % 16 == 0) {
      if (addr < 0x10) Serial.print("0");
      if (addr < 0x100) Serial.print("0");
      if (addr < 0x1000) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print(": ");
    }

    if (data < 0x10) Serial.print("0");
    Serial.print(data, HEX);
    Serial.print(" ");

    if ((addr + 1) % 16 == 0) Serial.println("");
#endif
  }
}

/***************************************************************************
 * ! DEBUG
 **************************************************************************/
#endif // DEBUG
