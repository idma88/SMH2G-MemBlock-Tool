#pragma once

#include <stdint.h> // uint8_t, uint16_t

#define ARDUINO_BRIDGE 1

namespace MemBlockTool {
  // Коды команд
  enum CommandCode : char
  {
    UNKNOWN     = 0,
    CHIP_SELECT = 'C',
    ADDRESS     = 'A',
    READ        = 'R',
    WRITE       = 'W'
  };

  // Коды ответов
  enum StatusCode : char
  {
    UNDEF = 0,
    OK    = 'O',
    ERR   = 'E'
  };

  enum ErrorCode : uint16_t
  {
    NO_ERROR            = 0x00,
    TRANSMISSION_FAILED = 0x01, // Ошибка при передачи данных с/на EEPROM
    NOTHING_TO_PROCESS  = 0x02, // Размер данных равен нулю
    PARTIALLY_PROCESSED = 0x03, // Не удалось обработать весь запрос
    ALLOC_FAILED  = 0x04, // Не удалось выделить память под буфер
    OUT_OF_BOUND  = 0x05, // Выход за границу
    UNKNOWN_ERROR = 0xFF
  };

#pragma pack(push, 1)
  // Структура запроса
  struct Request
  {
    CommandCode code;
    uint16_t    param;
#ifndef ARDUINO
    uint8_t* pPayload;
#endif
  };

  // Структура ответа
  struct Response
  {
    StatusCode code;
    uint16_t   param;
  };
#pragma pack(pop)

  // Скорость COM порта
  const unsigned long BAUDRATE = 57600;
  // const unsigned long BAUDRATE = 115200;

  // Количество EEPROM-микросхем
  const uint8_t CHIP_COUNT = 4;

  // Максимальный адрес EEPROM
  const uint16_t CHIP_MAX_ADDR = 65535;

  // Размер буфера
  const uint16_t BUFF_SIZE = 256;

  // Размер страницы для записи
  const uint16_t PAGE_SIZE = 128;

  // Время выполнения записи (мс.)
  const uint16_t WRITE_TIMEOUT = 5;
} // namespace MemBlockTool
