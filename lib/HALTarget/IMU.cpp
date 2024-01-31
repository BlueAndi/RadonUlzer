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
 * @brief  IMU implementation
 * @author Juliane Kerpe <juliane.kerpe@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IMU.h"
#include <Arduino.h>
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
    return m_imuDrv.init();
}

void IMU::enableDefault()
{
    m_imuDrv.enableDefault();
}

void IMU::configureForTurnSensing()
{
    m_imuDrv.configureForTurnSensing();
    /* Set a different Full Scale Factor. The Original Function uses +/- 2000 dps which is far too high for the our
     * purposes. The values are defined in the Data Sheets of the gyros L3GD20H and LSM6DS33. */
    switch (m_imuDrv.getType())
    {
    case Zumo32U4IMUType::LSM303D_L3GD20H:
        /* Set the Full Scale factor of the L3GD20H to +/- 500 dps (0x10 = 0b00010000) */
        m_imuDrv.writeReg(L3GD20H_ADDR, L3GD20H_REG_CTRL4, 0x10);
        break;
    case Zumo32U4IMUType::LSM6DS33_LIS3MDL:
        /* Set the Full Scale factor of the LSM6DS33  to +/- 500 dps (0x7C = 0b01110100). Also set the Output Data Rate 
         * to 833 Hz (high performance).  */
        m_imuDrv.writeReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL2_G, 0x74);
        break;
    default:
        return;
    }
}

void IMU::readAccelerometer()
{
    m_imuDrv.readAcc();
    m_accelerometerValues.valueX = m_imuDrv.a.x - m_rawAccelerometerOffsetX;
    m_accelerometerValues.valueY = m_imuDrv.a.y - m_rawAccelerometerOffsetY;
    m_accelerometerValues.valueZ = m_imuDrv.a.z;
}

void IMU::readGyro()
{
    m_imuDrv.readGyro();
    m_gyroValues.valueX = m_imuDrv.g.x;
    m_gyroValues.valueY = m_imuDrv.g.y;
    m_gyroValues.valueZ = m_imuDrv.g.z - m_rawGyroOffsetZ;
}

void IMU::readMagnetometer()
{
    m_imuDrv.readMag();
    m_magnetometerValues.valueX = m_imuDrv.m.x;
    m_magnetometerValues.valueY = m_imuDrv.m.y;
    m_magnetometerValues.valueZ = m_imuDrv.m.z;
}

bool IMU::accelerometerDataReady()
{
    return m_imuDrv.accDataReady();
}

bool IMU::gyroDataReady()
{
    return m_imuDrv.gyroDataReady();
}

bool IMU::magnetometerDataReady()
{
    return m_imuDrv.magDataReady();
}

const void IMU::getAccelerationValues(IMUData* accelerationValues)
{
    if (nullptr != accelerationValues)
    {
        accelerationValues->valueX = m_accelerometerValues.valueX;
        accelerationValues->valueY = m_accelerometerValues.valueY;
        accelerationValues->valueZ = m_accelerometerValues.valueZ;
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

/* TODO:  TD076	Make sure that it is necessary to read and calibrate all sensor values, if only 1 of the gyro values and
 2 of the acceleration values are being used.*/
void IMU::calibrate()
{
    /* Define how many measurements should be made for calibration. */
    const int32_t NUMBER_OF_MEASUREMENTS = 50U;

    /* Calibration takes place while the robot doesn't move. Therefore the Acceleration and turn values are near 0 and
     * int16_t values are enough. */
    int32_t sumOfRawAccelValuesX = 0;
    int32_t sumOfRawAccelValuesY = 0;

    int32_t sumOfRawGyroValuesZ = 0;

    int32_t measurementIndex = 0;
    while (measurementIndex < NUMBER_OF_MEASUREMENTS)
    {
        if (true == m_imuDrv.accDataReady() && true == m_imuDrv.gyroDataReady())
        {
            m_imuDrv.readAcc();
            m_imuDrv.readGyro();
            sumOfRawGyroValuesZ += m_imuDrv.g.z;
            sumOfRawAccelValuesX += m_imuDrv.a.x;
            sumOfRawAccelValuesY += m_imuDrv.a.y;
            ++measurementIndex;
        }
        else
        {
            delay(20U);
        }
    }

    m_rawAccelerometerOffsetX = static_cast<int16_t>(sumOfRawAccelValuesX / NUMBER_OF_MEASUREMENTS);
    m_rawAccelerometerOffsetY = static_cast<int16_t>(sumOfRawAccelValuesY / NUMBER_OF_MEASUREMENTS);

    m_rawGyroOffsetZ = static_cast<int16_t>(sumOfRawGyroValuesZ / NUMBER_OF_MEASUREMENTS);
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
