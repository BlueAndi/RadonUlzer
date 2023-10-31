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
 * @brief  IMU implementation
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
    bool isInitSuccessful = m_imu.init();
    if (isInitSuccessful) {
        m_imu.enableDefault();
    }
    
    // TD074    Make sure that a Full Scale of 245 dps of the gyros are enough
    // TD075	Make sure that it is valid to make the Full Scale setting in IMU::init()
    switch(m_imu.getType())
    {
    case Zumo32U4IMUType::LSM303D_L3GD20H:
        // 0x00  means +/- 245 dps according to L3GD20H data sheet
        m_imu.writeReg(L3GD20H_ADDR, L3GD20H_REG_CTRL4, 0x00);
        break;
    case Zumo32U4IMUType::LSM6DS33_LIS3MDL:
        // 0x00  means +/- 245 dps according to LSM6DS33 data sheet
        m_imu.writeReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL2_G, 0x00);
        break;
    default:
        ;
    }
    return isInitSuccessful;
}

void IMU::enableDefault()
{
    m_imu.enableDefault();
}

void IMU::configureForTurnSensing()
{
    m_imu.configureForTurnSensing();
}

void IMU::readAcc()
{
    m_imu.readAcc();
    m_accelerometerValues.x = m_imu.a.x;
    m_accelerometerValues.y = m_imu.a.y;
    m_accelerometerValues.z = m_imu.a.z;
}

void IMU::readGyro()
{
    m_imu.readGyro();
    m_gyroValues.x = m_imu.g.x;
    m_gyroValues.y = m_imu.g.y;
    m_gyroValues.z = m_imu.g.z;
}

void IMU::readMag()
{
    m_imu.readMag();
    m_magnetometerValues.x = m_imu.m.x;
    m_magnetometerValues.y = m_imu.m.y;
    m_magnetometerValues.z = m_imu.m.z;
}

bool IMU::accDataReady()
{
    return m_imu.accDataReady();
}

bool IMU::gyroDataReady()
{
    return m_imu.gyroDataReady();
}

bool IMU::magDataReady()
{
    return m_imu.magDataReady();
}

void IMU::getAccelerationValues(int16_t* accelerationValues)
{
    accelerationValues[AXIS_INDEX_X] = m_accelerometerValues.x - m_rawAccelerometerOffsetX;
    accelerationValues[AXIS_INDEX_Y] = m_accelerometerValues.y - m_rawAccelerometerOffsetY;
    accelerationValues[AXIS_INDEX_Z] = m_accelerometerValues.z - m_rawAccelerometerOffsetZ;
}

void IMU::getTurnRates(int16_t* turnRates)
{
    turnRates[AXIS_INDEX_X] = m_gyroValues.x - m_rawGyroOffsetX;
    turnRates[AXIS_INDEX_Y] = m_gyroValues.y - m_rawGyroOffsetY;
    turnRates[AXIS_INDEX_Z] = m_gyroValues.z - m_rawGyroOffsetZ;
}

void IMU::getMagnetometerValues(int16_t* magnetometerValues)
{
    magnetometerValues[AXIS_INDEX_X] = m_magnetometerValues.x;
    magnetometerValues[AXIS_INDEX_Y] = m_magnetometerValues.y;
    magnetometerValues[AXIS_INDEX_Z] = m_magnetometerValues.z;
}

// TD076	Make sure that it is necessary to read and calibrate all sensor values, if only 1 of the gyro values and 2 of the acceleration values are being used.
void IMU::calibrate()
{    
    /* Define how many measurements should be made for calibration. */
    const uint8_t NUMBER_OF_MEASUREMENTS      = 10; 

    /* Calibration takes place while the robot doesn't move. Therefore the Acceleration and turn values are near 0 and int16_t are enough. */
    int16_t sumOfRawAccelValuesX      = 0;
    int16_t sumOfRawAccelValuesY      = 0;
    int16_t sumOfRawAccelValuesZ      = 0;
    
    int16_t sumOfRawGyroValuesX       = 0;
    int16_t sumOfRawGyroValuesY       = 0;
    int16_t sumOfRawGyroValuesZ       = 0;

    for (uint8_t measurementIdx=0; measurementIdx<NUMBER_OF_MEASUREMENTS; ++measurementIdx)
    {
        while(!m_imu.accDataReady()){}
        m_imu.readAcc();
        sumOfRawAccelValuesX += m_imu.a.x;
        sumOfRawAccelValuesY += m_imu.a.y;
        sumOfRawAccelValuesZ += m_imu.a.z;
        
        while(!m_imu.gyroDataReady()){}
        m_imu.readGyro();
        sumOfRawGyroValuesX += m_imu.g.x;
        sumOfRawGyroValuesY += m_imu.g.y;
        sumOfRawGyroValuesZ += m_imu.g.z;
    }

    m_rawAccelerometerOffsetX = sumOfRawGyroValuesX / NUMBER_OF_MEASUREMENTS;
    m_rawAccelerometerOffsetY = sumOfRawAccelValuesY / NUMBER_OF_MEASUREMENTS;
    m_rawAccelerometerOffsetZ = sumOfRawAccelValuesZ / NUMBER_OF_MEASUREMENTS;

    m_rawGyroOffsetX = sumOfRawGyroValuesX / NUMBER_OF_MEASUREMENTS;
    m_rawGyroOffsetY = sumOfRawGyroValuesY / NUMBER_OF_MEASUREMENTS;
    m_rawGyroOffsetZ = sumOfRawGyroValuesZ / NUMBER_OF_MEASUREMENTS;
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
