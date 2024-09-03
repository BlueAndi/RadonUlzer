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
 * @brief  Ready state
 * @author Akram Bziouech
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ReadyState.h"
#include <Board.h>
#include <StateMachine.h>
#include <DifferentialDrive.h>
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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ReadyState::entry()
{
    IDisplay& display = Board::getInstance().getDisplay();
    display.clear();
    display.print("A: TMD");
    display.gotoXY(0, 1);
    display.print("B: DMD");

    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();
    diffDrive.setLinearSpeed(0, 0);

    if (true == m_isLapTimeAvailable)
    {
        display.gotoXY(0, 2);
        display.print(m_lapTime);
        display.print("ms");
    }
    m_stateTransitionTimer.start(STATE_TRANSITION_PERIOD);
    m_modeTimeoutTimer.start(MODE_SELECTED_PERIOD);
    m_mode                        = IDLE;
    m_isLastStartStopLineDetected = false;
    m_isButtonAPressed            = false;
    m_isButtonBPressed            = false;
}

void ReadyState::process(StateMachine& sm)
{
    IButton& buttonA = Board::getInstance().getButtonA();
    IButton& buttonB = Board::getInstance().getButtonB();

    /* Shall the driving mode be released? */
    if (true == Util::isButtonTriggered(buttonA, m_isButtonAPressed))
    {
        m_mode = DRIVING_MODE;
    }

    /* Shall the Training mode be released? */
    else if (true == Util::isButtonTriggered(buttonB, m_isButtonBPressed))
    {
        m_mode = TRAINING_MODE;
    }

    /*
     * If either Button A or Button B is not pressed and the timer
     * expires, the training mode should automatically activate.
     */
    else if (true == m_modeTimeoutTimer.isTimeout() && (IDLE == m_mode))
    {
        m_mode = TRAINING_MODE;
        m_modeTimeoutTimer.restart();
    }

    else
    {
        /* Nothing to do. */
        ;
    }

    /* Set DrivingState */
    if (true == m_stateTransitionTimer.isTimeout())
    {
        sm.setState(&DrivingState::getInstance());
        m_stateTransitionTimer.restart();
    }
}

void ReadyState::exit()
{
    m_isLapTimeAvailable = false;
}

void ReadyState::setLapTime(uint32_t lapTime)
{
    m_isLapTimeAvailable = true;
    m_lapTime            = lapTime;
}

uint8_t ReadyState::getSelectedMode()
{
    return m_mode;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

ReadyState::ReadyState() :
    m_isLapTimeAvailable(false),
    m_isButtonAPressed(false),
    m_isButtonBPressed(false),
    m_modeTimeoutTimer(),
    m_stateTransitionTimer(),
    m_lapTime(0),
    m_isLastStartStopLineDetected(false),
    m_mode(IDLE)
{
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
