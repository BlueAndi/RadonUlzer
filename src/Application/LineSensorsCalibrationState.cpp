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
 * @brief  Line sensors calibration state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "LineSensorsCalibrationState.h"
#include <Board.h>
#include "StateMachine.h"
#include "ReadyState.h"
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
    IDisplay&  display  = Board::getInstance().getDisplay();
    IEncoders& encoders = Board::getInstance().getEncoders();
    IMotors&   motors   = Board::getInstance().getMotors();

    display.clear();
    display.print("Calib");
    display.gotoXY(0, 1);
    display.print("LineS");

    /* Prepare calibration drive. */
    m_relEnc.setSteps(encoders.getCountsLeft());
    m_steps            = 0;
    m_calibrationSpeed = motors.getMaxSpeed() / 3;

    /* Wait some time, before starting the calibration drive. */
    m_phase = PHASE_1_WAIT;
    m_timer.start(WAIT_TIME);
}

void LineSensorsCalibrationState::process(StateMachine& sm)
{
    switch (m_phase)
    {
    case PHASE_1_WAIT:
        phase1Wait();
        break;

    case PHASE_2_TURN_LEFT:
        phase2TurnLeft();
        break;

    case PHASE_3_TURN_RIGHT:
        phase3TurnRight();
        break;

    case PHASE_4_TURN_ORIG:
        phase4TurnOrigin();
        break;

    case PHASE_5_FINISHED:
        phase5Finished(sm);
        break;

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

void LineSensorsCalibrationState::phase1Wait()
{
    if (true == m_timer.isTimeout())
    {
        IEncoders& encoders = Board::getInstance().getEncoders();
        IMotors&   motors   = Board::getInstance().getMotors();

        m_relEnc.setSteps(encoders.getCountsLeft());

        /* Turn left */
        m_phase = PHASE_2_TURN_LEFT;
        motors.setSpeeds(-m_calibrationSpeed, m_calibrationSpeed);
    }
}

void LineSensorsCalibrationState::phase2TurnLeft()
{
    IEncoders&    encoders    = Board::getInstance().getEncoders();
    ILineSensors& lineSensors = Board::getInstance().getLineSensors();
    uint16_t      steps       = abs(m_relEnc.calculate(encoders.getCountsLeft()));

    lineSensors.calibrate();

    if (CALIB_LR_STEPS <= steps)
    {
        IMotors& motors = Board::getInstance().getMotors();

        /* Turn right */
        m_phase = PHASE_3_TURN_RIGHT;
        m_relEnc.setSteps(encoders.getCountsLeft()); /* Clear */
        motors.setSpeeds(m_calibrationSpeed, -m_calibrationSpeed);
    }
}

void LineSensorsCalibrationState::phase3TurnRight()
{
    IEncoders&    encoders    = Board::getInstance().getEncoders();
    ILineSensors& lineSensors = Board::getInstance().getLineSensors();
    uint16_t      steps       = abs(m_relEnc.calculate(encoders.getCountsLeft()));

    lineSensors.calibrate();

    if ((2 * CALIB_LR_STEPS) <= steps)
    {
        IMotors& motors = Board::getInstance().getMotors();

        /* Turn left */
        m_phase = PHASE_4_TURN_ORIG;
        m_relEnc.setSteps(encoders.getCountsLeft()); /* Clear */
        motors.setSpeeds(-m_calibrationSpeed, m_calibrationSpeed);
    }
}

void LineSensorsCalibrationState::phase4TurnOrigin()
{
    IEncoders&    encoders    = Board::getInstance().getEncoders();
    ILineSensors& lineSensors = Board::getInstance().getLineSensors();
    uint16_t      steps       = abs(m_relEnc.calculate(encoders.getCountsLeft()));

    lineSensors.calibrate();

    if (CALIB_LR_STEPS <= steps)
    {
        IMotors& motors = Board::getInstance().getMotors();

        /* Stop */
        m_phase = PHASE_5_FINISHED;
        motors.setSpeeds(0, 0);
    }
}

void LineSensorsCalibrationState::phase5Finished(StateMachine& sm)
{
    ILineSensors& lineSensors = Board::getInstance().getLineSensors();

    if (false == lineSensors.isCalibrationSuccessful())
    {
        char str[10];

        snprintf(str, sizeof(str), "Cal %u", lineSensors.getCalibErrorInfo());

        ErrorState::getInstance().setErrorMsg(str);
        sm.setState(&ErrorState::getInstance());
    }
    else
    {
        sm.setState(&ReadyState::getInstance());
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
