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
 * @brief  Remote control state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ColorState.h"

#include <Board.h>
#include <StateMachine.h>

#include "StartupState.h"
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

        m_lastId = COLOR_ID_IDLE;
        break;
    }
    case COLOR_ID_RED:
    {
        /* Toggle other speed methods off to avoid bugs. */
        DrivingState::getInstance().enableSlowdown(false);
        DrivingState::getInstance().enableSpeedup(false);

        /* Stop motors. */
        IMotors& motors = Board::getInstance().getMotors();
        motors.setSpeeds(0, 0);

        m_lastId = COLOR_ID_RED;
        break;
    }
    case COLOR_ID_GREEN:
    {
        /* Toggle other speed methods off to avoid bugs. */
        DrivingState::getInstance().enableSlowdown(false);
        DrivingState::getInstance().enableSpeedup(false);

        /* Keep driving. */
        sm.setState(&DrivingState::getInstance());

        m_lastId = COLOR_ID_GREEN;
        break;
    }
    case COLOR_ID_YELLOW_GR:
    {
        if (m_lastId == COLOR_ID_GREEN)
        {
            /* code */
        }

        /* Toggle slowdown flag. */
        DrivingState::getInstance().enableSlowdown(true);

        sm.setState(&DrivingState::getInstance());

        m_lastId = COLOR_ID_YELLOW_GR;
        m_tlcId = COLOR_ID_IDLE;
        break;
    }
    case COLOR_ID_YELLOW_RG:
    {
        if (m_lastId == COLOR_ID_RED)
        {
            /* code */
        }

        /* Toggle speedup flag. */
        DrivingState::getInstance().enableSpeedup(true);

        sm.setState(&DrivingState::getInstance());

        m_lastId = COLOR_ID_YELLOW_RG;
        m_tlcId = COLOR_ID_IDLE;
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
