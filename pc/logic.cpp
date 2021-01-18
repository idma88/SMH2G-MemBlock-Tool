/*
 * This is an independent project of an individual developer. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include "logic.h"

namespace MemBlockTool
{
    CLogic::CLogic()
    {
    }

    CLogic::~CLogic()
    {
    }

    bool CLogic::SetPort(const std::string& port)
    {
        if (port.length() == 0) return false;

        std::vector<std::string> portList = GetPortList();

        std::vector<std::string>::iterator it =
            std::find_if(portList.begin(), portList.end(), [&port](const std::string& item) {
                return boost::iequals(port, item);
            });

        if (it == portList.end()) return false;

        m_portNum = atoi(port.c_str() + strlen("COM"));

        return true;
    }

    bool CLogic::SetCallback(const TCallbackProgress& callback)
    {
        if (callback == nullptr) return false;

        m_callbackProgress = callback;
        return true;
    }

    bool CLogic::Backup(const std::string& filename)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        TBuffer        pBuffer(new uint8_t[BUFF_SIZE]);
        uint32_t       chipReadBytes = 0;
        uint32_t       transferBytes = 0;
        const uint32_t totalBytes    = CHIP_COUNT * CHIP_MAX_ADDR;
        SResponse      response;

        m_com.reset(new xserial::ComPort(m_portNum, BAUDRATE), [](xserial::ComPort* pCom) {
            pCom->close();
            delete pCom;
        });

        if (!m_com->getStateComPort())
        {
            // TODO Error message: Failed to open port
            return false;
        }

        boost::shared_ptr<std::ofstream> pFile(
            new std::ofstream(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc),
            [](std::ofstream* f) {
                f->close();
                delete f;
            });

        if (!pFile->is_open())
        {
            // TODO Set error message
            return false;
        }

        if (m_callbackProgress) m_callbackProgress(0);

        for (uint8_t chip = 0; chip < CHIP_COUNT; ++chip)
        {
            // Select chip
            if (ComRPC(SRequest { ECommand::CHIP_SELECT, (uint16_t)chip, nullptr }).code != OK)
            {
                // TODO Set error message
                return false;
            }

            // Set address
            if (ComRPC(SRequest { ECommand::ADDRESS, 0, nullptr }).code != OK)
            {
                // TODO Set error message
                return false;
            }

            chipReadBytes = 0;
            do
            {
                // Read EEPROM
                response = ComRPC(SRequest { ECommand::READ, BUFF_SIZE, pBuffer.get() });
                if (response.code != OK)
                {
                    // TODO Set error message
                    return false;
                }

                pFile->write((char*)pBuffer.get(), response.length);

                chipReadBytes += response.length;
                transferBytes += response.length;

                if (m_callbackProgress) m_callbackProgress((int)100 * transferBytes / totalBytes);
            } while (chipReadBytes < CHIP_MAX_ADDR);
        }

        if (m_callbackProgress) m_callbackProgress(100);
        return true;
    }

    bool CLogic::Restore(const std::string& filename)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        TBuffer        pBuffer(new uint8_t[BUFF_SIZE]);
        uint32_t       chipWriteBytes = 0;
        uint32_t       transferBytes  = 0;
        const uint32_t totalBytes     = CHIP_COUNT * CHIP_MAX_ADDR;
        SResponse      response;

        if (!(boost::filesystem::exists(filename) && boost::filesystem::is_regular_file(filename)))
        {
            // TODO Set error message
            return false;
        }

        boost::shared_ptr<std::ifstream> pFile(
            new std::ifstream(filename, std::ios_base::in | std::ios_base::binary), [](std::ifstream* f) {
                f->close();
                delete f;
            });

        if (!pFile->is_open())
        {
            // TODO Set error message
            return false;
        }

        if (m_callbackProgress) m_callbackProgress(0);

        for (uint8_t chip = 0; chip < CHIP_COUNT; ++chip)
        {
            // Select chip
            if (ComRPC(SRequest { ECommand::CHIP_SELECT, (uint16_t)chip, nullptr }).code != OK)
            {
                // TODO Set error message
                return false;
            }

            // Set address
            if (ComRPC(SRequest { ECommand::ADDRESS, 0, nullptr }).code != OK)
            {
                // TODO Set error message
                return false;
            }

            chipWriteBytes = 0;
            do
            {
                pFile->read((char*)pBuffer.get(), BUFF_SIZE);

                if (pFile->gcount() == 0) break;

                // Write EEPROM
                response = ComRPC(SRequest { ECommand::WRITE, (uint16_t)pFile->gcount(), pBuffer.get() });
                if (response.code != OK)
                {
                    // TODO Set error message
                    return false;
                }

                chipWriteBytes += response.length;
                transferBytes += response.length;

                if (m_callbackProgress) m_callbackProgress((int)100 * transferBytes / totalBytes);
            } while (chipWriteBytes < CHIP_MAX_ADDR);
        }

        if (m_callbackProgress) m_callbackProgress(100);
        return true;
    }

    std::vector<std::string> CLogic::GetPortList()
    {
        xserial::ComPort         com;
        std::vector<std::string> portList;

        com.getListSerialPorts(portList);

        return portList;
    }

    CLogic::SResponse CLogic::ComRPC(CLogic::SRequest data)
    {
#if ARDUINO_BRIDGE
        uint16_t buffSize = sizeof(data.code) + sizeof(data.param);
        if (data.code == ECommand::WRITE) buffSize += data.param;

        TBuffer   pBuffer(new uint8_t[buffSize]);
        SResponse response;
        SRequest* req = reinterpret_cast<SRequest*>(pBuffer.get());

        // Make request + payload (if exist)
        req->code  = data.code;
        req->param = data.param;
        if (data.code == ECommand::WRITE) memcpy(&req->pPayload, data.pPayload, data.param);

        // Send request
        if (!m_com->write((char*)req, buffSize)) return SResponse { EStatus::ERR, ERR_WRITE };

        // Get status
        if (m_com->read((char*)&response, sizeof(response)) != sizeof(response))
            return SResponse { EStatus::ERR, ERR_ANSWER };

        if (response.code == EStatus::OK && data.code == ECommand::READ)
        {
            // Get payload
            if (m_com->read((char*)data.pPayload, response.length) != response.length)
                return SResponse { EStatus::ERR, ERR_DATA_PARTIAL };
        }

        return response;
#else
        // TODO For USB-I2C converter
        return SResponse { EStatus::ERR, 0xFF };
#endif // ARDUINO_BRIDGE
    }
} // namespace MemBlockTool