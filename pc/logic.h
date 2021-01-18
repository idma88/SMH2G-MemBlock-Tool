#ifndef __LOGIC_H__
#define __LOGIC_H__

#include <algorithm>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "ext/xserial.hpp"
#include "../arduino/protocol.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#define ARDUINO_BRIDGE 1

namespace MemBlockTool
{
    class CLogic
    {
      public:
        // Тип callback-функции прогресса выполнения
        typedef std::function<void(int)> TCallbackProgress;

        const uint8_t ERR_WRITE        = 0xF0;
        const uint8_t ERR_ANSWER       = 0xF1;
        const uint8_t ERR_DATA_PARTIAL = 0xF2;

      public:
        /**
         * Конструктор
         */
        CLogic();
        /**
         * Деструктор
         */
        ~CLogic();

        /**
         * Устанавливает COM-порт для связи
         * @param portName - [in] Имя порта
         * @return Возвращает true в случае успеха или false при возникновении ошибки
         */
        bool SetPort(const std::string& portName);

        /**
         * Устанавливает callback-функцию контроля прогресса
         * @param callback - [in] Callback-функция
         * @return Возвращает true в случае успеха или false при возникновении ошибки
         */
        bool SetCallback(const TCallbackProgress& callback);

        /**
         * Производит резервное копировании из EEPROM в файл
         * @param filename - [in] Путь к файлу
         * @return Возвращает true в случае успеха или false при возникновении ошибки
         */
        bool Backup(const std::string& filename);

        /**
         * Производит восстановление резервной копии из файла в EEPROM
         * @param filename - [in] Путь к файлу
         * @return Возвращает true в случае успеха или false при возникновении ошибки
         */
        bool Restore(const std::string& filename);

      public:
        /**
         * Получает список всех доступных COM-портов
         * @return Список доступных COM-портов
         */
        static std::vector<std::string> GetPortList();

      private:
        typedef boost::shared_array<uint8_t> TBuffer;

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
            ECommand code /* = ECommand::UNKNOWN */;
            uint16_t param /* = 0 */;
            uint8_t* pPayload /* = nullptr */;
        };

        // Структура ответа
        struct SResponse
        {
            EStatus code /* = EAnswer::UNDEF */;
            uint8_t length /* = 0 */;
        };

      private:
        SResponse ComRPC(SRequest data);

      private:
        // Выбранный COM-порт
        uint8_t m_portNum = 0;
        // std::string m_port = "";

        // Callback функция прогресса выполнения
        TCallbackProgress m_callbackProgress = nullptr;

        // Интерфейс COM-порта
        boost::shared_ptr<xserial::ComPort> m_com;

        std::mutex m_mutex;
    };

} // namespace MemBlockTool
#endif // !__LOGIC_H__
