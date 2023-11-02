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
 *
 * @addtogroup HALSim
 *
 * @{
 */
#ifndef IMU_H
#define IMU_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IIMU.h"
#include "Zumo32U4IMU.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The IMU adapter. */
class IMU : public IIMU
{
public:
    /**
     * Constructs the IMU adapter.
     *
     */
    IMU() :
        IIMU(),
        m_rawAccelerometerOffsetX(0),
        m_rawAccelerometerOffsetY(0),
        m_rawAccelerometerOffsetZ(0),
        m_rawGyroOffsetX(0),
        m_rawGyroOffsetY(0),
        m_rawGyroOffsetZ(0)
    {
    }

    /**
     * Destroys the IMU adapter.
     */
    ~IMU()
    {
    }

    /**
     * Initializes the inertial sensors and detects their type.
     *
     * @return True if the sensor type was detected succesfully; false otherwise.
     */
    bool init();

    /**
     * Enables all of the inertial sensors with a default configuration.
     */
    void enableDefault();

    /**
     * 	Configures the sensors with settings optimized for turn sensing.
     */
    void configureForTurnSensing();

    /**
     * Takes a reading from the accelerometer and makes the measurements available in a.
     */
    void readAccelerometer();

    /**
     * Takes a reading from the gyro and makes the measurements available in g.
     */
    void readGyro();

    /**
     * Takes a reading from the magnetometer and makes the measurements available in m.
     */
    void readMagnetometer();

    /**
     * Indicates whether the accelerometer has new measurement data ready.
     *
     * @return True if there is new accelerometer data available; false otherwise.
     */
    bool accelerometerDataReady();

    /**
     * Indicates whether the gyro  has new measurement data ready.
     *
     * @return True if there is new gyro data available; false otherwise.
     */
    bool gyroDataReady();

    /**
     * Indicates whether the magnetometer  has new measurement data ready.
     *
     * @return True if there is new magnetometer data available; false otherwise.
     */
    bool magnetometerDataReady();

    /**
     * Get last raw Accelerometer values as a IMUData struct containing values in x, y and z.
     *
     * @param[in] accelerationValues  Pointer to IMUData struct.
     */
    void getAccelerationValues(IMUData* accelerationValues);

    /**
     * Get last raw Gyroscope values as a IMUData struct containing values in x, y and z.
     *
     * @param[in] turnRates  Pointer to IMUData struct.
     */
    void getTurnRates(IMUData* turnRates);

    /**
     * Get last raw Magnetometer values as a IMUData struct containing values in x, y and z.
     *
     * @param[in] magnetometerValues  Pointer to IMUData struct.
     */
    void getMagnetometerValues(IMUData* magnetometerValues);

    /**
     * Calibrate the IMU.
     */
    void calibrate();

private:
    IMUData m_accelerometerValues = {0, 0, 0}; /**< Raw accelerometer readings. */
    IMUData m_gyroValues          = {0, 0, 0}; /**< Raw gyro readings. */
    IMUData m_magnetometerValues  = {0, 0, 0}; /**< Raw magnetometer readings. */

protected:
private:
    Zumo32U4IMU m_imu;

    /**
     * Bias values of the sensors determined by the calibration.
     */
    int16_t m_rawAccelerometerOffsetX;
    int16_t m_rawAccelerometerOffsetY;
    int16_t m_rawAccelerometerOffsetZ;
    int16_t m_rawGyroOffsetX;
    int16_t m_rawGyroOffsetY;
    int16_t m_rawGyroOffsetZ;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMU_H */
/** @} */