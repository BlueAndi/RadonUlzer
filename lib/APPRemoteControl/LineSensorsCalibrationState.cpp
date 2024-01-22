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
 * @brief  Line sensors calibration state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "LineSensorsCalibrationState.h"
#include <Board.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
#include <StateMachine.h>
#include <Util.h>
#include "RemoteCtrlState.h"
#include "ErrorState.h"

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

void LineSensorsCalibrationState::entry()
{
    IDisplay&          display   = Board::getInstance().getDisplay();
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();
    Odometry&          odometry  = Odometry::getInstance();

    display.clear();
    display.print("Calib");
    display.gotoXY(0, 1);
    display.print("LineS");

    /* Prepare calibration drive. */
    m_calibrationSpeed = diffDrive.getMaxMotorSpeed() / 3;
    m_orientation      = odometry.getOrientation();

    /* Wait some time, before starting the calibration drive. */
    m_phase = PHASE_1_WAIT;
    m_timer.start(WAIT_TIME);
}

void LineSensorsCalibrationState::process(StateMachine& sm)
{
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    switch (m_phase)
    {
    case PHASE_1_WAIT:
        if (true == m_timer.isTimeout())
        {
            m_phase = PHASE_2_TURN_LEFT;
            diffDrive.setLinearSpeed(-m_calibrationSpeed, m_calibrationSpeed);
        }
        break;

    case PHASE_2_TURN_LEFT:
        if (true == turnAndCalibrate(CALIB_ANGLE, true))
        {
            m_phase = PHASE_3_TURN_RIGHT;
            diffDrive.setLinearSpeed(m_calibrationSpeed, -m_calibrationSpeed);
        }
        break;

    case PHASE_3_TURN_RIGHT:
        if (true == turnAndCalibrate(-CALIB_ANGLE, false))
        {
            m_phase = PHASE_4_TURN_ORIG;
            diffDrive.setLinearSpeed(-m_calibrationSpeed, m_calibrationSpeed);
        }
        break;

    case PHASE_4_TURN_ORIG:
        if (true == turnAndCalibrate(0, true))
        {
            m_phase = PHASE_5_FINISHED;
            diffDrive.setLinearSpeed(0, 0);
            finishCalibration(sm);
        }
        break;

    case PHASE_5_FINISHED:
        /* fallthrough */
    default:
        break;
    }
}

void LineSensorsCalibrationState::exit()
{
    m_timer.stop();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool LineSensorsCalibrationState::turnAndCalibrate(int32_t calibAlpha, bool isGreaterEqual)
{
    ILineSensors& lineSensors = Board::getInstance().getLineSensors();
    Odometry&     odometry    = Odometry::getInstance();
    int32_t       alpha       = odometry.getOrientation() - m_orientation; /* [mrad] */
    bool          isSuccesful = false;

    /* Continously calibrate the line sensors. */
    lineSensors.calibrate();

    /* Is the goal that the current angle shall be lower or equal than the destination calibration angle? */
    if (false == isGreaterEqual)
    {
        /* Is alpha lower or equal than the destination calibration angle? */
        if (calibAlpha >= alpha)
        {
            isSuccesful = true;
        }
    }
    else
    {
        /* Is alpha greater or equal than the destination calibration angle? */
        if (calibAlpha <= alpha)
        {
            isSuccesful = true;
        }
    }

    return isSuccesful;
}

void LineSensorsCalibrationState::finishCalibration(StateMachine& sm)
{
    ILineSensors& lineSensors = Board::getInstance().getLineSensors();

    if (false == lineSensors.isCalibrationSuccessful())
    {
        char str[10];
        char valueStr[10];

        Util::uintToStr(valueStr, sizeof(valueStr), lineSensors.getCalibErrorInfo());

        strncpy(str, "Cal=", sizeof(str) - 1);
        str[sizeof(str) - 1] = '\0';

        strncat(str, valueStr, sizeof(str) - strlen(valueStr) - 1);

        ErrorState::getInstance().setErrorMsg(str);
        sm.setState(&ErrorState::getInstance());
    }
    else
    {
        sm.setState(&RemoteCtrlState::getInstance());
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
