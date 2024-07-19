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
#include "ErrorState.h"
#include "DrivingState.h"
#include <Board.h>
#include <StateMachine.h>
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
    IDisplay& display = Board::getInstance().getDisplay();

    /* Initialize HAL */
    Board::getInstance().init();

    display.clear();
    display.print("Remote");
    display.gotoXY(0, 1);
    display.print("Ctrl");
    delay(APP_NAME_DURATION);
}

void StartupState::process(StateMachine& sm)
{
    IBoard&            board     = Board::getInstance();
    ISettings&         settings  = board.getSettings();
    int16_t            maxSpeed  = settings.getMaxSpeed();
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    /* Load the max. motor speed always from the settings, because
     * it may happen that there was no calibration before.
     */
    diffDrive.setMaxMotorSpeed(maxSpeed);
    diffDrive.enable();

    if (0 == maxSpeed)
    {
        /* Missing calibration data. Run AppCalib first. */
        ErrorState::getInstance().setErrorMsg("MCAL=0");
        sm.setState(&ErrorState::getInstance());
    }
    else if (true == m_initialDataSet)
    {
        sm.setState(&DrivingState::getInstance());
    }
}

void StartupState::exit()
{
    /* Nothing to do */
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
