#pragma once

#include <mutex>
#include <stdarg.h> // va_list, va_start, va_end
#include <stdio.h>  // vsprintf; MSVC: _vscprintf, vsprintf_s
#include <string>

class ErrorMessage
{
public:
  /**
   * Возвращает сообщение об ошибке
   * @return Сообщение об ошибке
   */
  std::string GetErrorMessage() const;

protected:
  /**
   * Конструктор
   */
  ErrorMessage();
  /**
   * Деструктор
   */
  virtual ~ErrorMessage() = default;

  /**
   * Устанавливает сообщение об ошибке
   * @param message - [in] Сообщение об ошибке
   */
  void SetErrorMessage(const std::string& message) const;

  /**
   * Устанавливает сообщение об ошибке с форматированием и аргументами
   * @param message - [in] Строка форматирования сообщения об ошибке
   */
  void SetErrorMessage(const char* format, ...) const;

private:
  // Сообщение об ошибке
  mutable std::string m_message;
  // Признак наличия сообщения
  mutable bool m_isMessageSet;
  // Мютекс для доступа к данным
  mutable std::mutex m_mutex;
};