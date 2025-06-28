/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef UTIL_H
#define UTIL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <IButton.h>
#include <RobotConstants.h>

/**
 * Utilities
 */
namespace Util
{

    /******************************************************************************
     * Macros
     *****************************************************************************/

    /******************************************************************************
     * Types and Classes
     *****************************************************************************/

    /******************************************************************************
     * Functions
     *****************************************************************************/

    /**
     * Unsigned integer to string, without preceeding zeros.
     *
     * @param[out]  str     Destination string
     * @param[in]   size    Size of the destination string in byte
     * @param[in]   value   Value
     */
    void uintToStr(char* str, size_t size, uint32_t value);

    /**
     * Signed integer to string, without preceeding zeros.
     *
     * @param[out]  str     Destination string
     * @param[in]   size    Size of the destination string in byte
     * @param[in]   value   Value
     */
    void intToStr(char* str, size_t size, int32_t value);

    /**
     * Divide and round.
     *
     * @param[in] numerator     The numerator.
     * @param[in] denominator   The denominator.
     *
     * @return Result
     */
    uint32_t divRoundUp(uint32_t numerator, uint32_t denominator);

    /**
     * Divide and round.
     *
     * @param[in] numerator     The numerator.
     * @param[in] denominator   The denominator.
     *
     * @return Result
     */
    int32_t divRoundUp(int32_t numerator, int32_t denominator);

    /**
     * Is button triggered?
     * Triggered means a pressed/released change.
     *
     * @param[in]       button      The button.
     * @param[inout]    lastState   The last button state.
     *
     * @return If button is triggered, it will return true otherwise false.
     */
    bool isButtonTriggered(IButton& button, bool& lastState);

    /**
     * Convert a speed in encoder steps per second to a speed in mm/s.
     *
     * @param[in]   speedStepsPerSec Speed in encoder steps per second.
     *
     * @return Speed in mm/s.
     */
    inline int32_t stepsPerSecondToMillimetersPerSecond(int16_t speedStepsPerSec)
    {
        return (static_cast<int32_t>(speedStepsPerSec) * 1000 /
                static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_M));
    }

    /**
     * Convert a speed in mm/s to a speed in encoder steps per second.
     *
     * @param[in]   speedMmPerSec Speed in mm/s.
     *
     * @return Speed in encoder steps per second.
     */
    inline int16_t millimetersPerSecondToStepsPerSecond(int32_t speedMmPerSec)
    {
        int32_t speedStepsPerSec = speedMmPerSec * static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_M);
        return (static_cast<int16_t>(speedStepsPerSec / 1000));
    }

} /* namespace Util */

#endif /* UTIL_H */
/** @} */
