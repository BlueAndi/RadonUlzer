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
    bool isInitSuccessful = m_imuDrv.init();
    if (true == isInitSuccessful)
    {
        m_imuDrv.enableDefault();
        /* TODO: TD074  Make sure that a Full Scale of 245 dps of the gyros are enough */
        /* TODO: TD075	Make sure that it is valid to make the Full Scale setting in IMU::init() */
        switch (m_imuDrv.getType())
        {
        case Zumo32U4IMUType::LSM303D_L3GD20H:
            /* 0x00  means +/- 245 dps according to L3GD20H data sheet */
            m_imuDrv.writeReg(L3GD20H_ADDR, L3GD20H_REG_CTRL4, 0x00);
            break;
        case Zumo32U4IMUType::LSM6DS33_LIS3MDL:
            /* 0x00  means +/- 245 dps according to LSM6DS33 data sheet */
            m_imuDrv.writeReg(LSM6DS33_ADDR, LSM6DS33_REG_CTRL2_G, 0x00);
            break;
        default:
            isInitSuccessful = false;
            break;
        }
    }

    return isInitSuccessful;
}

void IMU::enableDefault()
{
    m_imuDrv.enableDefault();
}

void IMU::configureForTurnSensing()
{
    m_imuDrv.configureForTurnSensing();
}

void IMU::readAccelerometer()
{
    m_imuDrv.readAcc();
    m_accelerometerValues.valueX = m_imuDrv.a.x;
    m_accelerometerValues.valueY = m_imuDrv.a.y;
    m_accelerometerValues.valueZ = m_imuDrv.a.z;
}

void IMU::readGyro()
{
    m_imuDrv.readGyro();
    m_gyroValues.valueX = m_imuDrv.g.x;
    m_gyroValues.valueY = m_imuDrv.g.y;
    m_gyroValues.valueZ = m_imuDrv.g.z;
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
        accelerationValues->valueX = m_accelerometerValues.valueX - m_rawAccelerometerOffsetX;
        accelerationValues->valueY = m_accelerometerValues.valueY - m_rawAccelerometerOffsetY;
        accelerationValues->valueZ = m_accelerometerValues.valueZ - m_rawAccelerometerOffsetZ;
    }
}

const void IMU::getTurnRates(IMUData* turnRates)
{
    if (nullptr != turnRates)
    {
        turnRates->valueX = m_gyroValues.valueX - m_rawGyroOffsetX;
        turnRates->valueY = m_gyroValues.valueY - m_rawGyroOffsetY;
        turnRates->valueZ = m_gyroValues.valueZ - m_rawGyroOffsetZ;
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
    const uint8_t NUMBER_OF_MEASUREMENTS = 10;

    /* Calibration takes place while the robot doesn't move. Therefore the Acceleration and turn values are near 0 and
     * int16_t values are enough. */
    int16_t sumOfRawAccelValuesX = 0;
    int16_t sumOfRawAccelValuesY = 0;
    int16_t sumOfRawAccelValuesZ = 0;

    int16_t sumOfRawGyroValuesX = 0;
    int16_t sumOfRawGyroValuesY = 0;
    int16_t sumOfRawGyroValuesZ = 0;

    for (uint8_t measurementIdx = 0; measurementIdx < NUMBER_OF_MEASUREMENTS; ++measurementIdx)
    {
        while (!m_imuDrv.accDataReady())
        {
            /* Do nothing and wait until new Accelerometer Data is available. */
        }
        m_imuDrv.readAcc();
        sumOfRawAccelValuesX += m_imuDrv.a.x;
        sumOfRawAccelValuesY += m_imuDrv.a.y;
        sumOfRawAccelValuesZ += m_imuDrv.a.z;

        while (!m_imuDrv.gyroDataReady())
        {
            /* Do nothing and wait until new Gyro Data is available. */
        }
        m_imuDrv.readGyro();
        sumOfRawGyroValuesX += m_imuDrv.g.x;
        sumOfRawGyroValuesY += m_imuDrv.g.y;
        sumOfRawGyroValuesZ += m_imuDrv.g.z;
    }

    m_rawAccelerometerOffsetX = sumOfRawAccelValuesX / NUMBER_OF_MEASUREMENTS;
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
