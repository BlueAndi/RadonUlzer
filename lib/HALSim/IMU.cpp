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
 * @brief  IMU implementation for the Simulation
 * @author Juliane Kerpe <juliane.kerpe@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IMU.h"
#include <limits>
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

bool IMU::init()
{
    /* Nothing to do. */
    return true;
}

void IMU::readAccelerometer()
{
    if (nullptr != m_accelerometer)
    {
        const double* accelerationValues = m_accelerometer->getValues();
        if (nullptr != accelerationValues)
        {
            m_accelerationValues.valueX = convertFromDoubleToInt16(accelerationValues[0]); /* Index 0 is the x-axis. */
            m_accelerationValues.valueY = convertFromDoubleToInt16(accelerationValues[1]); /* Index 1 is the y-axis. */
            m_accelerationValues.valueZ = convertFromDoubleToInt16(accelerationValues[2]); /* Index 2 is the z-axis. */
        }
    }
}

void IMU::readGyro()
{
    if (nullptr != m_gyro)
    {
        const double* gyroValues = m_gyro->getValues();

        if (nullptr != gyroValues)
        {
            m_gyroValues.valueX = convertFromDoubleToInt16(gyroValues[0]); /* Index 0 is the x-axis. */
            m_gyroValues.valueY = convertFromDoubleToInt16(gyroValues[1]); /* Index 1 is the y-axis. */
            m_gyroValues.valueZ = convertFromDoubleToInt16(gyroValues[2]); /* Index 2 is the z-axis. */
        }
    }
}

void IMU::readMagnetometer()
{
    if (nullptr != m_magnetometer)
    {
        const double* magnetometerValues = m_magnetometer->getValues();

        if (nullptr != magnetometerValues)
        {
            m_magnetometerValues.valueX = convertFromDoubleToInt16(magnetometerValues[0]); /* Index 0 is the x-axis. */
            m_magnetometerValues.valueY = convertFromDoubleToInt16(magnetometerValues[1]); /* Index 1 is the y-axis. */
            m_magnetometerValues.valueZ = convertFromDoubleToInt16(magnetometerValues[2]); /* Index 2 is the z-axis. */
        }
    }
}

const void IMU::getAccelerationValues(IMUData* accelerationValues)
{
    if (nullptr != accelerationValues)
    {
        accelerationValues->valueX = m_accelerationValues.valueX;
        accelerationValues->valueY = m_accelerationValues.valueY;
        accelerationValues->valueZ = m_accelerationValues.valueZ;
    }
}

const void IMU::getTurnRates(IMUData* turnRates)
{
    if (nullptr != turnRates)
    {
        turnRates->valueX = m_gyroValues.valueX;
        turnRates->valueY = m_gyroValues.valueY;
        turnRates->valueZ = m_gyroValues.valueZ;
    }
}

const void IMU::getMagnetometerValues(IMUData* magnetometerValues)
{
    if (nullptr != magnetometerValues)
    {
        magnetometerValues->valueX = m_magnetometerValues.valueX;
        magnetometerValues->valueY = m_magnetometerValues.valueY;
        magnetometerValues->valueZ = m_magnetometerValues.valueZ;
    }
}

void IMU::calibrate()
{
    /* Nothing to do. */
}

int16_t IMU::convertFromDoubleToInt16(double originalValue)
{
    int16_t convertedValue;

    if (originalValue <= INT16_MIN)
    {
        convertedValue = INT16_MIN;
    }
    else if (originalValue >= INT16_MAX)
    {
        convertedValue = INT16_MAX;
    }
    else
    {
        convertedValue = static_cast<int16_t>(originalValue);
    }
    return convertedValue;
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
