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
 * @brief  IMU (Inertial Measurement Unit) implementation
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

/** The IMU adapter.
 * IMU stands for Inertial Measurement Unit.
 */
class IMU : public IIMU
{
public:
    /**
     * Constructs the IMU adapter.
     *
     */
    IMU() :
        IIMU(),
        m_accelerometerValues{0, 0, 0},
        m_gyroValues{0, 0, 0},
        m_magnetometerValues{0, 0, 0},
        m_rawAccelerometerOffsetX(0),
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
    bool init() final;

    /**
     * Enables all of the inertial sensors with a default configuration.
     */
    void enableDefault() final;

    /**
     * 	Configures the sensors with settings optimized for turn sensing.
     *
     *  Additionally, only the z-axis of the gyroscope and the x-axis of the accelerometer are enabled.
     */
    void configureForTurnSensing() final;

    /**
     * Takes a reading from the accelerometer and makes the measurements available in a.
     *
     * Only the Acceleration along the x-axis is read. The measurements along the other axes are disabled and set to 0.
     */
    void readAccelerometer() final;

    /**
     * Takes a reading from the gyro and makes the measurements available in g.
     *
     * Only the Turn Rate around the z-axis is read. The measurements along the other axes are disabled and set to f0.
     */
    void readGyro() final;

    /**
     * Takes a reading from the magnetometer and makes the measurements available in m.
     */
    void readMagnetometer() final;

    /**
     * Indicates whether the accelerometer has new measurement data ready.
     *
     * @return True if there is new accelerometer data available; false otherwise.
     */
    bool accelerometerDataReady() final;

    /**
     * Indicates whether the gyro  has new measurement data ready.
     *
     * @return True if there is new gyro data available; false otherwise.
     */
    bool gyroDataReady() final;

    /**
     * Indicates whether the magnetometer  has new measurement data ready.
     *
     * @return True if there is new magnetometer data available; false otherwise.
     */
    bool magnetometerDataReady() final;

    /**
     * Get last raw Accelerometer values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] accelerationValues  Pointer to IMUData struct where the raw acceleration values in digits in
     * x, y and z direction will be written into. The values can be converted into physical values in mm/s^2 via the
     * multiplication with a sensitivity factor in mm/s^2/digit.
     */
    const void getAccelerationValues(IMUData* accelerationValues) final;

    /**
     * Get last raw Gyro values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] turnRates  Pointer to IMUData struct where the raw turn Rates in digits in x, y and z
     * direction will be written into. The values can be converted into physical values in mrad/s via the multiplication
     * with a sensitivity factor in mrad/s/digit.
     */
    const void getTurnRates(IMUData* turnRates) final;

    /**
     * Get last raw Magnetometer values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] magnetometerValues  Pointer to IMUData struct where the raw magnetometer values in digits in
     * x, y and z direction will be written into. The values can be converted into physical values in mgauss via the
     * multiplication with a sensitivity factor in mgauss/digit.
     */
    const void getMagnetometerValues(IMUData* magnetometerValues) final;

    /**
     * Calibrate the IMU.
     * Only the used axes (acceleration along x and y and turn rate around z) are calibrated.
     */
    void calibrate() final;

protected:
private:
    IMUData m_accelerometerValues; /**< Raw accelerometer readings. */
    IMUData m_gyroValues;          /**< Raw gyro readings. */
    IMUData m_magnetometerValues;  /**< Raw magnetometer readings. */

    Zumo32U4IMU m_imuDrv; /**< IMU driver from Zumo32U4 Library. */

    int16_t m_rawAccelerometerOffsetX; /**< Mean raw accelerometer offset in x-direction determined by calibration. */
    int16_t m_rawGyroOffsetZ;          /**< Mean raw gyro offset in z-direction determined by calibration. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMU_H */
/** @} */
