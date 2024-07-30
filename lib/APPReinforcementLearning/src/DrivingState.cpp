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
 * @author Akram Bziouech
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DrivingState.h"
#include <StateMachine.h>
#include <DifferentialDrive.h>
#include <Board.h>
#include <Odometry.h>

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

    diffDrive.setLinearSpeed(0, 0);
    m_observationTimer.start(OBSERVATION_DURATION);
}

void DrivingState::process(StateMachine& sm)
{
    /* Nothing to do. */
    (void)sm;
}

void DrivingState::exit()
{
    m_observationTimer.stop();
}

void DrivingState::setTargetSpeeds(int16_t leftMotor, int16_t rightMotor)
{
    DifferentialDrive::getInstance().setLinearSpeed(leftMotor, rightMotor);
}

bool DrivingState::isAbortRequired()
{
    bool isAbort = false;

    /* If track is not finished over a certain time, abort driving. */
    if (true == m_observationTimer.isTimeout())
    {
        isAbort = true;
    }
    return isAbort;
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
