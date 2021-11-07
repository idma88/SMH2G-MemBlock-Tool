// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "ErrorMessage.h"

ErrorMessage::ErrorMessage()
  : m_message("")
  , m_isMessageSet(false)
{}

std::string ErrorMessage::GetErrorMessage() const
{
  std::unique_lock<std::mutex> lock(m_mutex);

  return m_message;
}

void ErrorMessage::SetErrorMessage(const std::string& message) const
{
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_isMessageSet) return;

  m_message = message;
}

void ErrorMessage::SetErrorMessage(const char* format, ...) const
{
  std::unique_lock<std::mutex> lock(m_mutex);
  va_list                      args;

  if (m_isMessageSet) return;

  va_start(args, format);

#ifdef _MSC_VER
  int len = _vscprintf(format, args);
#else
  int len = vsprintf(NULL, format, args);
#endif
  len++; // Для \0 в конце строки
  m_message.reserve(len);

#ifdef _MSC_VER
  vsprintf_s(const_cast<char*>(m_message.c_str()), len, format, args);
#else
  vsprintf(const_cast<char*>(m_message.c_str()), format, args);
#endif

  va_end(args);
}