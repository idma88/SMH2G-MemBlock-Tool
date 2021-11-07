#pragma once

#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "ext/xserial.hpp"

#include "../arduino/Protocol.h"
#include "ErrorMessage.h"

#define ARDUINO_BRIDGE 1

namespace MemBlockTool {
  class Logic : public ErrorMessage
  {
  public:
    /// Тип callback-функции прогресса выполнения
    typedef std::function<void(int)> TCallbackProgress;

    /// Тип callback-функции окончания выполнения задачи
    typedef std::function<void(bool)> TCallbackTaskEnd;

    const uint8_t ERR_WRITE        = 0xF0;
    const uint8_t ERR_ANSWER       = 0xF1;
    const uint8_t ERR_DATA_PARTIAL = 0xF2;

  public:
    /**
     * Конструктор
     */
    Logic();
    /**
     * Деструктор
     */
    ~Logic();

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
    bool SetCallbackProgress(const TCallbackProgress& callback);

    /**
     * Устанавливает callback-функцию окончания выполнения задачи
     * @param callback - [in] Callback-функция
     * @return Возвращает true в случае успеха или false при возникновении ошибки
     */
    bool SetCallbackTaskEnd(const TCallbackTaskEnd& callback);

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
    typedef std::vector<uint8_t> TBuffer;

  private:
    Response ComRPC(Request data);

    void BackupProc(const std::string& filename);

    void RestoreProc(const std::string& filename);

  private:
    /// Выбранный COM-порт
    uint8_t m_portNum;

    /// Callback функция прогресса выполнения
    TCallbackProgress m_callbackProgress;

    /// Callback функция окончания выполнения задачи
    TCallbackTaskEnd m_callbackTaskEnd;

    /// Интерфейс COM-порта
    std::shared_ptr<xserial::ComPort> m_com;

    /// Мьютекс для блокировки одновременного выполнения нескольких задач
    std::mutex m_mutexProc;

    std::thread m_proc;
  };

} // namespace MemBlockTool
