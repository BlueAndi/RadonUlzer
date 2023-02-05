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
 * @brief  Calibration state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CalibrationState.h"
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

void CalibrationState::entry()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Calib");

    /* Prepare calibration drive. */
    m_steps = 0;

    /* Wait some time, before starting the calibration drive. */
    m_timer.start(WAIT_TIME);
}

void CalibrationState::process(StateMachine& sm)
{
    /* Wait some time so the user gets its fingers off the robot and then begin
     * automatic sensor calibration by rotating in place to sweep the sensors
     * over the line.
     */
    if (true == m_timer.isTimeout())
    {
        IMotors&      motors          = Board::getInstance().getMotors();
        ILineSensors& lineSensors     = Board::getInstance().getLineSensors();
        const int16_t CALIB_SPEED     = motors.getMaxSpeed() / 2;
        const uint16_t CALIB_LR_STEPS = 600;
        const uint16_t CALIB_STEPS_MAX = 4 * CALIB_LR_STEPS;

        /*
         * The first 30 steps the robot rotates right, the next
         * 60 steps left and then 30 steps right again.
         */
        if (CALIB_STEPS_MAX > m_steps)
        {
            if ((CALIB_LR_STEPS < m_steps) && ((CALIB_STEPS_MAX - CALIB_LR_STEPS) >= m_steps))
            {
                motors.setSpeeds(-CALIB_SPEED, CALIB_SPEED);
            }
            else
            {
                motors.setSpeeds(CALIB_SPEED, -CALIB_SPEED);
            }

            lineSensors.calibrate();

            m_steps += abs(m_encoder.getCountsAndResetLeft());
        }
        else
        {
            motors.setSpeeds(0, 0);

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
    }
}

void CalibrationState::exit()
{
    m_timer.stop();
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
