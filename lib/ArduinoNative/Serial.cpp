/* MIT License
 *
 * Copyright (c) 2023 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 *  @brief  Implementation of Arduino Serial
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "Serial.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

Serial_::Serial_(Stream& stream) : Stream(), m_stream(stream)
{
}

Serial_::~Serial_()
{
}

void Serial_::begin(unsigned long baudrate)
{
    (void)baudrate;
}

void Serial_::end()
{
}

void Serial_::print(const char str[])
{
    m_stream.print(str);
}

void Serial_::print(uint8_t value)
{
    m_stream.print(value);
}

void Serial_::print(uint16_t value)
{
    m_stream.print(value);
}

void Serial_::print(uint32_t value)
{
    m_stream.print(value);
}

void Serial_::print(int8_t value)
{
    m_stream.print(value);
}

void Serial_::print(int16_t value)
{
    m_stream.print(value);
}

void Serial_::print(int32_t value)
{
    m_stream.print(value);
}

void Serial_::println(const char str[])
{
    m_stream.println(str);
}

void Serial_::println(uint8_t value)
{
    m_stream.println(value);
}

void Serial_::println(uint16_t value)
{
    m_stream.println(value);
}

void Serial_::println(uint32_t value)
{
    m_stream.println(value);
}

void Serial_::println(int8_t value)
{
    m_stream.println(value);
}

void Serial_::println(int16_t value)
{
    m_stream.println(value);
}

void Serial_::println(int32_t value)
{
    m_stream.println(value);
}

size_t Serial_::write(const uint8_t* buffer, size_t length)
{
    return m_stream.write(buffer, length);
}

int Serial_::available() const
{
    return m_stream.available();
}

size_t Serial_::readBytes(uint8_t* buffer, size_t length)
{
    return m_stream.readBytes(buffer, length);
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
