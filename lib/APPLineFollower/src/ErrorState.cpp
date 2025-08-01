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
 * @brief  Error state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ErrorState.h"
#include <Board.h>
#include <StateMachine.h>
#include "StartupState.h"
#include <DifferentialDrive.h>
#include <Logging.h>
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
 * Error logging tag.
 */
LOG_TAG("EState");

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ErrorState::entry()
{
    IBoard&            board     = Board::getInstance();
    IDisplay&          display   = board.getDisplay();
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    /* Stop the motors in any case! */
    diffDrive.disable();

    display.clear();
    display.print("A: CONT");
    display.gotoXY(0, 1);

    if ('\0' == m_errorMsg[0])
    {
        display.print("ERR");
    }
    else
    {
        display.print(m_errorMsg);
    }

    LOG_ERROR_VAL("Error: ", m_errorMsg);
}

void ErrorState::process(StateMachine& sm)
{
    IButton& buttonA = Board::getInstance().getButtonA();

    /* Restart calibration? */
    if (true == Util::isButtonTriggered(buttonA, m_isButtonAPressed))
    {
        sm.setState(&StartupState::getInstance());
    }
}

void ErrorState::exit()
{
    /* Nothing to do. */
}

void ErrorState::setErrorMsg(const char* msg)
{
    if (nullptr == msg)
    {
        m_errorMsg[0] = '\0';
    }
    else
    {
        strncpy(m_errorMsg, msg, ERROR_MSG_SIZE - 1);
        m_errorMsg[ERROR_MSG_SIZE - 1] = '\0';
    }
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
