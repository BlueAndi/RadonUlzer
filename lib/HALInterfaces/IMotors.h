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
 * @brief  Abstract motors interface
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup HALInterfaces
 *
 * @{
 */
#ifndef IMOTORS_H
#define IMOTORS_H

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

/** The abstract motors interface. */
class IMotors
{
public:
    /**
     * Destroys the interface.
     */
    virtual ~IMotors()
    {
    }

    /**
     * Sets the speeds for both motors.
     *
     * @param[in] leftSpeed A number from -400 to 400 representing the speed and
     * direction of the right motor. Values of -400 or less result in full speed
     * reverse, and values of 400 or more result in full speed forward.
     * @param[in] rightSpeed A number from -400 to 400 representing the speed and
     * direction of the right motor. Values of -400 or less result in full speed
     * reverse, and values of 400 or more result in full speed forward.
     */
    virtual void setSpeeds(int16_t leftSpeed, int16_t rightSpeed) = 0;

    /**
     * Get maximum speed of the motors in digits.
     *
     * @return Max. speed in digits
     */
    virtual int16_t getMaxSpeed() const = 0;

    /**
     * Get the current speed of the left motor.
     * 
     * @return The speed of the left motor in digits.
     */
    virtual int16_t getLeftSpeed() = 0;

    /** 
     * Get the current speed of the right motor.
     * 
     * @return The speed of the right motor in digits.
     */
    virtual int16_t getRightSpeed() = 0;

protected:
    /**
     * Constructs the interface.
     */
    IMotors()
    {
    }

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMOTORS_H */
/** @} */
