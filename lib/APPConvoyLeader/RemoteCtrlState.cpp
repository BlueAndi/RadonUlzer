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
 * @brief  Remote control state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RemoteCtrlState.h"
#include "LineSensorsCalibrationState.h"
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

void RemoteCtrlState::entry()
{
    IDisplay& display = Board::getInstance().getDisplay();

    display.clear();
    display.print("Rdy.");

    IBoard&            board     = Board::getInstance();
    ISettings&         settings  = board.getSettings();
    int16_t            maxSpeed  = settings.getMaxSpeed();
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    /* Load the max. motor speed always from the settings, because
     * it may happen that there was no calibration before.
     */
    diffDrive.setMaxMotorSpeed(maxSpeed);
    diffDrive.enable();

    /* It is assumed that by entering this state, that a pending command will be complete. */
    finishCommand(RSP_ID_OK);
}

void RemoteCtrlState::process(StateMachine& sm)
{
    switch (m_cmdId)
    {
    case CMD_ID_IDLE:
        /* Nothing to do. */
        break;

    case CMD_ID_START_LINE_SENSOR_CALIB:
        sm.setState(&LineSensorsCalibrationState::getInstance());
        break;

    case CMD_ID_START_MOTOR_SPEED_CALIB:
        /* Not available in this application. */
        finishCommand(RSP_ID_ERROR);
        break;

    case CMD_ID_REINIT_BOARD:
        /* Ensure that the motors are stopped, before re-initialize the board. */
        DifferentialDrive::getInstance().setLinearSpeed(0, 0);

        /* Re-initialize the board. This is required for the webots simulation in
         * case the world is reset by a supervisor without restarting the RadonUlzer
         * controller executable.
         */
        Board::getInstance().init();

        finishCommand(RSP_ID_OK);
        break;

    case CMD_ID_GET_MAX_SPEED:
        m_cmdRsp.maxMotorSpeed = DifferentialDrive::getInstance().getMaxMotorSpeed();
        finishCommand(RSP_ID_OK);
        break;

    case CMD_ID_START_DRIVING:
        sm.setState(&DrivingState::getInstance());
        break;

    default:
        break;
    }
}

void RemoteCtrlState::exit()
{
    DifferentialDrive::getInstance().disable();
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
