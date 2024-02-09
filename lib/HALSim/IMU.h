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
 * @brief  IMU (Inertial Measurement Unit) implementation of the simulation
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
#include <stdint.h>
#include "IIMU.h"

#include "SimTime.h"
#include <webots/Accelerometer.hpp>
#include <webots/Gyro.hpp>
#include <webots/Compass.hpp>

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
     * @param[in] accelerometer The accelerometer
     * @param[in] gyro          The gyro
     * @param[in] compass       The compass
     */
    IMU(webots::Accelerometer* accelerometer, webots::Gyro* gyro, webots::Compass* compass) :
        IIMU(),
        m_accelerationValues{0, 0, 0},
        m_gyroValues{0, 0, 0},
        m_magnetometerValues{0, 0, 0},
        m_accelerometer(accelerometer),
        m_gyro(gyro),
        m_magnetometer(compass)
    {
    }

    /**
     * Destroys the interface.
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
    void enableDefault() final
    {
    }

    /**
     * 	Configures the sensors with settings optimized for turn sensing.
     */
    void configureForTurnSensing() final
    {
    }

    /**
     * Takes a reading from the accelerometer and makes the measurements available in a.
     */
    void readAccelerometer() final;

    /**
     * Takes a reading from the gyro and makes the measurements available in g.
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
    bool accelerometerDataReady() final
    {
        return true;
    }

    /**
     * Indicates whether the gyro  has new measurement data ready.
     *
     * @return True if there is new gyro data available; false otherwise.
     */
    bool gyroDataReady() final
    {
        return true;
    }

    /**
     * Indicates whether the magnetometer  has new measurement data ready.
     *
     * @return True if there is new magnetometer data available; false otherwise.
     */
    bool magnetometerDataReady() final
    {
        return true;
    }

    /**
     * Get last raw Accelerometer values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] accelerationValues  Pointer to IMUData struct where the raw acceleration values in digits in
     * x, y and z direction will be written into. The values can be converted into physical values in mm/s^2 via the
     * multiplication with a sensitivity factor in mm/s^2/digit.
     */
    void const getAccelerationValues(IMUData* accelerationValues) final;

    /**
     * Get last raw Gyro values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] turnRates  Pointer to IMUData struct where the raw turn Rates in digits in x, y and z
     * direction will be written into. The values can be converted into physical values in mrad/s via the multiplication
     * with a sensitivity factor in mrad/s/digit.
     */
    void const getTurnRates(IMUData* turnRates) final;

    /**
     * Get last raw Magnetometer values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] magnetometerValues  Pointer to IMUData struct where the raw magnetometer values in digits in
     * x, y and z direction will be written into. The values can be converted into physical values in mgauss via the
     * multiplication with a sensitivity factor in mgauss/digit.
     */
    void const getMagnetometerValues(IMUData* magnetometerValues) final;

    /**
     * Calibrate the IMU.
     */
    void calibrate() final;

private:
    /**
     * Convert a parameter from double to int16_t. Clip the value to the minimum / maximum range of the int16_t data
     * type if the input value exceeds the limits of int16_t.
     *
     * @param[in]   originalValue  The Original double value which shall be converted.
     * @return  The converted value to int16_t, which might be clipped.
     */
    int16_t convertFromDoubleToInt16(double originalValue);

    IMUData m_accelerationValues; /**< Struct to store the current raw accelerometer data. */
    IMUData m_gyroValues;         /**< Struct to store the current gyro data. */
    IMUData m_magnetometerValues; /**< Struct to store the current magnotometer data. */

    webots::Accelerometer* m_accelerometer; /**< The accelerometer of Webots. */
    webots::Gyro*          m_gyro;          /**< The gyro of Webots. */
    webots::Compass*       m_magnetometer;  /**< The magnetometer of Webots. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMU_H */
/** @} */
