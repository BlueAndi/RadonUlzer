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
 * @brief  IMU realization
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

class IMU : public IIMU
{
public:
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
    bool init(){return true;}
        
    /**
     * Enables all of the inertial sensors with a default configuration.
     */
    void enableDefault() {};

    /**
     * 	Configures the sensors with settings optimized for turn sensing.
     */
    void configureForTurnSensing() {}

    /**
     * Takes a reading from the accelerometer and makes the measurements available in a.
     */ 
    void readAcc() {}
        
    /**
     * Takes a reading from the gyro and makes the measurements available in g.
     */
    void readGyro() {}
        
    /**
     * Takes a reading from the magnetometer and makes the measurements available in m.
     */
    void readMag() {}
        
    /**
     * Indicates whether the accelerometer has new measurement data ready.
     * 
     * @return True if there is new accelerometer data available; false otherwise. 
     */
    bool accDataReady() {return true;}
        
    /**
     * Indicates whether the gyro  has new measurement data ready.
     * 
     * @return True if there is new gyro data available; false otherwise.
     */
    bool gyroDataReady() {return true;}
        
    /**
     * Indicates whether the magnetometer  has new measurement data ready.
     * 
     * @return True if there is new magnetometer data available; false otherwise.
     */
    bool magDataReady() {return true;}
    
    /**
     * Get last raw Accelerometer values.
     *
     * @param[in] accelerationValues  Pointer to array where the Accelerometer values shall be written into.
     */
    void getAccelerationValues(int16_t* accelerationValues) {}

    /**
     * Get last raw Gyroscope values.
     *
     * @param[in] turnRates  Pointer to array where the Gyroscope values shall be written into.
     */
    void getTurnRates(int16_t* turnRates) {}

    /**
     * Get last raw Magnetometer.W
     *
     * @param[in] magnetometerValues  Pointer to array where the Magnetometer values shall be written into.
     */
    void getMagnetometerValues(int16_t* magnetometerValues)  {}
    
    /**
     * Calibrate the IMU.
     */
    virtual void calibrate() {}
    
private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMU_H */
/** @} */
