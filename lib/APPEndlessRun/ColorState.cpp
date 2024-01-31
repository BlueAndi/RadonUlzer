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
 * @brief  Color state
 * @author Paul Gramescu <paul.gramescu@gmail.com>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ColorState.h"

#include <Board.h>
#include <StateMachine.h>

#include "StartupState.h"
#include "DrivingState.h"
#include "DifferentialDrive.h"

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

void ColorState::entry()
{
    /* ColorState is marked with Green LED. */
    Board::getInstance().getGreenLed().enable(true);
}

void ColorState::process(StateMachine& sm)
{
    /* Trigger different functions based on what color the robot received. */
    switch (m_tlcId)
    {
    case COLOR_ID_IDLE:
    {
        /* No new color received. */
        sm.setState(&DrivingState::getInstance());
        break;
    }
    case COLOR_ID_RED:
    {
        /* Toggle other speed methods off to avoid bugs. */
        DrivingState::getInstance().enableSlowdown(false);
        DrivingState::getInstance().enableSpeedup(false);

        DifferentialDrive::getInstance().setLinearSpeed(0, 0);
        break;
    }
    case COLOR_ID_GREEN:
    {
        /* Toggle other speed methods off to avoid bugs. */
        DrivingState::getInstance().enableSlowdown(false);
        DrivingState::getInstance().enableSpeedup(false);

        /* Keep driving. */
        sm.setState(&DrivingState::getInstance());
        break;
    }
    case COLOR_ID_YELLOW_GR:
    {
        DrivingState::getInstance().enableSlowdown(false);

        /* Toggle slowdown flag. */
        DrivingState::getInstance().enableSlowdown(true);

        sm.setState(&DrivingState::getInstance());
        break;
    }
    case COLOR_ID_YELLOW_RG:
    {
        DrivingState::getInstance().enableSlowdown(false);

        /* Toggle speedup flag. */
        DrivingState::getInstance().enableSpeedup(true);

        sm.setState(&DrivingState::getInstance());
        break;
    }
    default:
        /* Fatal error */
        sm.setState(&StartupState::getInstance());
        break;
    }
}

void ColorState::exit()
{
    Board::getInstance().getGreenLed().enable(false);
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
