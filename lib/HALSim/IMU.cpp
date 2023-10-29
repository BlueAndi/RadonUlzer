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
    m_accelerometer->enable(m_simTime.getTimeStep());
    m_gyro->enable(m_simTime.getTimeStep());
    m_magnetometer->enable(m_simTime.getTimeStep());   
    return true; 
}

void IMU::readAcc()
{    
    const double * accelerationValues = m_accelerometer->getValues();
    if (accelerationValues != nullptr) {
    for (uint8_t axisIndex = 0; axisIndex < NUMBER_OF_AXIS; ++axisIndex)
        {
            m_accelerationValues[axisIndex] = static_cast<int16_t>(accelerationValues[axisIndex]);
        }
    }
}

void IMU::readGyro()
{    
    const double * gyroValues = m_gyro->getValues();
    
    if (gyroValues != nullptr) 
    {
        for (uint8_t axisIndex = 0; axisIndex < NUMBER_OF_AXIS; ++axisIndex)
        {
            m_gyroValues[axisIndex] = static_cast<int16_t>(gyroValues[axisIndex]);
        }
    }
}

void IMU::readMag()
{    
    const double * magnetometerValues = m_magnetometer->getValues();
    
    if (magnetometerValues != nullptr) 
    {
        for (uint8_t axisIndex = 0; axisIndex < NUMBER_OF_AXIS; ++axisIndex)
        {
            m_magnetometerValues[axisIndex] = static_cast<int16_t>(magnetometerValues[axisIndex]);
        }
    }
}

void IMU::getAccelerationValues(int16_t* accelerationValues)
{
    for (uint8_t axisIndex = 0; axisIndex < NUMBER_OF_AXIS; ++axisIndex)
    {
        accelerationValues[axisIndex] = m_accelerationValues[axisIndex];
    }
}

void IMU::getTurnRates(int16_t* turnRates)
{
    for (uint8_t axisIndex = 0; axisIndex < NUMBER_OF_AXIS; ++axisIndex)
    {
        turnRates[axisIndex] = m_gyroValues[axisIndex];
    }
}

void IMU::getMagnetometerValues(int16_t* magnetometerValues)
{
    for (uint8_t axisIndex = 0; axisIndex < NUMBER_OF_AXIS; ++axisIndex)
    {
        magnetometerValues[axisIndex] = m_magnetometerValues[axisIndex];
    }
}

void IMU::calibrate()
{
    // TODO: implement TD067
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
