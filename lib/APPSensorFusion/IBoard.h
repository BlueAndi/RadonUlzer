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
 * @brief  Board interface, which abstracts the physical board
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALInterfaces
 *
 * @{
 */

#ifndef IBOARD_H
#define IBOARD_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IButton.h>
#include <IBuzzer.h>
#include <IEncoders.h>
#include <ILineSensors.h>
#include <IMotors.h>
#include <ILed.h>
#include <IMU.h>
#include <ISettings.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstracts the physical board interface.
 */
class IBoard
{
public:
    /**
     * Destroys the board interface.
     */
    virtual ~IBoard()
    {
    }

    /**
     * Initialize the hardware.
     */
    virtual void init() = 0;

    /**
     * Get button A driver.
     *
     * @return Button A driver.
     */
    virtual IButton& getButtonA() = 0;

    /**
     * Get buzzer driver.
     *
     * @return Buzzer driver.
     */
    virtual IBuzzer& getBuzzer() = 0;

    /**
     * Get encoders driver.
     *
     * @return Encoders driver.
     */
    virtual IEncoders& getEncoders() = 0;

    /**
     * Get line sensors driver.
     *
     * @return Line sensor driver.
     */
    virtual ILineSensors& getLineSensors() = 0;

    /**
     * Get motor driver.
     *
     * @return Motor driver.
     */
    virtual IMotors& getMotors() = 0;

    /**
     * Get yellow LED driver.
     *
     * @return Yellow LED driver.
     */
    virtual ILed& getYellowLed() = 0;

    /**
     * Get IMU (Inertial Measurement Unit) driver.
     *
     * @return IMU driver
     */
    virtual IIMU& getIMU() = 0;

    /**
     * Get settings instance.
     *
     * @return Settings
     */
    virtual ISettings& getSettings() = 0;

    /**
     * Process actuators and sensors.
     */
    virtual void process() = 0;

protected:
    /**
     * Constructs the board interface.
     */
    IBoard()
    {
    }

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IBOARD_H */
/** @} */
