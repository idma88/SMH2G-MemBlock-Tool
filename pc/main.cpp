// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <QApplication>

#include "Logic.h"
#include "ui/MainWindow.h"

// Строковая метка, используемая для предотвращения повторного запуска
const std::string DUPLICATE_PREVENTION_TAG = "prevent_duplicates.mbt";

/**
 * Проверяет не запущена ли в данный момент другая копия данного приложения
 * @param tag         - [in] строковая метка
 * @param isDuplicate - [out] true - копия приложения запущена, false - копия приложения не запущена
 * @return true - функция выполнена успешно, false - в случае ошибки
 * @note функция определяет не запущено ли в данный момент приложение с заданной строковой меткой.
 * Если передать метку другого приложения (также использующего данную функцию),
 * то функция определит запущено ли то приложение в данный момент.
 */
inline bool CheckIfDuplicateApp(const std::string& tag, bool& isDuplicate)
{
  void*         hPreventDuplicates = ::CreateMutexA(NULL, 0, tag.c_str());
  unsigned long err                = ::GetLastError();

  if (hPreventDuplicates == NULL) {
    if (err != ERROR_ACCESS_DENIED) return false;
    isDuplicate = true;
  } else {
    isDuplicate = (err == ERROR_ALREADY_EXISTS);
    if (isDuplicate) ::CloseHandle(hPreventDuplicates);
  }

  return true;
}

// Точка входа в приложение
int main(int argc, char* argv[])
{
  bool isDuplicate;
  if (!CheckIfDuplicateApp(DUPLICATE_PREVENTION_TAG, isDuplicate)) return -1;
  if (isDuplicate) return -1;

  QApplication::setStyle("cleanlooks");
  QApplication app(argc, argv);

  MemBlockTool::Logic logic;

  MemBlockTool::MainWindow wnd(logic,
                               NULL,
                               Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint |
                                 Qt::WindowCloseButtonHint);

  wnd.show();

  return app.exec();
}
