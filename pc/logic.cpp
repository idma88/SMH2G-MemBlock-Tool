// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "Logic.h"

#define ERR_MSG_OPEN_PORT    "Не удалось открыть порт"
#define ERR_MSG_OPEN_FILE    "Не удалось открыть файл"
#define ERR_MSG_SELECT_CHIP  "Не удалось выбрать чип EEPROM"
#define ERR_MSG_SET_ADDRESS  "Не удалось установить адрес в EEPROM"
#define ERR_MSG_READ_EEPROM  "Не удалось прочитать данные из EEPROM"
#define ERR_MSG_WRITE_EEPROM "Не удалось прочитать данные из EEPROM"

namespace MemBlockTool {
  Logic::Logic()
    : ErrorMessage()
    , m_portNum(0)
    , m_callbackProgress(nullptr)
    , m_callbackTaskEnd(nullptr)
  {}

  Logic::~Logic()
  {
    m_com->close();

    if (m_proc.joinable()) m_proc.join();
  }

  bool Logic::SetPort(const std::string& port)
  {
    if (port.length() == 0) return false;

    std::vector<std::string> portList = GetPortList();

    auto iequals = [](char a, char b) { return tolower(a) == tolower(b); };

    std::vector<std::string>::iterator it =
      std::find_if(portList.begin(), portList.end(), [&port, &iequals](const std::string& item) {
        return std::equal(port.begin(), port.end(), item.begin(), item.end(), iequals);
      });

    if (it == portList.end()) return false;

    m_portNum = atoi(port.c_str() + strlen("COM"));

    return true;
  }

  bool Logic::SetCallbackProgress(const TCallbackProgress& callback)
  {
    if (callback == nullptr) return false;

    m_callbackProgress = callback;
    return true;
  }

  bool Logic::SetCallbackTaskEnd(const TCallbackTaskEnd& callback)
  {
    if (callback == nullptr) return false;

    m_callbackTaskEnd = callback;
    return true;
  }

  bool Logic::Backup(const std::string& filename)
  {
    std::unique_lock<std::mutex> lock(m_mutexProc);

    m_proc.swap(std::thread(&Logic::BackupProc, this, filename));

    return true;
  }

  bool Logic::Restore(const std::string& filename)
  {
    std::unique_lock<std::mutex> lock(m_mutexProc);

    m_proc.swap(std::thread(&Logic::RestoreProc, this, filename));

    return true;
  }

  std::vector<std::string> Logic::GetPortList()
  {
    xserial::ComPort         com;
    std::vector<std::string> portList;

    com.getListSerialPorts(portList);

    return portList;
  }

  Response Logic::ComRPC(Request data)
  {
#if ARDUINO_BRIDGE
    unsigned long _readed;

    uint16_t buffSize = sizeof(data.code) + sizeof(data.param);
    if (data.code == CommandCode::WRITE) buffSize += data.param;

    TBuffer  pBuffer(buffSize);
    Response response;
    Request* req = reinterpret_cast<Request*>(pBuffer.data());

    // Make request + payload (if exist)
    req->code  = data.code;
    req->param = data.param;
    if (data.code == CommandCode::WRITE) memcpy(&req->pPayload, data.pPayload, data.param);

    // Send request
    if (!m_com->write((char*)req, buffSize)) {
      return Response{ StatusCode::ERR, ERR_WRITE };
    }

    // char tmpBuff[3] = { 0x43, 0, 0 };
    // m_com->write(tmpBuff, 3);

    // uint8_t b[100];
    // m_com->read((char*)&b[0], m_com->bytesToRead());

    // Get status
    while (m_com->bytesToRead() < sizeof(response)) {
    }

    // if (m_com->read((char*)&response, sizeof(response)) != sizeof(response))
    _readed = m_com->read((char*)&response, sizeof(response));
    if (_readed != sizeof(response)) {
      return Response{ StatusCode::ERR, ERR_ANSWER };
    }

    if (response.code == StatusCode::OK && data.code == CommandCode::READ) {
      // Get payload
      // if (m_com->read((char*)data.pPayload, response.param) != response.param)

      while (m_com->bytesToRead() == 0) {
      }

      _readed = m_com->read((char*)data.pPayload, response.param);
      if (_readed != response.param) {
        return Response{ StatusCode::ERR, ERR_DATA_PARTIAL };
      }
    }

    return response;
#else
    // For USB-I2C converter
    return Response{ StatusCode::ERR, 0xFF };
#endif // ARDUINO_BRIDGE
  }

