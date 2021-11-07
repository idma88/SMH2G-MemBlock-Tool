#pragma once

namespace MemBlockTool {
  class SimpleBuffer
  {
  public:
    /**
     * Конструктор по-умолчанию
     */
    SimpleBuffer()
    {}

    /**
     * Конструктор
     * @param length - [in] Размер выделяемого буфера
     */
    SimpleBuffer(int32_t length)
    {
      Alloc(length);
    }

    /**
     * Деструктор
     */
    ~SimpleBuffer()
    {
      Free();
    }

    /**
     * Выделяет или перевыделяет буфер
     * @param length - [in] Размер выделяемого буфера
     * @return Указатель на выделенный буфер или nullptr в случае ошибки
     */
    uint8_t* Alloc(int32_t length)
    {
      Free();

      if (length) {
        m_ptr  = new uint8_t[length];
        m_size = length;
      }
      return m_ptr;
    }

    /**
     * Освобождает выделенную память
     */
    void Free()
    {
      if (m_ptr == nullptr) return;

      delete[] m_ptr;
      m_size = 0;
    }

    /**
     * Указатель на выделенный буфер
     * @return Указатель на выделенный буфер или nullptr в случае пустого буфера/ошибки
     */
    uint8_t* Get()
    {
      return m_ptr;
    }

    /**
     * Размер выделенного буфера
     * @return Размер выделенного буфера или ноль в случае пустого буфера/ошибки
     */
    uint16_t Size()
    {
      return m_size;
    }

  private:
    uint8_t* m_ptr  = nullptr; ///< Указатель на буфер для данных
    uint16_t m_size = 0;       ///< Размер буфера

  }; // class SimpleBuffer
} // namespace MemBlockTool