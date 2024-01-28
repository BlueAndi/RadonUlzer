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
#include "LineSensorsCalibrationState.h"
#include "Sound.h"
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

    /* Surprise the audience. */
    Sound::playMelody(Sound::MELODY_WELCOME);

    /* Show team id / team name */
    display.clear();
    display.print(TEAM_NAME_LINE_1);
    display.gotoXY(0, 1);
    display.print(TEAM_NAME_LINE_2);
    delay(TEAM_NAME_DURATION);

    /* Show operator info on LCD */
    display.clear();
    display.print("Press A");
    display.gotoXY(0, 1);
    display.print("to calib");
}

void StartupState::process(StateMachine& sm)
{
    Board&   board   = Board::getInstance();
    IButton& buttonA = board.getButtonA();
    IButton& buttonB = board.getButtonB();

    /* Start max. motor speed calibration? */
    if (true == buttonA.isPressed())
    {
        buttonA.waitForRelease();
        sm.setState(&MotorSpeedCalibrationState::getInstance());
    }
    /* Load max. motor speed from settings and start line sensor calibration? */
    else if (true == buttonB.isPressed())
    {
        IDisplay&          display   = board.getDisplay();
        ISettings&         settings  = board.getSettings();
        DifferentialDrive& diffDrive = DifferentialDrive::getInstance();
        int16_t            maxSpeed  = settings.getMaxSpeed();

        buttonB.waitForRelease();

        /* If no calibration value is available, skip it and notify user. */
        if (0 == maxSpeed)
        {
            display.clear();
            display.print("Press A");
            display.gotoXY(0, 1);
            display.print("MCAL=0");
        }
        /* Calibration value is available. */
        else
        {
            /* With setting the max. motor speed in [steps/s] the differential drive control
             * can now be used.
             */
            diffDrive.setMaxMotorSpeed(maxSpeed);

            /* Differential drive can now be used. */
            diffDrive.enable();

            sm.setState(&LineSensorsCalibrationState::getInstance());
        }
    }
    else
    {
        /* Nothing to do. */
        ;
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
