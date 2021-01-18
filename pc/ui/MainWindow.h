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

namespace MemBlockTool
{
    //! Класс главного окна приложения
    class CMainWindow : public QMainWindow
    {
        Q_OBJECT
      public:
        /**
         * Конструктор
         * @param logic  - [in] объект логики;
         * @param parent - [in] родительское окно;
         * @param flags  - [in] флаги (см. Qt Assistant).
         */
        CMainWindow(CLogic& logic, QWidget* parent = 0, Qt::WFlags flags = 0);

        /**
         * Деструктор
         */
        ~CMainWindow();

      signals:
        void UiState(bool enable = true);
        void Progress(int percent);
        void RescanPorts();

      private slots:
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
        CLogic& m_logic;
    };

} // namespace MemBlockTool

#endif // MAIN_WINDOW_H
