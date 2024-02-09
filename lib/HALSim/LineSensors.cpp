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

void LineSensors::init()
{
    for (uint8_t sensorIndex = 0; sensorIndex < MAX_SENSORS; ++sensorIndex)
    {
        m_sensorValuesU16[sensorIndex] = 0;
        m_sensorMaxValues[sensorIndex] = 0;
        m_sensorMinValues[sensorIndex] = SENSOR_MAX_VALUE;
    }
}

void LineSensors::calibrate()
{
    (void)getSensorValues();

    for (uint8_t sensorIndex = 0; sensorIndex < MAX_SENSORS; ++sensorIndex)
    {
        if (m_sensorValuesU16[sensorIndex] < m_sensorMinValues[sensorIndex])
        {
            m_sensorMinValues[sensorIndex] = m_sensorValuesU16[sensorIndex];
        }

        if (m_sensorValuesU16[sensorIndex] > m_sensorMaxValues[sensorIndex])
        {
            m_sensorMaxValues[sensorIndex] = m_sensorValuesU16[sensorIndex];
        }
    }

    m_sensorCalibStarted = true;
}

int16_t LineSensors::readLine()
{
    uint32_t       estimatedPos = 0;
    uint32_t       numerator    = 0;
    uint32_t       denominator  = 0;
    const uint32_t WEIGHT       = 1000;
    bool           isOnLine     = false;

    (void)getSensorValues();

    for (uint32_t idx = 0; idx < MAX_SENSORS; ++idx)
    {
        uint32_t sensorValue = m_sensorValuesU16[idx];

        numerator += idx * WEIGHT * sensorValue;
        denominator += sensorValue;

        /* Keep track of whether we see the line at all. */
        if (SENSOR_OFF_LINE_THRESHOLD < sensorValue)
        {
            isOnLine = true;
        }
    }

    if (false == isOnLine)
    {
        /* If it last read to the left of center, return 0. */
        if (m_lastPosValue < (((MAX_SENSORS - 1) * SENSOR_MAX_VALUE) / 2))
        {
            estimatedPos = 0;
        }
        /* If it last read to the right of center, return the max. value. */
        else
        {
            estimatedPos = (MAX_SENSORS - 1) * SENSOR_MAX_VALUE;
        }
    }
    else
    {
        /* Check to avoid division by zero. */
        if (0 != denominator)
        {
            estimatedPos = numerator / denominator;
        }

        m_lastPosValue = estimatedPos;
    }

    return static_cast<int16_t>(estimatedPos);
}

const uint16_t* LineSensors::getSensorValues()
{
    for (uint8_t sensorIndex = 0; sensorIndex < MAX_SENSORS; ++sensorIndex)
    {
        if (nullptr != m_lightSensors[sensorIndex])
        {
            m_sensorValuesU16[sensorIndex] = static_cast<uint16_t>(m_lightSensors[sensorIndex]->getValue());
        }
    }

    return m_sensorValuesU16;
}

bool LineSensors::isCalibrationSuccessful()
{
    bool isSuccessful = false;

    m_calibErrorInfo = CALIB_ERROR_NOT_CALIBRATED;

    if (true == m_sensorCalibStarted)
    {
        uint8_t index = 0;

        isSuccessful = true;
        while ((MAX_SENSORS > index) && (true == isSuccessful))
        {
            uint16_t distance = 0;

            /* Check whether the max. value is really greater than the min. value.
             * It can happen that someone try to calibrate over a blank surface.
             */
            if (m_sensorMaxValues[index] > m_sensorMinValues[index])
            {
                distance = m_sensorMaxValues[index] - m_sensorMinValues[index];
            }

            /* The assumption here is, that the distance (max. value - min. value) must be
             * higher than a quarter of the max. measured duration.
             */
            if ((SENSOR_MAX_VALUE / 4) > distance)
            {
                m_calibErrorInfo = index;
                isSuccessful     = false;
            }

            ++index;
        }
    }

    return isSuccessful;
}

void LineSensors::resetCalibration()
{
    for (uint8_t sensorIndex = 0; sensorIndex < MAX_SENSORS; ++sensorIndex)
    {
        m_sensorValuesU16[sensorIndex] = 0;
        m_sensorMaxValues[sensorIndex] = 0;
        m_sensorMinValues[sensorIndex] = SENSOR_MAX_VALUE;
    }

    m_sensorCalibStarted = false;
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
