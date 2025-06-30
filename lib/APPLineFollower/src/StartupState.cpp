/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Startup state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "StartupState.h"
#include <Board.h>
#include <StateMachine.h>
#include "MotorSpeedCalibrationState.h"
#include "LineSensorsCalibrationState.h"
#include "Sound.h"
#include <DifferentialDrive.h>
#include <Util.h>

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

void StartupState::entry()
{
    Board&     board         = Board::getInstance();
    ISettings& settings      = board.getSettings();
    int16_t    maxMotorSpeed = settings.getMaxSpeed();

    /* If max. motor speed calibration value is available, the differential
     * drive will be enabled.
     */
    if (0 < maxMotorSpeed)
    {
        DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

        /* With setting the max. motor speed in [steps/s] the differential drive control
         * can now be used.
         */
        diffDrive.setMaxMotorSpeed(maxMotorSpeed);

        /* Differential drive can now be used. */
        diffDrive.enable();

        m_isMaxMotorSpeedCalibAvailable = true;
    }

    showUserInfo(m_userInfoState);
}

void StartupState::process(StateMachine& sm)
{
    Board&   board   = Board::getInstance();
    IButton& buttonA = board.getButtonA();

    /* Start motor calibration? */
    if (true == Util::isButtonTriggered(buttonA, m_isButtonAPressed))
    {
        sm.setState(&MotorSpeedCalibrationState::getInstance());
    }

    /* If the max. motor speed calibration is done, it will be possible to
     * start the line sensor calibration immediately.
     */
    if (true == m_isMaxMotorSpeedCalibAvailable)
    {
        IButton& buttonB = board.getButtonB();

        /* Start line sensor calibration? */
        if (true == Util::isButtonTriggered(buttonB, m_isButtonBPressed))
        {
            sm.setState(&LineSensorsCalibrationState::getInstance());
        }
    }

    /* Periodically change the user info on the display. */
    if (true == m_timer.isTimeout())
    {
        int8_t next = m_userInfoState + 1;

        if (USER_INFO_COUNT <= next)
        {
            next = 0;
        }

        showUserInfo(static_cast<UserInfo>(next));
        m_timer.restart();
    }
}

void StartupState::exit()
{
    /* Next time start again from begin with the info. */
    m_userInfoState = USER_INFO_TEAM_NAME;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void StartupState::showUserInfo(UserInfo next)
{
    Board&     board    = Board::getInstance();
    IDisplay&  display  = board.getDisplay();
    ISettings& settings = board.getSettings();

    display.clear();

    switch (next)
    {
    case USER_INFO_TEAM_NAME:
        display.print(TEAM_NAME_LINE_1);
        display.gotoXY(0, 1);
        display.print(TEAM_NAME_LINE_2);
        break;

    case USER_INFO_MAX_MOTOR_SPEED:
        display.print("maxSpeed");
        display.gotoXY(0, 1);
        display.print(settings.getMaxSpeed());
        break;

    case USER_INFO_UI:
        display.print("A: MCAL");

        if (true == m_isMaxMotorSpeedCalibAvailable)
        {
            display.gotoXY(0, 1);
            display.print("B: LCAL");
        }
        break;

    case USER_INFO_COUNT:
        /* fallthrough */
    default:
        display.print("?");
        next = USER_INFO_TEAM_NAME;
        break;
    }

    m_userInfoState = next;

    m_timer.start(INFO_DURATION);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
