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
 * @brief  ConvoyLeader application
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "App.h"
#include "StartupState.h"
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
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

static void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void App::setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Board::getInstance().init();
    m_systemStateMachine.setState(&StartupState::getInstance());
    m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);

    /* Setup SerialMuxProt Channels. */
    m_serialMuxProtChannelIdOdometry = m_smpServer.createChannel(ODOMETRY_CHANNEL_NAME, ODOMETRY_CHANNEL_DLC);
    m_serialMuxProtChannelIdSpeed    = m_smpServer.createChannel(SPEED_CHANNEL_NAME, SPEED_CHANNEL_DLC);
    m_smpServer.subscribeToChannel(SPEED_SETPOINT_CHANNEL_NAME, App_motorSpeedSetpointsChannelCallback);

    /* Channel sucesfully created? */
    if ((0U != m_serialMuxProtChannelIdOdometry) && (0U != m_serialMuxProtChannelIdSpeed))
    {
        m_reportTimer.start(REPORTING_PERIOD);
    }
}

void App::loop()
{
    m_smpServer.process(millis());
    Speedometer::getInstance().process();

    if (true == m_controlInterval.isTimeout())
    {
        /* The differential drive control needs the measured speed of the
         * left and right wheel. Therefore it shall be processed after
         * the speedometer.
         */
        DifferentialDrive::getInstance().process(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);

        /* The odometry unit needs to detect motor speed changes to be able to
         * calculate correct values. Therefore it shall be processed right after
         * the differential drive control.
         */
        Odometry::getInstance().process();

        m_controlInterval.restart();
    }

    if (true == m_reportTimer.isTimeout())
    {
        /* Send current data to SerialMuxProt Client */
        reportOdometry();
        reportSpeed();

        m_reportTimer.restart();
    }

    m_systemStateMachine.process();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::reportOdometry()
{
    Odometry&    odometry = Odometry::getInstance();
    OdometryData payload;
    int32_t      xPos = 0;
    int32_t      yPos = 0;

    odometry.getPosition(xPos, yPos);
    payload.xPos        = xPos;
    payload.yPos        = yPos;
    payload.orientation = odometry.getOrientation();

    m_smpServer.sendData(m_serialMuxProtChannelIdOdometry, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

void App::reportSpeed()
{
    Speedometer& speedometer = Speedometer::getInstance();
    SpeedData    payload;
    payload.left  = speedometer.getLinearSpeedLeft();
    payload.right = speedometer.getLinearSpeedRight();

    m_smpServer.sendData(m_serialMuxProtChannelIdSpeed, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Receives motor speed setpoints over SerialMuxProt channel.
 *
 * @param[in] payload       Motor speed left/right
 * @param[in] payloadSize   Size of twice motor speeds
 */
void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize)
{
    if ((nullptr != payload) && (SPEED_SETPOINT_CHANNEL_DLC == payloadSize))
    {
        const SpeedData* motorSpeedData = reinterpret_cast<const SpeedData*>(payload);
        DifferentialDrive::getInstance().setLinearSpeed(motorSpeedData->left, motorSpeedData->right);
    }
}