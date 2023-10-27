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
 * @brief  Utilities
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Util.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

void Util::uintToStr(char* str, size_t size, uint32_t value)
{
    if ((nullptr != str) && (0 < size))
    {
        size_t   idx     = 0;
        uint8_t  digits  = 10; /* 0 - 4294967295 */
        uint32_t divisor = 1000000000;

        while (((size - 1) > idx) && (0 < digits))
        {
            char digit = static_cast<char>(value / divisor);

            /* No preceeding zeros. */
            if (0 == idx)
            {
                if ((0 < digit) || (1 == digits))
                {
                    str[idx] = digit + '0';
                    ++idx;
                }
            }
            else
            {
                str[idx] = digit + '0';
                ++idx;
            }

            --digits;
            value %= divisor;
            divisor /= 10;
        }

        str[idx] = '\0';
    }
}

void Util::intToStr(char* str, size_t size, int32_t value)
{
    if ((nullptr != str) && (0 < size))
    {
        size_t   idx     = 0;
        uint8_t  digits  = 10; /* -2147483648 - 2147483647 */
        uint32_t divisor = 1000000000;

        if (0 > value)
        {
            str[idx] = '-';
            ++idx;

            value *= -1;
        }

        while (((size - 1) > idx) && (0 < digits))
        {
            char digit = static_cast<char>(value / divisor);

            /* No preceeding zeros. */
            if (0 == idx)
            {
                if ((0 < digit) || (1 == digits))
                {
                    str[idx] = digit + '0';
                    ++idx;
                }
            }
            else
            {
                str[idx] = digit + '0';
                ++idx;
            }

            --digits;
            value %= divisor;
            divisor /= 10;
        }

        str[idx] = '\0';
    }
}

void Util::int16ToByteArray(uint8_t* buffer, size_t size, int16_t value)
{
    if ((nullptr != buffer) && (sizeof(int16_t) <= size))
    {
        buffer[0U] = ((value >> 8U) & 0xFF);
        buffer[1U] = ((value >> 0U) & 0xFF);
    }
}

void Util::uint16ToByteArray(uint8_t* buffer, size_t size, uint16_t value)
{
    if ((nullptr != buffer) && (sizeof(uint16_t) <= size))
    {
        buffer[0U] = ((value >> 8U) & 0xFF);
        buffer[1U] = ((value >> 0U) & 0xFF);
    }
}

void Util::int32ToByteArray(uint8_t* buffer, size_t size, int32_t value)
{
    if ((nullptr != buffer) && (sizeof(int32_t) <= size))
    {
        uint16_t hiBytes  = ((value >> 16U) & 0xFFFF);
        uint16_t lowBytes = ((value >>  0U) & 0xFFFF);

        buffer[0U] = ((hiBytes  >> 8U) & 0xFF);
        buffer[1U] = ((hiBytes  >> 0U) & 0xFF);
        buffer[2U] = ((lowBytes >> 8U) & 0xFF);
        buffer[3U] = ((lowBytes >> 0U) & 0xFF);
    }
}

void Util::uint32ToByteArray(uint8_t* buffer, size_t size, uint32_t value)
{
    if ((nullptr != buffer) && (sizeof(uint32_t) <= size))
    {
        uint16_t hiBytes  = ((value >> 16U) & 0xFFFF);
        uint16_t lowBytes = ((value >>  0U) & 0xFFFF);

        buffer[0U] = ((hiBytes  >> 8U) & 0xFF);
        buffer[1U] = ((hiBytes  >> 0U) & 0xFF);
        buffer[2U] = ((lowBytes >> 8U) & 0xFF);
        buffer[3U] = ((lowBytes >> 0U) & 0xFF);
    }
}

bool Util::byteArrayToInt16(const uint8_t* buffer, size_t size, int16_t& value)
{
    bool isSuccess = false;

    if ((nullptr != buffer) && (sizeof(int16_t) <= size))
    {
        value = ((static_cast<int16_t>(buffer[0U]) << 8U) & 0xFF00) |
                ((static_cast<int16_t>(buffer[1U]) << 0U) & 0x00FF) ;

        isSuccess = true;
    }

    return isSuccess;
}

bool Util::byteArrayToUint16(const uint8_t* buffer, size_t size, uint16_t& value)
{
    bool isSuccess = false;

    if ((nullptr != buffer) && (sizeof(uint16_t) <= size))
    {
        value = ((static_cast<uint16_t>(buffer[0U]) << 8U) & 0xFF00) |
                ((static_cast<uint16_t>(buffer[1U]) << 0U) & 0x00FF) ;

        isSuccess = true;
    }

    return isSuccess;
}

bool Util::byteArrayToUint32(const uint8_t* buffer, size_t size, uint32_t& value)
{
    bool isSuccess = false;

    if ((nullptr != buffer) && (sizeof(uint32_t) <= size))
    {
        value = ((static_cast<uint32_t>(buffer[0U]) << 24U) & 0xFF000000) |
                ((static_cast<uint32_t>(buffer[1U]) << 16U) & 0x00FF0000) |
                ((static_cast<uint32_t>(buffer[2U]) <<  8U) & 0x0000FF00) |
                ((static_cast<uint32_t>(buffer[3U]) <<  0U) & 0x000000FF) ;

        isSuccess = true;
    }

    return isSuccess;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
