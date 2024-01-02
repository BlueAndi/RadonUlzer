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
 * @brief  Motrs realization
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Motors.h"

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

void Motors::init()
{
    initMotor(m_leftMotor);
    initMotor(m_rightMotor);
}

void Motors::setSpeeds(int16_t leftSpeed, int16_t rightSpeed)
{
    const double _MAX_SPEED = static_cast<double>(MAX_SPEED);

    if (nullptr != m_leftMotor)
    {
        const double MAX_SIM_VELOCITY = m_leftMotor->getMaxVelocity();

        if (leftSpeed > MAX_SPEED)
        {
            m_leftMotor->setVelocity(MAX_SIM_VELOCITY);
        }
        else if (leftSpeed < (-MAX_SPEED))
        {
            m_leftMotor->setVelocity(-MAX_SIM_VELOCITY);
        }
        else
        {
            double _leftSpeed = static_cast<double>(leftSpeed);

            m_leftMotor->setVelocity((MAX_SIM_VELOCITY * _leftSpeed) / _MAX_SPEED);
        }
    }

    if (nullptr != m_rightMotor)
    {
        const double MAX_SIM_VELOCITY = m_rightMotor->getMaxVelocity();

        if (rightSpeed > MAX_SPEED)
        {
            m_rightMotor->setVelocity(MAX_SIM_VELOCITY);
        }
        else if (rightSpeed < (-MAX_SPEED))
        {
            m_rightMotor->setVelocity(-MAX_SIM_VELOCITY);
        }
        else
        {
            double _rightSpeed = static_cast<double>(rightSpeed);

            m_rightMotor->setVelocity((MAX_SIM_VELOCITY * _rightSpeed) / _MAX_SPEED);
        }
    }

    m_leftSpeed  = leftSpeed;
    m_rightSpeed = rightSpeed;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Motors::initMotor(webots::Motor* motor)
{
    if (nullptr != motor)
    {
        /* Endless linear motion */
        motor->setPosition(std::numeric_limits<double>::infinity());

        /* Stop motor, note velocity in m/s */
        motor->setVelocity(0.0F);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
