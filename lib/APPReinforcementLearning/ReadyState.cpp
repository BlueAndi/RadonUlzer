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
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ReadyState.h"
#include <Board.h>
#include <StateMachine.h>
#include <DifferentialDrive.h>
#include "DrivingState.h"
#include "ErrorState.h"
#include <Logging.h>
#include <Util.h>
#include <Odometry.h>

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
    IDisplay&     display                 = Board::getInstance().getDisplay();
    const int32_t SENSOR_VALUE_OUT_PERIOD = 1000; /* ms */
    const int32_t Mode_Selected_period    = 1000; /* ms*/
    display.clear();
    display.print("A: TMD");
    display.gotoXY(0, 1);
    display.print("B: DMD");

    if (true == m_isLapTimeAvailable)
    {   
        display.gotoXY(0, 2);
        display.print(m_lapTime);
        display.print("ms");
    }

    /* The line sensor value shall be output on console cyclic. */
    m_timer.start(SENSOR_VALUE_OUT_PERIOD);

    m_ModeTimeoutTimer.start(Mode_Selected_period);
}

void ReadyState::process(StateMachine& sm)
{
    IButton& buttonA = Board::getInstance().getButtonA();
    IButton& buttonB = Board::getInstance().getButtonB();
    uint8_t selectedMode = 0;    
    /* Shall the driving mode be released? */
    if (true == buttonA.isPressed())
    {
        buttonA.waitForRelease();
        myMode = DRIVING_MODE;    
        sm.setState(&DrivingState::getInstance());
    }
     /* Shall the Training mode be released? */
    else if ((true == buttonB.isPressed()) )
    {   
        buttonB.waitForRelease();
        myMode = TRAINING_MODE;
        sm.setState(&DrivingState::getInstance());
    }
    else if (true == m_ModeTimeoutTimer.isTimeout())
    {
        myMode = TRAINING_MODE;
        sm.setState(&DrivingState::getInstance());
        
    }
    else
    {
        /* Nothing to do. */
        ;
    }   

}

void ReadyState::exit()
{
    m_timer.stop();
    m_isLapTimeAvailable = false;
}

void ReadyState::setLapTime(uint32_t lapTime)
{
    m_isLapTimeAvailable = true;
    m_lapTime            = lapTime;
}

uint8_t ReadyState::selectedMode()
{
    return static_cast<uint8_t>(myMode);
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
    /*if (true == m_smpServer.isSynced())
    {*/