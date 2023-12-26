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
    IBoard&            board     = Board::getInstance();
    ISettings&         settings  = board.getSettings();
    int16_t            maxSpeed  = settings.getMaxSpeed();
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    /* Load the max. motor speed always from the settings, because
     * it may happen that there was no calibration before.
     */
    diffDrive.setMaxMotorSpeed(maxSpeed);

    if (0 == maxSpeed)
    {
        LOG_ERROR("Calibration is missing.");
    }

    /* Differential drive can now be used. */
    diffDrive.enable();

    m_page = PAGE_DRIVE_FORWARD;
    showCurrentPage();
    m_pageTimer.start(NEXT_PAGE_PERIOD);
}

void ReadyState::process(StateMachine& sm)
{
    IBoard&  board   = Board::getInstance();
    IButton& buttonA = board.getButtonA();
    IButton& buttonB = board.getButtonB();
    IButton& buttonC = board.getButtonC();

    /* Drive forward? */
    if (true == buttonA.isPressed())
    {
        DrivingState::getInstance().setCmd(DrivingState::DRIVING_CMD_FORWARD);
        buttonA.waitForRelease();
        m_releaseTimer.start(RELEASE_DURATION);

        LOG_INFO("Drive forward 10 cm.");
    }

    /* Turn left? */
    if (true == buttonB.isPressed())
    {
        DrivingState::getInstance().setCmd(DrivingState::DRIVING_CMD_TURN_LEFT);
        buttonB.waitForRelease();
        m_releaseTimer.start(RELEASE_DURATION);

        LOG_INFO("Turn left 90°.");
    }

    /* Turn right? */
    if (true == buttonC.isPressed())
    {
        DrivingState::getInstance().setCmd(DrivingState::DRIVING_CMD_TURN_RIGHT);
        buttonC.waitForRelease();
        m_releaseTimer.start(RELEASE_DURATION);

        LOG_INFO("Turn right 90°.");
    }

    /* Release track after specific time. */
    if ((true == m_releaseTimer.isRunning()) && (true == m_releaseTimer.isTimeout()))
    {
        m_releaseTimer.stop();
        sm.setState(&DrivingState::getInstance());
    }
}

void ReadyState::exit()
{
    /* Nothing to do. */
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ReadyState::showCurrentPage()
{
    switch(m_page)
    {
    case PAGE_DRIVE_FORWARD:
        showDriveForwardPage();
        break;

    case PAGE_TURN_LEFT:
        showTurnLeftPage();
        break;

    case PAGE_TURN_RIGHT:
        showTurnRightPage();
        break;

    default:
        break;
    }
}

void ReadyState::showDriveForwardPage()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Press A");
    display.gotoXY(0, 1);
    display.print("drv fwd");
}

void ReadyState::showTurnLeftPage()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Press B");
    display.gotoXY(0, 1);
    display.print("turn left");
}

void ReadyState::showTurnRightPage()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Press C");
    display.gotoXY(0, 1);
    display.print("turn right");
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
