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
 * @brief  Motors realization
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALTarget
 *
 * @{
 */

#ifndef MOTORS_H
#define MOTORS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IMotors.h"
#include "IMotorsTest.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the Zumo target motors. */
class Motors : public IMotors, public IMotorsTest
{
public:
    /**
     * Constructs the motors adapter.
     */
    Motors() : IMotors()
    {
    }

    /**
     * Destroys the motors adapter.
     */
    ~Motors()
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
    void setSpeeds(int16_t leftSpeed, int16_t rightSpeed) final
    {
        m_speedLeft  = leftSpeed;
        m_speedRight = rightSpeed;
    }

    /**
     * Get maximum speed of the motors in digits.
     *
     * @return Max. speed in digits
     */
    int16_t getMaxSpeed() const final
    {
        return MAX_SPEED;
    }

    /**
     * Get the current speed of the left motor.
     *
     * @return The left motor speed in digits.
     */
    int16_t getLeftSpeed() final
    {
        return m_speedLeft;
    }

    /**
     * Get the current speed of the right motor.
     *
     * @return The right motor speed in digits.
     */
    int16_t getRightSpeed() final
    {
        return m_speedRight;
    }

    /* ---------- Test Interface ---------- */

    /**
     * Set speed of the left motor.
     *
     * @param[in] speed Speed in digits.
     */
    void setLeftSpeed(int16_t speed) final
    {
        m_speedLeft = speed;
    }

    /**
     * Set speed of the right motor.
     *
     * @param[in] speed Speed in digits.
     */
    void setRightSpeed(int16_t speed) final
    {
        m_speedRight = speed;
    }

private:
    /**
     * The maximum speed of a single motor in PWM digits.
     */
    static const int16_t MAX_SPEED = 400;

    int16_t m_speedLeft;  /**< Left motor speed in digits. */
    int16_t m_speedRight; /**< Right motor speed in digits. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MOTORS_H */
/** @} */
