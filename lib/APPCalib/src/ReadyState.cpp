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
 * @brief  Ready state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ReadyState.h"
#include <Board.h>
#include <StateMachine.h>
#include <Settings.h>
#include <DifferentialDrive.h>
#include <Logging.h>
#include "DrivingState.h"
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

/**
 * Logging source.
 */
LOG_TAG("RState");

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ReadyState::entry()
{
    showUserInfo(m_userInfoState);
}

void ReadyState::process(StateMachine& sm)
{
    IBoard&  board   = Board::getInstance();
    IButton& buttonA = board.getButtonA();
    IButton& buttonB = board.getButtonB();
    IButton& buttonC = board.getButtonC();

    /* Drive forward? */
    if (true == Util::isButtonTriggered(buttonA, m_isButtonAPressed))
    {
        DrivingState::getInstance().setCmd(DrivingState::DRIVING_CMD_FORWARD);

        m_releaseTimer.start(RELEASE_DURATION);

        LOG_INFO("Drive forward 10 cm.");
    }

    /* Turn left? */
    if (true == Util::isButtonTriggered(buttonB, m_isButtonBPressed))
    {
        DrivingState::getInstance().setCmd(DrivingState::DRIVING_CMD_TURN_LEFT);

        m_releaseTimer.start(RELEASE_DURATION);

        LOG_INFO("Turn left 90°.");
    }

    /* Turn right? */
    if (true == Util::isButtonTriggered(buttonC, m_isButtonCPressed))
    {
        DrivingState::getInstance().setCmd(DrivingState::DRIVING_CMD_TURN_RIGHT);

        m_releaseTimer.start(RELEASE_DURATION);

        LOG_INFO("Turn right 90°.");
    }

    /* Release track after specific time. */
    if ((true == m_releaseTimer.isRunning()) && (true == m_releaseTimer.isTimeout()))
    {
        m_releaseTimer.stop();
        sm.setState(&DrivingState::getInstance());
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

void ReadyState::exit()
{
    /* Next time start again from begin with the info. */
    m_userInfoState = USER_INFO_DRIVE_FORWARD;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ReadyState::showUserInfo(UserInfo next)
{
    Board&    board   = Board::getInstance();
    IDisplay& display = board.getDisplay();

    display.clear();

    switch (next)
    {
    case USER_INFO_DRIVE_FORWARD:
        display.print("A:");
        display.gotoXY(0, 1);
        display.print("DRV FWD");
        break;

    case USER_INFO_TURN_LEFT:
        display.print("B:");
        display.gotoXY(0, 1);
        display.print("TURN L");
        break;

    case USER_INFO_TURN_RIGHT:
        display.print("C:");
        display.gotoXY(0, 1);
        display.print("TURN R");
        break;

    case USER_INFO_COUNT:
        /* fallthrough */
    default:
        display.print("?");
        next = USER_INFO_DRIVE_FORWARD;
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
