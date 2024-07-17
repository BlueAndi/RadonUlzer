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
const uint16_t ReadyState::SENSOR_VALUE_MAX = Board::getInstance().getLineSensors().getSensorValueMax();
/* Initialize the required sensor IDs to be generic. */
const uint8_t ReadyState::SENSOR_ID_MOST_LEFT  = 0U;
const uint8_t ReadyState::SENSOR_ID_MIDDLE     = Board::getInstance().getLineSensors().getNumLineSensors() / 2U;
const uint8_t ReadyState::SENSOR_ID_MOST_RIGHT = Board::getInstance().getLineSensors().getNumLineSensors() - 1U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ReadyState::entry()
{
    IDisplay&     display                 = Board::getInstance().getDisplay();
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
    m_ModeTimeoutTimer.start(mode_selected_period);
}

void ReadyState::process(StateMachine& sm)
{
    IButton& buttonA = Board::getInstance().getButtonA();
    IButton& buttonB = Board::getInstance().getButtonB();
    ILineSensors&   lineSensors      = Board::getInstance().getLineSensors();
    uint8_t         maxLineSensors   = lineSensors.getNumLineSensors();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues(); 
    /* Shall the driving mode be released? */
    if (true == buttonA.isPressed())
    {
        buttonA.waitForRelease();
        m_mode = DRIVING_MODE; 
    }
     /* Shall the Training mode be released? */
    else if ((true == buttonB.isPressed()) )
    {   
        buttonB.waitForRelease();
        m_mode = TRAINING_MODE;  
    }
    else if (true == m_ModeTimeoutTimer.isTimeout() && (m_mode == IDLE))
    {
        m_mode = TRAINING_MODE;  
    }
    else
    {
        /* Nothing to do. */
        ;
    }

    DriveUntilStartLinecross();

    if ((isStartStopLineDetected(lineSensorValues, maxLineSensors) == false) && (LastStatus == true))
    {
        sm.setState(&DrivingState::getInstance());
    }
    else 
    {
        LastStatus = isStartStopLineDetected(lineSensorValues, maxLineSensors);
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

uint8_t ReadyState::setSelectedMode()
{
    return (m_mode);
}

void ReadyState :: DriveUntilStartLinecross()
{
    DifferentialDrive& diffDrive       = DifferentialDrive::getInstance();
    int16_t         top_speed          = 2000;
    int16_t         leftMotor          = top_speed / 2U;
    int16_t         rightMotor         = top_speed / 2U;
    diffDrive.setLinearSpeed(leftMotor, rightMotor);
}

bool ReadyState::isStartStopLineDetected(const uint16_t* lineSensorValues, uint8_t length) const
{
    bool           isDetected  = false;
    const uint32_t LINE_MAX_30 = (SENSOR_VALUE_MAX * 3U) / 10U; /* 30 % of max. value */
    const uint32_t LINE_MAX_70 = (SENSOR_VALUE_MAX * 7U) / 10U; /* 70 % of max. value */

    /*
     * ===     =     ===
     *   +   + + +   +
     *   L     M     R
     */
    if ((LINE_MAX_30 <= lineSensorValues[SENSOR_ID_MOST_LEFT]) &&
        (LINE_MAX_70 > lineSensorValues[SENSOR_ID_MIDDLE - 1U]) &&
        (LINE_MAX_70 <= lineSensorValues[SENSOR_ID_MIDDLE]) &&
        (LINE_MAX_70 > lineSensorValues[SENSOR_ID_MIDDLE + 1U]) &&
        (LINE_MAX_30 <= lineSensorValues[SENSOR_ID_MOST_RIGHT]))
    {
        isDetected = true;
    }

    return isDetected;
}
