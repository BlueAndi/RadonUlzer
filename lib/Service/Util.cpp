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

/******************************************************************************
 * Local Functions
 *****************************************************************************/