  void Logic::BackupProc(const std::string& filename)
  {
    TBuffer        pBuffer(BUFF_SIZE);
    uint32_t       chipReadBytes = 0;
    uint32_t       transferBytes = 0;
    const uint32_t totalBytes    = CHIP_COUNT * CHIP_MAX_ADDR;
    Response       response;

    m_com.reset(new xserial::ComPort(m_portNum, BAUDRATE), [](xserial::ComPort* pCom) {
      pCom->close();
      delete pCom;
    });

    if (!m_com->getStateComPort()) {
      SetErrorMessage(ERR_MSG_OPEN_PORT);
      if (m_callbackTaskEnd) m_callbackTaskEnd(false);
      return; // false;
    }

    // std::shared_ptr<std::ofstream> pFile(
    //     new std::ofstream(filename, std::ios::out | std::ios::binary | std::ios::trunc),
    //     [](std::ofstream* f) {
    //         f->close();
    //         delete f;
    //     });

    std::ofstream _File(filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

    if (!_File.is_open()) {
      SetErrorMessage(ERR_MSG_OPEN_FILE);
      if (m_callbackTaskEnd) m_callbackTaskEnd(false);
      return; // false;
    }

    if (m_callbackProgress) m_callbackProgress(0);

    for (uint8_t chip = 0; chip < CHIP_COUNT; ++chip) {
      // Select chip
      response = ComRPC(Request{ CommandCode::CHIP_SELECT, (uint16_t)chip, nullptr });
      if (response.code != OK) {
        SetErrorMessage(ERR_MSG_SELECT_CHIP);
        return; // false;
      }

      for (uint32_t addr = 0; addr < CHIP_MAX_ADDR; addr += BUFF_SIZE) {
        // Set address
        response = ComRPC(Request{ CommandCode::ADDRESS, (uint16_t)addr, nullptr });
        if (response.code != OK) {
          SetErrorMessage(ERR_MSG_SET_ADDRESS);
          if (m_callbackTaskEnd) m_callbackTaskEnd(false);
          return; // false;
        }

        // Read EEPROM
        response = ComRPC(Request{ CommandCode::READ, BUFF_SIZE, pBuffer.data() });
        if (response.code != OK) {
          SetErrorMessage(ERR_MSG_READ_EEPROM);
          if (m_callbackTaskEnd) m_callbackTaskEnd(false);
          return; // false;
        }

        _File.write((char*)pBuffer.data(), response.param);

        chipReadBytes += response.param;
        transferBytes += response.param;

        if (m_callbackProgress) m_callbackProgress((int)100 * transferBytes / totalBytes);
      }
    }

    if (m_callbackProgress) m_callbackProgress(100);
    if (m_callbackTaskEnd) m_callbackTaskEnd(true);
    return; // true;
  }

  void Logic::RestoreProc(const std::string& filename)
  {
    TBuffer        pBuffer(BUFF_SIZE);
    uint32_t       chipWriteBytes = 0;
    uint32_t       transferBytes  = 0;
    const uint32_t totalBytes     = CHIP_COUNT * CHIP_MAX_ADDR;
    Response       response;

    m_com.reset(new xserial::ComPort(m_portNum, BAUDRATE), [](xserial::ComPort* pCom) {
      pCom->close();
      delete pCom;
    });

    if (!m_com->getStateComPort()) {
      SetErrorMessage(ERR_MSG_OPEN_PORT);
      if (m_callbackTaskEnd) m_callbackTaskEnd(false);
      return; // false;
    }

    std::ifstream _File(filename.c_str(), std::ios::in | std::ios::binary);

    if (!_File.is_open()) {
      SetErrorMessage(ERR_MSG_OPEN_FILE);
      if (m_callbackTaskEnd) m_callbackTaskEnd(false);
      return; // false;
    }

    if (m_callbackProgress) m_callbackProgress(0);

    for (uint8_t chip = 0; chip < CHIP_COUNT; ++chip) {
      // Select chip
      response = ComRPC(Request{ CommandCode::CHIP_SELECT, (uint16_t)chip, nullptr });
      if (response.code != OK) {
        SetErrorMessage(ERR_MSG_SELECT_CHIP);
        return; // false;
      }

      for (uint32_t addr = 0; addr < CHIP_MAX_ADDR; addr += BUFF_SIZE) {
        // Set address
        response = ComRPC(Request{ CommandCode::ADDRESS, (uint16_t)addr, nullptr });
        if (response.code != OK) {
          SetErrorMessage(ERR_MSG_SET_ADDRESS);
          if (m_callbackTaskEnd) m_callbackTaskEnd(false);
          return; // false;
        }

        _File.read((char*)pBuffer.data(), BUFF_SIZE);

        if (_File.gcount() == 0 || _File.eof()) break;

        // Write EEPROM
        response = ComRPC(Request{ CommandCode::WRITE, (uint16_t)_File.gcount(), pBuffer.data() });
        if (response.code != OK) {
          SetErrorMessage(ERR_MSG_READ_EEPROM);
          if (m_callbackTaskEnd) m_callbackTaskEnd(false);
          return; // false;
        }

        chipWriteBytes += response.param;
        transferBytes += response.param;

        if (m_callbackProgress) m_callbackProgress((int)100 * transferBytes / totalBytes);
      }

      if (_File.gcount() == 0 || _File.eof()) break;
    }

    if (m_callbackProgress) m_callbackProgress(100);
    if (m_callbackTaskEnd) m_callbackTaskEnd(true);
    return; // true;
  }

} // namespace MemBlockTool