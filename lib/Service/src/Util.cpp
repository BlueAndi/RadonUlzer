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
#include <RobotConstants.h>

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

uint32_t Util::divRoundUp(uint32_t numerator, uint32_t denominator)
{
    uint32_t result    = numerator / denominator;
    uint32_t rest      = numerator % denominator;
    uint32_t threshold = denominator / 2U;

    if (threshold <= rest)
    {
        result += 1U;
    }

    return result;
}

int32_t Util::divRoundUp(int32_t numerator, int32_t denominator)
{
    int32_t result    = numerator / denominator;
    int32_t rest      = numerator % denominator;
    int32_t threshold = denominator / 2;

    if (0 > rest)
    {
        rest *= -1;
    }

    if (0 > threshold)
    {
        threshold *= -1;
    }

    if (threshold <= rest)
    {
        if (0 > result)
        {
            result -= 1;
        }
        else
        {
            result += 1;
        }
    }

    return result;
}

bool Util::isButtonTriggered(IButton& button, bool& lastState)
{
    bool isTriggered = false;
    bool isPressed   = button.isPressed();

    if ((false == isPressed) && (true == lastState))
    {
        isTriggered = true;
    }

    lastState = isPressed;

    return isTriggered;
}

int32_t Util::stepsPerSecondToMillimetersPerSecond(int16_t speedStepsPerSec)
{
    return (static_cast<int32_t>(speedStepsPerSec) * 1000 / static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_M));
}

int16_t Util::millimetersPerSecondToStepsPerSecond(int32_t speedMmPerSec)
{
    int32_t speedStepsPerSec = speedMmPerSec * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_M);
    return (static_cast<int16_t>(speedStepsPerSec / 1000));
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
