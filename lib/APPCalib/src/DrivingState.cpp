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
#include <Board.h>
#include <DifferentialDrive.h>
#include <StateMachine.h>
#include <Odometry.h>
#include "ReadyState.h"

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
    Odometry&          odometry  = Odometry::getInstance();
    int32_t            angle     = odometry.getOrientation();
    int16_t            speed     = diffDrive.getMaxMotorSpeed() / 3;

    switch (m_cmd)
    {
    case DRIVING_CMD_FORWARD:
        odometry.clearMileage();
        diffDrive.setLinearSpeed(speed);
        break;

    case DRIVING_CMD_TURN_LEFT:
        m_dstAngle = angle + TURN_ANGLE;

        if (FP_2PI() <= m_dstAngle)
        {
            m_dstAngle -= FP_2PI();
            m_wrapAroundCnt = 1;
        }

        diffDrive.setLinearSpeed(-speed, speed);
        break;

    case DRIVING_CMD_TURN_RIGHT:
        m_dstAngle = angle - TURN_ANGLE;

        if (-FP_2PI() >= m_dstAngle)
        {
            m_dstAngle += FP_2PI();
            m_wrapAroundCnt = 1;
        }

        diffDrive.setLinearSpeed(speed, -speed);
        break;

    default:
        break;
    }
}

void DrivingState::process(StateMachine& sm)
{
    Odometry& odometry = Odometry::getInstance();

    switch (m_cmd)
    {
    case DRIVING_CMD_FORWARD:
        if (DRIVE_FORWARD_DISTANCE <= odometry.getMileageCenter())
        {
            sm.setState(&ReadyState::getInstance());
        }
        break;

    case DRIVING_CMD_TURN_LEFT:
        if (0 < m_wrapAroundCnt)
        {
            if (FP_PI() > odometry.getOrientation())
            {
                --m_wrapAroundCnt;
            }
        }

        if ((m_dstAngle <= odometry.getOrientation()) && (0 == m_wrapAroundCnt))
        {
            sm.setState(&ReadyState::getInstance());
        }
        break;

    case DRIVING_CMD_TURN_RIGHT:
        if (0 < m_wrapAroundCnt)
        {
            if (-FP_PI() < odometry.getOrientation())
            {
                --m_wrapAroundCnt;
            }
        }

        if ((m_dstAngle >= odometry.getOrientation()) && (0 == m_wrapAroundCnt))
        {
            sm.setState(&ReadyState::getInstance());
        }
        break;

    default:
        break;
    }
}

void DrivingState::exit()
{
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();
    diffDrive.setLinearSpeed(0, 0);
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
