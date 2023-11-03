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
    bool isInitializationSuccessful = false;
    if ((nullptr != m_accelerometer) && (nullptr != m_gyro) && (nullptr != m_magnetometer))
    {
        m_accelerometer->enable(m_simTime.getTimeStep());
        m_gyro->enable(m_simTime.getTimeStep());
        m_magnetometer->enable(m_simTime.getTimeStep());
        isInitializationSuccessful = true;
    }
    return isInitializationSuccessful;
}

void IMU::readAccelerometer()
{
    if (nullptr != m_accelerometer)
    {
        const double* accelerationValues = m_accelerometer->getValues();
        if (nullptr != accelerationValues)
        {
            m_accelerationValues.valueX = convertFromDoubleToInt16(accelerationValues[0]); /** Index 0 is the x-axis. */
            m_accelerationValues.valueY = convertFromDoubleToInt16(accelerationValues[1]); /** Index 1 is the y-axis. */
            m_accelerationValues.valueZ = convertFromDoubleToInt16(accelerationValues[2]); /** Index 2 is the z-axis. */
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
            m_gyroValues.valueX = convertFromDoubleToInt16(gyroValues[0]); /** Index 0 is the x-axis. */
            m_gyroValues.valueY = convertFromDoubleToInt16(gyroValues[1]); /** Index 1 is the y-axis. */
            m_gyroValues.valueZ = convertFromDoubleToInt16(gyroValues[2]); /** Index 2 is the z-axis. */
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
            m_magnetometerValues.valueX = convertFromDoubleToInt16(magnetometerValues[0]); /** Index 0 is the x-axis. */
            m_magnetometerValues.valueY = convertFromDoubleToInt16(magnetometerValues[1]); /** Index 1 is the y-axis. */
            m_magnetometerValues.valueZ = convertFromDoubleToInt16(magnetometerValues[2]); /** Index 2 is the z-axis. */
        }
    }
}

void IMU::getAccelerationValues(IMUData* accelerationValues)
{
    if (nullptr != accelerationValues)
    {
        accelerationValues->valueX = m_accelerationValues.valueX;
        accelerationValues->valueY = m_accelerationValues.valueY;
        accelerationValues->valueZ = m_accelerationValues.valueZ;
    }
}

void IMU::getTurnRates(IMUData* turnRates)
{
    if (nullptr != turnRates)
    {
        turnRates->valueX = m_gyroValues.valueX;
        turnRates->valueY = m_gyroValues.valueY;
        turnRates->valueZ = m_gyroValues.valueZ;
    }
}

void IMU::getMagnetometerValues(IMUData* magnetometerValues)
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
    // TODO: implement TD067
}

int16_t IMU::convertFromDoubleToInt16(double originalValue)
{
    int16_t       convertedValue;
    const int16_t minimumValue = std::numeric_limits<int16_t>::min();
    const int16_t maximumValue = std::numeric_limits<int16_t>::max();

    if (originalValue <= minimumValue)
    {
        convertedValue = minimumValue;
    }
    else if (originalValue >= maximumValue)
    {
        convertedValue = maximumValue;
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
