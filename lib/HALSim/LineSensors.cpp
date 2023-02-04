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

const uint16_t* LineSensors::getSensorValues()
{
    m_sensorValuesU16[0] = static_cast<uint16_t>(m_lightSensor0->getValue());
    m_sensorValuesU16[1] = static_cast<uint16_t>(m_lightSensor1->getValue());
    m_sensorValuesU16[2] = static_cast<uint16_t>(m_lightSensor2->getValue());
    m_sensorValuesU16[3] = static_cast<uint16_t>(m_lightSensor3->getValue());
    m_sensorValuesU16[4] = static_cast<uint16_t>(m_lightSensor4->getValue());
    return m_sensorValuesU16;
}

int16_t LineSensors::readLine()
{
    float numerator = (0 * m_sensorValuesU16[0] + 1000 * m_sensorValuesU16[1] + 
                       2000 * m_sensorValuesU16[2] + 3000 * m_sensorValuesU16[3] + 
                       4000 * m_sensorValuesU16[4]);
    float denominator = (m_sensorValuesU16[0] + m_sensorValuesU16[1] + m_sensorValuesU16[2] + 
                         m_sensorValuesU16[3] + m_sensorValuesU16[4]);

    /* Check to avoid division by zero. */
    if(denominator <= 0.0f)
    {
        denominator = 1.0f;
    }

    return static_cast<int16_t>(numerator/denominator);
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
