#pragma once

#include <Wire.h>

#include "Protocol.h"
#include "SimpleBuffer.h"

#define CHIP_BASE            0x50 ///< Маска адреса EEPROM (b:1010aaa)
#define WIRE_TRANSMISSION_OK 0    ///< Статус OK для библиотеки Wire

namespace MemBlockTool {
  uint8_t      g_chipId  = 0; ///< Номер микросхемы
  uint16_t     g_address = 0; ///< Адрес в EEPROM
  SimpleBuffer g_buffer;      ///< Буфер для данных

  uint8_t AddrHigh(const uint16_t address)
  {
    return (address >> 8) & 0xFF;
  }

  uint8_t AddrLow(const uint16_t address)
  {
    return address & 0xFF;
  }

  Response ChipSelect(const Request& req)
  {
    if (req.param >= CHIP_COUNT) return { StatusCode::ERR, ErrorCode::OUT_OF_BOUND };

    g_chipId = CHIP_BASE + req.param;
    return { StatusCode::OK, ErrorCode::NO_ERROR };
  }

  Response SetAddress(const Request& req)
  {
    if (req.param > CHIP_MAX_ADDR) return { StatusCode::ERR, ErrorCode::OUT_OF_BOUND };

    g_address = req.param;
    return { StatusCode::OK, ErrorCode::NO_ERROR };
  }

  Response ReadEEPROM(const Request& req)
  {
    uint16_t dataSize = req.param;
    // uint16_t dataToRead;
    uint16_t readed = 0;
    uint8_t* pData;

    Wire.beginTransmission(g_chipId);
    Wire.write(AddrHigh(g_address));
    Wire.write(AddrLow(g_address));
    if (Wire.endTransmission() != WIRE_TRANSMISSION_OK)
      return { StatusCode::ERR, ErrorCode::TRANSMISSION_FAILED };

    if (dataSize + g_address > CHIP_MAX_ADDR) dataSize = CHIP_MAX_ADDR - g_address;

    if (dataSize == 0) return { StatusCode::ERR, ErrorCode::NOTHING_TO_PROCESS };

    pData = g_buffer.Alloc(dataSize);
    if (pData == nullptr) return { StatusCode::ERR, ErrorCode::ALLOC_FAILED };

    /*
        dataToRead = dataSize;
        while (dataToRead) {
          Wire.requestFrom(g_chipId, (uint8_t)min(dataToRead, 255));

          while (Wire.available()) {
            *pData = (uint8_t)Wire.read();
            ++pData;
            --dataToRead;
          }
        }

        return { StatusCode::OK, dataSize - dataToRead };
    */
    while (readed < dataSize) {
      if (Wire.requestFrom(g_chipId, (uint8_t)min(dataSize - readed, 255)) == 0) break;

      while (Wire.available()) {
        *(pData + readed) = (uint8_t)Wire.read();
        ++readed;
      }
    }

    return { StatusCode::OK, readed };
  }

  Response WriteEEPROM(const Request& req)
  {
    Response ans      = { StatusCode::OK, ErrorCode::NO_ERROR };
    uint16_t dataSize = req.param;
    uint16_t rest     = 0;
    uint16_t offset   = 0;
    uint8_t* pData    = g_buffer.Alloc(min(BUFF_SIZE, dataSize));

    if (pData == nullptr) return { StatusCode::ERR, ErrorCode::ALLOC_FAILED };

    while (rest) {
      uint16_t payloadReaded = Serial.readBytes(g_buffer.Get(), min(dataSize, g_buffer.Size()));

      for (uint16_t i = 0; i < payloadReaded; ++i, ++offset, --rest) {
        if (g_address + offset > CHIP_MAX_ADDR) {
          ans.param = ErrorCode::PARTIALLY_PROCESSED;
          break;
        }

        Wire.beginTransmission(g_chipId);
        Wire.write(AddrHigh(g_address + offset));
        Wire.write(AddrLow(g_address + offset));
        Wire.write(pData[i]);

        delay(WRITE_TIMEOUT);

        if (Wire.endTransmission() != WIRE_TRANSMISSION_OK)
          return { StatusCode::ERR, ErrorCode::TRANSMISSION_FAILED };
      }
    }

    // Вычитываем остаток полезной нагрузки, если он есть
    while (rest) {
      rest -= Serial.readBytes(g_buffer.Get(), min(rest, g_buffer.Size()));
    }

    return ans;
  }

} // namespace MemBlockTool