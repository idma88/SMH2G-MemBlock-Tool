// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "MainWindow.h"

namespace MemBlockTool {
  MainWindow::MainWindow(Logic& logic, QWidget* parent /* = 0*/, Qt::WindowFlags flags /* = 0*/)
    : QMainWindow(parent, flags)
    , m_logic(logic)
  {
    m_ui.setupUi(this);

    connect(m_ui.btnScanPorts, SIGNAL(clicked()), this, SLOT(OnRescanPorts()));
    connect(m_ui.btnBackup, SIGNAL(clicked()), this, SLOT(OnBackup()));
    connect(m_ui.btnRestore, SIGNAL(clicked()), this, SLOT(OnRestore()));

    connect(this, SIGNAL(UiState(bool)), this, SLOT(OnUiState(bool)));
    connect(this, SIGNAL(RescanPorts()), this, SLOT(OnRescanPorts()));
    connect(this, SIGNAL(Progress(int)), this, SLOT(OnProgress(int)));

    connect(this, SIGNAL(TaskStart()), this, SLOT(OnTaskStart()));
    connect(this, SIGNAL(TaskEnd(bool)), this, SLOT(OnTaskEnd(bool)));

    m_logic.SetCallbackProgress(std::bind(&MainWindow::OnProgress, this, std::placeholders::_1));
    m_logic.SetCallbackTaskEnd(std::bind(&MainWindow::OnTaskEnd, this, std::placeholders::_1));

    emit RescanPorts();
    emit UiState();
  }

  MainWindow::~MainWindow()
  {}

  void MainWindow::OnTaskStart()
  {
    if (QThread::currentThread() != this->thread()) {
      emit TaskStart();
      return;
    }

    emit UiState(false);
  }

  void MainWindow::OnTaskEnd(bool status)
  {
    if (QThread::currentThread() != this->thread()) {
      emit TaskEnd(status);
      return;
    }

    if (!status) {
      QMessageBox msgbox(QMessageBox::Critical,
                         "Ошибка",
                         QString::fromStdString(m_logic.GetErrorMessage()),
                         QMessageBox::StandardButton::Ok);
      msgbox.show();
    }

    emit UiState(true);
  }

  void MainWindow::OnBackup()
  {
    emit TaskStart();

    if (!m_logic.SetPort(m_ui.cbComPort->currentText().toStdString())) {
      QMessageBox msgbox(QMessageBox::Critical,
                         "Ошибка",
                         QString::fromStdString(m_logic.GetErrorMessage()),
                         QMessageBox::StandardButton::Ok);
      msgbox.show();

      return;
    }

    if (!m_logic.Backup("D:/firmware.bin")) {
      QMessageBox msgbox(QMessageBox::Critical,
                         "Ошибка",
                         QString::fromStdString(m_logic.GetErrorMessage()),
                         QMessageBox::StandardButton::Ok);
      msgbox.show();

      return;
    }

    // TODO

    //- emit UiState(true);
  }

  void MainWindow::OnRestore()
  {
    emit TaskStart();

    // if (!m_logic.SetPort(m_ui.cbComPort->currentText().toStdString()))
    if (!m_logic.SetPort("COM6")) {
      QMessageBox msgbox(QMessageBox::Critical,
                         "Ошибка",
                         QString::fromStdString(m_logic.GetErrorMessage()),
                         QMessageBox::StandardButton::Ok);
      msgbox.show();

      return;
    }

    if (!m_logic.Restore("D:/firmware_to_eeprom.bin")) {
      QMessageBox msgbox(QMessageBox::Critical,
                         "Ошибка",
                         QString::fromStdString(m_logic.GetErrorMessage()),
                         QMessageBox::StandardButton::Ok);
      msgbox.show();

      return;
    }

    // TODO

    //- emit UiState(true);
  }

  void MainWindow::OnUiState(bool enable /*= true*/)
  {
    if (QThread::currentThread() != this->thread()) {
      emit UiState(enable);
      return;
    }

    m_ui.cbComPort->setEnabled(enable);
    m_ui.btnScanPorts->setEnabled(enable);
    m_ui.btnBackup->setEnabled(enable);
    m_ui.btnRestore->setEnabled(enable);

    update();
  }

  void MainWindow::OnProgress(int percent)
  {
    if (QThread::currentThread() != this->thread()) {
      emit Progress(percent);
      return;
    }

    m_ui.progress->setValue(percent);
  }

  void MainWindow::OnRescanPorts()
  {
    if (QThread::currentThread() != this->thread()) {
      emit RescanPorts();
      return;
    }

    std::vector<std::string> ports = m_logic.GetPortList();

    m_ui.cbComPort->clear();
    for (std::vector<std::string>::iterator it = ports.begin(); it != ports.end(); ++it) {
      QString str = QString::fromStdString(*it);
      m_ui.cbComPort->addItem(str, str);
    }

    update();
  }

} // namespace MemBlockTool
