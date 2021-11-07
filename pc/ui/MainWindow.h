#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <fstream>
#include <string>

#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressBar>
#include <QThread>

#include "logic.h"
#include "ui_MainWindow.h"

namespace MemBlockTool {
  //! Класс главного окна приложения
  class MainWindow : public QMainWindow
  {
    Q_OBJECT
  public:
    /**
     * Конструктор
     * @param logic  - [in] объект логики;
     * @param parent - [in] родительское окно;
     * @param flags  - [in] флаги (см. Qt Assistant).
     */
    MainWindow(Logic& logic, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    /**
     * Деструктор
     */
    ~MainWindow();

  signals:
    void UiState(bool enable = true);
    void Progress(int percent);
    void RescanPorts();
    void TaskStart();
    void TaskEnd(bool status);

  private slots:
    /**
     * Обработчик начала выполнения задания
     */
    void OnTaskStart();
    /**
     * Обработчик окончания выполнения задания
     * @param status - [in] Признак успешного выполнения задания
     */
    void OnTaskEnd(bool status);

    /**
     * Обработчик кнопки резервного копирования
     */
    void OnBackup();

    /**
     * Обработчик кнопки восстановления
     */
    void OnRestore();

    /**
     * Обработчик, устанавливающий состояния элементов
     * @param enable - [in] Состояние элементов управления (вкл./выкл.)
     */
    void OnUiState(bool enable = true);

    /**
     * Обработчик прогресса выполнения
     * @param percent - [in] Процент завершения задачи
     */
    void OnProgress(int percent);

    /**
     * Обработчик, заполняющий список доступных COM-портов
     */
    void OnRescanPorts();

  private:
    //! Объект главного окна
    Ui::MainWindow m_ui;
    //! Объект логики приложения.
    Logic& m_logic;
  };

} // namespace MemBlockTool

#endif // MAIN_WINDOW_H
