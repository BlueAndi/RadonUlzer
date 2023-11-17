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
 * @brief  IMU (Inertial Measurement Unit) realization for RemoteControl Application
 * @author Juliane Kerpe <juliane.kerpe@web.de>
 *
 * @addtogroup HALTarget
 *
 * @{
 */

#ifndef REMOTECONTROLIMU_H
#define REMOTECONTROLIMU_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IIMU.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The IMU adapter as a dummy driver for the IMU of the RemoteControl Application.
 * IMU stands for Inertial Measurement Unit.
 */
class RemoteControlIMU : public IIMU
{
public:
    /**
     * Destroys the adapter.
     */
    ~RemoteControlIMU()
    {
    }

    /**
     * Initializes the inertial sensors and detects their type.
     *
     * @return True if the sensor type was detected succesfully; false otherwise.
     */
    bool init()
    {
        return true;
    }

    /**
     * Enables all of the inertial sensors with a default configuration.
     */
    void enableDefault(){};

    /**
     * 	Configures the sensors with settings optimized for turn sensing.
     */
    void configureForTurnSensing()
    {
    }

    /**
     * Takes a reading from the accelerometer and makes the measurements available in a.
     */
    void readAccelerometer()
    {
    }

    /**
     * Takes a reading from the gyro and makes the measurements available in g.
     */
    void readGyro()
    {
    }

    /**
     * Takes a reading from the magnetometer and makes the measurements available in m.
     */
    void readMagnetometer()
    {
    }

    /**
     * Indicates whether the accelerometer has new measurement data ready.
     *
     * @return True if there is new accelerometer data available; false otherwise.
     */
    bool accelerometerDataReady()
    {
        return true;
    }

    /**
     * Indicates whether the gyro  has new measurement data ready.
     *
     * @return True if there is new gyro data available; false otherwise.
     */
    bool gyroDataReady()
    {
        return true;
    }

    /**
     * Indicates whether the magnetometer  has new measurement data ready.
     *
     * @return True if there is new magnetometer data available; false otherwise.
     */
    bool magnetometerDataReady()
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
    const void getAccelerationValues(IMUData* accelerationValues)
    {
    }

    /**
     * Get last raw Gyro values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] turnRates  Pointer to IMUData struct where the raw turn Rates in digits in x, y and z
     * direction will be written into. The values can be converted into physical values in mrad/s via the multiplication
     * with a sensitivity factor in mrad/s/digit.
     */
    const void getTurnRates(IMUData* turnRates)
    {
    }

    /**
     * Get last raw Magnetometer values as an IMUData struct containing values in x, y and z in digits.
     *
     * @param[in] magnetometerValues  Pointer to IMUData struct where the raw magnetometer values in digits in
     * x, y and z direction will be written into. The values can be converted into physical values in mgauss via the
     * multiplication with a sensitivity factor in mgauss/digit.
     */
    const void getMagnetometerValues(IMUData* magnetometerValues)
    {
    }

    /**
     * Calibrate the IMU.
     */
    virtual void calibrate()
    {
    }

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* REMOTECONTROLIMU_H */
/** @} */
