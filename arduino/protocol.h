#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h> // uint8_t, uint16_t

#define ARDUINO_BRIDGE 1

namespace MemBlockTool
{
    // Коды команд
    enum ECommand : char
    {
        UNKNOWN     = 0,
        CHIP_SELECT = 'C',
        ADDRESS     = 'A',
        READ        = 'R',
        WRITE       = 'W'
    };

    // Коды ответов
    enum EStatus : char
    {
        UNDEF = 0,
        OK    = 'O',
        ERR   = 'E'
    };

    // Структура запроса
    struct SRequest
    {
        ECommand code;
        uint16_t param;
        uint8_t* pPayload;
    };

    // Структура ответа
    struct SResponse
    {
        EStatus code;
        uint8_t length;
    };

} // namespace MemBlockTool
#endif // !__PROTOCOL_H__
