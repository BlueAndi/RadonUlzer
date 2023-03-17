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
 * @brief  Motor speed calibration state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MotorSpeedCalibrationState.h"
#include <Board.h>
#include <RobotConstants.h>
#include <DifferentialDrive.h>
#include <StateMachine.h>
#include <Logging.h>
#include <Util.h>
#include "LineSensorsCalibrationState.h"
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

/**
 * Logging source.
 */
const char* TAG = "MotorSpeedCalibrationState";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MotorSpeedCalibrationState::entry()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Calib");
    display.gotoXY(0, 1);
    display.print("MSpeed");

    /* Setup relative encoders */
    m_relEncoders.clear();

    /* Wait some time, before starting the calibration drive. */
    m_phase = PHASE_1_WAIT;
    m_timer.start(WAIT_TIME);
}

void MotorSpeedCalibrationState::process(StateMachine& sm)
{
    switch (m_phase)
    {
    case PHASE_1_WAIT:
        phase1Wait();
        break;

    case PHASE_2_BACK:
        phase2Back();
        break;

    case PHASE_3_FORWARD:
        phase3Forward();
        break;

    case PHASE_4_FINISHED:
        phase4Finished(sm);
        break;

    default:
        break;
    }
}

void MotorSpeedCalibrationState::exit()
{
    /* Nothing to do. */
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MotorSpeedCalibrationState::phase1Wait()
{
    if (true == m_timer.isTimeout())
    {
        IMotors& motors = Board::getInstance().getMotors();

        /* Full back */
        m_phase = PHASE_2_BACK;
        m_timer.start(CALIB_DURATION);

        /* Control motors directly and not via differential drive control,
         * because the differential drive control needs first to be updated
         * regarding the max. possible motor speed in [steps/s] which is
         * determined by this calibration.
         */
        motors.setSpeeds(-motors.getMaxSpeed(), -motors.getMaxSpeed());
    }
}

void MotorSpeedCalibrationState::phase2Back()
{
    if (true == m_timer.isTimeout())
    {
        IMotors& motors     = Board::getInstance().getMotors();
        int32_t  stepsLeft  = 0;
        int32_t  stepsRight = 0;

        /* Stop motors immediately */
        motors.setSpeeds(0, 0);

        /* Determine max. speed backward. */
        stepsLeft  = abs(m_relEncoders.getCountsLeft());
        stepsRight = abs(m_relEncoders.getCountsRight());

        /* Convert number of steps to [steps/s] */
        stepsLeft *= 1000;
        stepsLeft /= CALIB_DURATION;
        stepsRight *= 1000;
        stepsRight /= CALIB_DURATION;

        if (INT16_MAX >= stepsLeft)
        {
            m_maxSpeedLeft  = static_cast<int16_t>(stepsLeft);
        }

        if (INT16_MAX >= stepsRight)
        {
            m_maxSpeedRight = static_cast<int16_t>(stepsRight);
        }

        /* Clear relative encoders */
        m_relEncoders.clear();

        /* Full forward */
        m_phase = PHASE_3_FORWARD;
        m_timer.start(CALIB_DURATION);
        motors.setSpeeds(motors.getMaxSpeed(), motors.getMaxSpeed());
    }
}

void MotorSpeedCalibrationState::phase3Forward()
{
    if (true == m_timer.isTimeout())
    {
        IMotors& motors     = Board::getInstance().getMotors();
        int32_t  stepsLeft  = 0;
        int32_t  stepsRight = 0;

        /* Stop motors immediately */
        motors.setSpeeds(0, 0);

        /* Determine max. speed forward. */
        stepsLeft  = abs(m_relEncoders.getCountsLeft());
        stepsRight = abs(m_relEncoders.getCountsRight());

        /* Convert number of steps to [steps/s] */
        stepsLeft *= 1000;
        stepsLeft /= CALIB_DURATION;
        stepsRight *= 1000;
        stepsRight /= CALIB_DURATION;

        /* Use lower speed to ensure that motor speed can be reached in both
         * directions.
         */
        if (stepsLeft < m_maxSpeedLeft)
        {
            m_maxSpeedLeft = stepsLeft;
        }

        /* Use lower speed to ensure that motor speed can be reached in both
         * directions.
         */
        if (stepsRight < m_maxSpeedRight)
        {
            m_maxSpeedRight = stepsRight;
        }

        /* Clear relative encoders */
        m_relEncoders.clear();

        /* Finished */
        m_phase = PHASE_4_FINISHED;
        m_timer.stop();
    }
}

void MotorSpeedCalibrationState::phase4Finished(StateMachine& sm)
{
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    /* Set the lower speed as max. motor speed to ensure that both motors
     * can reach the same max. speed.
     */
    int16_t maxSpeed = (m_maxSpeedLeft < m_maxSpeedRight) ? m_maxSpeedLeft : m_maxSpeedRight;

    /* With setting the max. motor speed in [steps/s] the differential drive control
     * can now be used.
     */
    diffDrive.setMaxMotorSpeed(maxSpeed);

    if (0 == maxSpeed)
    {
        ErrorState::getInstance().setErrorMsg("MS=0");
        sm.setState(&ErrorState::getInstance());
    }
    else
    {
        char valueStr[10];

        LOG_DEBUG_HEAD(TAG);
        LOG_DEBUG_MSG("Calibrated max. speed:");

        Util::intToStr(valueStr, sizeof(valueStr), maxSpeed);
        LOG_DEBUG_MSG(valueStr);

        LOG_DEBUG_MSG(" steps/s (= ");

        Util::intToStr(valueStr, sizeof(valueStr), maxSpeed / RobotConstants::ENCODER_STEPS_PER_MM);
        LOG_DEBUG_MSG(valueStr);

        LOG_DEBUG_MSG(" mm/s)");
        LOG_DEBUG_TAIL();

        sm.setState(&LineSensorsCalibrationState::getInstance());
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
