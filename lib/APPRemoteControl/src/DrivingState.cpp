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
 * @brief  Driving state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DrivingState.h"
#include <StateMachine.h>
#include <DifferentialDrive.h>

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

void DrivingState::entry()
{
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    m_isActive = true;
    diffDrive.setLinearSpeed(0, 0);
    diffDrive.enable();
}

void DrivingState::process(StateMachine& sm)
{
    /* Nothing to do. */
    (void)sm;
}

void DrivingState::exit()
{
    m_isActive = false;

    /* Stop motors. */
    DifferentialDrive::getInstance().setLinearSpeed(0, 0);
    DifferentialDrive::getInstance().disable();
}

void DrivingState::setMotorSpeeds(int16_t leftMotor, int16_t rightMotor)
{
    if (true == m_isActive)
    {
        DifferentialDrive::getInstance().setLinearSpeed(leftMotor, rightMotor);
    }
}

void DrivingState::setRobotSpeeds(int16_t linearSpeed, int16_t angularSpeed)
{
    if (true == m_isActive)
    {
        DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

        /* Linear speed is set first. Overwrites all speed setpoints. */
        diffDrive.setLinearSpeed(linearSpeed);

        /* Angular speed is set on-top of the linear speed. Must be called after setLinearSpeed(). */
        diffDrive.setAngularSpeed(angularSpeed);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
