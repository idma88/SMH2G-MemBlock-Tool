/*
 * This is an independent project of an individual developer. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include "MainWindow.h"

namespace MemBlockTool
{
    CMainWindow::CMainWindow(CLogic& logic, QWidget* parent /* = 0*/, Qt::WFlags flags /* = 0*/) :
        QMainWindow(parent, flags), m_logic(logic)
    {
        m_ui.setupUi(this);

        connect(m_ui.btnScanPorts, SIGNAL(clicked()), this, SLOT(OnRescanPorts()));
        connect(m_ui.btnBackup, SIGNAL(clicked()), this, SLOT(OnBackup()));
        connect(m_ui.btnRestore, SIGNAL(clicked()), this, SLOT(OnRestore()));

        connect(this, SIGNAL(UiState(bool)), this, SLOT(OnUiState(bool)));
        connect(this, SIGNAL(RescanPorts()), this, SLOT(OnRescanPorts()));

        m_logic.SetCallback(std::bind(&CMainWindow::OnProgress, this, std::placeholders::_1));

        emit RescanPorts();
        emit UiState();
    }

    CMainWindow::~CMainWindow()
    {
    }

    void CMainWindow::OnBackup()
    {
        emit UiState(false);

        if (!m_logic.SetPort(m_ui.cbComPort->currentText().toStdString()))
        {
            // TODO Error message
            return;
        }

        if (!m_logic.Backup("D:/firmware.bin"))
        {
            // TODO Error message
            return;        
        }

        // TODO
        emit UiState(true);
    }

    void CMainWindow::OnRestore()
    {
        emit UiState(false);
        // TODO
        emit UiState(true);
    }

    void CMainWindow::OnUiState(bool enable /*= true*/)
    {
        if (QThread::currentThread() != this->thread())
        {
            emit UiState(enable);
            return;
        }

        m_ui.btnBackup->setEnabled(enable);
        m_ui.btnRestore->setEnabled(enable);

        update();
    }

    void CMainWindow::OnProgress(int percent)
    {
        if (QThread::currentThread() != this->thread())
        {
            emit Progress(percent);
            return;
        }

        m_ui.progress->setValue(percent);
    }

    void CMainWindow::OnRescanPorts()
    {
        if (QThread::currentThread() != this->thread())
        {
            emit RescanPorts();
            return;
        }

        std::vector<std::string> ports = m_logic.GetPortList();

        m_ui.cbComPort->clear();
        for (std::vector<std::string>::iterator it = ports.begin(); it != ports.end(); ++it)
        {
            QString str = QString::fromStdString(*it);
            m_ui.cbComPort->addItem(str, str);
        }

        update();
    }

} // namespace MemBlockTool
