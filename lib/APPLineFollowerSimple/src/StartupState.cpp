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
 * @brief  Startup state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "StartupState.h"
#include <Board.h>
#include <StateMachine.h>
#include "LineSensorsCalibrationState.h"
#include "Sound.h"
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

void StartupState::entry()
{
    showUserInfo(m_userInfoState);
}

void StartupState::process(StateMachine& sm)
{
    Board&   board   = Board::getInstance();
    IButton& buttonA = board.getButtonA();

    /* Start line sensor calibration? */
    if (true == buttonA.isPressed())
    {
        buttonA.waitForRelease();
        sm.setState(&LineSensorsCalibrationState::getInstance());
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

    display.clear();

    switch (next)
    {
    case USER_INFO_TEAM_NAME:
        display.print(TEAM_NAME_LINE_1);
        display.gotoXY(0, 1);
        display.print(TEAM_NAME_LINE_2);
        break;

    case USER_INFO_UI:
        display.print("A: LCAL");
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
