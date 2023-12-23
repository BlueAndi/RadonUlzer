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
#include "StartupState.h"
#include <Board.h>
#include <StateMachine.h>
#include "MotorSpeedCalibrationState.h"
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

void StartupState::entry()
{
    /* Initialize HAL */
    Board::getInstance().init();

    m_page = PAGE_CALIB;
    showCurrentPage();
    m_pageTimer.start(NEXT_PAGE_PERIOD);
}

void StartupState::process(StateMachine& sm)
{
    IBoard&  board   = Board::getInstance();
    IButton& buttonA = board.getButtonA();
    IButton& buttonB = board.getButtonB();

    /* Flip display page periodically. */
    if (true == m_pageTimer.isTimeout())
    {
        if (PAGE_CALIB == m_page)
        {
            m_page = PAGE_CONTINUE;
        }
        else
        {
            m_page = PAGE_CALIB;
        }

        showCurrentPage();
        m_pageTimer.restart();
    }

    if (true == buttonA.isPressed())
    {
        buttonA.waitForRelease();
        sm.setState(&MotorSpeedCalibrationState::getInstance());
    }

    if (true == buttonB.isPressed())
    {
        buttonB.waitForRelease();
        sm.setState(&ReadyState::getInstance());
    }
}

void StartupState::exit()
{
    m_pageTimer.stop();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void StartupState::showCurrentPage()
{
    switch(m_page)
    {
    case PAGE_CALIB:
        showCalibrationPage();
        break;

    case PAGE_CONTINUE:
        showContinuePage();
        break;

    default:
        break;
    }
}

void StartupState::showCalibrationPage()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Press A");
    display.gotoXY(0, 1);
    display.print("to calib");
}

void StartupState::showContinuePage()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Press B");
    display.gotoXY(0, 1);
    display.print("to cont");
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
