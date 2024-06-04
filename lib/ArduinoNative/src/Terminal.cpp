/* MIT License
 *
 * Copyright (c) 2023 - 2024 Andreas Merkle <web@blue-andi.de>
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
 *  @brief  Implementation of the Terminal/Console Stream
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdio.h>
#include "Terminal.h"

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

Terminal::Terminal() : Stream()
{
}

Terminal::~Terminal()
{
}

void Terminal::print(const char str[])
{
    printf("%s", str);
}

void Terminal::print(uint8_t value)
{
    printf("%u", value);
}

void Terminal::print(uint16_t value)
{
    printf("%u", value);
}

void Terminal::print(uint32_t value)
{
    printf("%u", value);
}

void Terminal::print(int8_t value)
{
    printf("%d", value);
}

void Terminal::print(int16_t value)
{
    printf("%d", value);
}

void Terminal::print(int32_t value)
{
    printf("%d", value);
}

void Terminal::println(const char str[])
{
    printf("%s\n", str);
}

void Terminal::println(uint8_t value)
{
    printf("%u\n", value);
}

void Terminal::println(uint16_t value)
{
    printf("%u\n", value);
}

void Terminal::println(uint32_t value)
{
    printf("%u\n", value);
}

void Terminal::println(int8_t value)
{
    printf("%d\n", value);
}

void Terminal::println(int16_t value)
{
    printf("%d\n", value);
}

void Terminal::println(int32_t value)
{
    printf("%d\n", value);
}

size_t Terminal::write(const uint8_t* buffer, size_t length)
{
    size_t count = 0;

    if ((nullptr != buffer) && (0U != length))
    {
        for (count = 0; count < length; count++)
        {
            printf("%u", buffer[count]);
        }
    }

    return count;
}

int Terminal::available() const
{
    /* Not implemented*/
    return 0;
}

size_t Terminal::readBytes(uint8_t* buffer, size_t length)
{
    /* Not implemented*/
    (void) buffer;
    (void) length;
    return 0;
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
