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
 * @brief  Line sensors array realization
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "LineSensors.h"

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

bool LineSensors::isCalibrationSuccessful()
{
    bool isSuccessful = false;

    m_calibErrorInfo = CALIB_ERROR_NOT_CALIBRATED;

    if ((nullptr != m_lineSensors.calibratedMinimumOn) &&
        (nullptr != m_lineSensors.calibratedMaximumOn) &&
        (nullptr != m_lineSensors.calibratedMinimumOff) &&
        (nullptr != m_lineSensors.calibratedMaximumOff))
    {
        uint8_t index = 0;

        isSuccessful = true;
        while((MAX_SENSORS > index) && (true == isSuccessful))
        {
            uint16_t distance = 0;
            
            /* Check whether the max. value is really greater than the min. value.
             * It can happen that someone try to calibrate over a blank surface.
             */
            if (m_lineSensors.calibratedMaximumOn[index] > m_lineSensors.calibratedMinimumOn[index])
            {
                distance = m_lineSensors.calibratedMaximumOn[index] - m_lineSensors.calibratedMinimumOn[index];
            }

            /* The assumption here is, that the distance (max. value - min. value) must be
             * higher than a quarter of the max. measure duration.
             */
            if ((MEASURE_DURATION / 4) > distance)
            {
                m_calibErrorInfo = index;
                isSuccessful = false;
            }

            ++index;
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
