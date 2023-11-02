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
 * @brief  Abstract IMU interface, depicts Zumo32U4IMU library
 * @author Juliane Kerpe <juliane.kerpe@web.de>
 * 
 * @addtogroup HALInterfaces
 *
 * @{
 */
#ifndef IIMU_H
#define IIMU_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Struct of the IMU data in x, y and z direction. */
typedef struct _IMUData
{
    int16_t valueX;
    int16_t valueY;
    int16_t valueZ;
} __attribute__((packed)) IMUData;

/** The abstract IMU interface. */
class IIMU
{
public:
    /**
     * Destroys the interface.
     */
    virtual ~IIMU()
    {
    }

    /**
     * Initializes the inertial sensors and detects their type.
     *
     * @return True if the sensor type was detected succesfully; false otherwise.
     */
    virtual bool init() = 0;
    
    /**
     * Enables all of the inertial sensors with a default configuration.
     */
    virtual void enableDefault() = 0;
    
    /**
     * 	Configures the sensors with settings optimized for turn sensing.
     */
    virtual void configureForTurnSensing() = 0;

    /**
     * Takes a reading from the accelerometer and makes the measurements available in a.
     */ 
    virtual void readAccelerometer() = 0;
        
    /**
     * Takes a reading from the gyro and makes the measurements available in g.
     */
    virtual void readGyro() = 0;
        
    /**
     * Takes a reading from the magnetometer and makes the measurements available in m.
     */
    virtual void readMagnetometer() = 0;
        
    /**
     * Indicates whether the accelerometer has new measurement data ready.
     * 
     * @return True if there is new accelerometer data available; false otherwise. 
     */
    virtual bool accelerometerDataReady() = 0;
        
    /**
     * Indicates whether the gyro  has new measurement data ready.
     * 
     * @return True if there is new gyro data available; false otherwise.
     */
    virtual bool gyroDataReady() = 0;
        
    /**
     * Indicates whether the magnetometer  has new measurement data ready.
     * 
     * @return True if there is new magnetometer data available; false otherwise.
     */
    virtual bool magnetometerDataReady() = 0;

    /**
     * Get last raw Accelerometer values as a IMUData struct containing values in x, y and z.
     *
     * @param[in] accelerationValues  Pointer to IMUData struct.
     */
    virtual void getAccelerationValues(IMUData* accelerationValues) = 0;

    /**
     * Get last raw Gyroscope values as a IMUData struct containing values in x, y and z.
     *
     * @param[in] turnRates  Pointer to IMUData struct.
     */
    virtual void getTurnRates(IMUData* turnRates) = 0;

    /**
     * Get last raw Magnetometer values as a IMUData struct containing values in x, y and z.
     *
     * @param[in] magnetometerValues  Pointer to IMUData struct.
     */
    virtual void getMagnetometerValues(IMUData* magnetometerValues) = 0;

    /**
     * Calibrate the IMU.
     */
    virtual void calibrate() = 0;

protected:
    /**
     * Constructs the interface.
     */
    IIMU()
    {
    }

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IIMU_H */
/** @} */