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
 * @brief  RemoteControl application
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "App.h"
#include "StartupState.h"
#include "RemoteCtrlState.h"
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
#include <Board.h>
#include <Util.h>
#include <Logging.h>

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

static void App_cmdChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
static void App_motorSpeedsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Only in remote control state its possible to control the robot. */
static bool gIsRemoteCtrlActive = false;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void App::setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Logging::disable();
    Board::getInstance().init();

    m_systemStateMachine.setState(&StartupState::getInstance());
    m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);
    m_sendLineSensorsDataInterval.start(SEND_LINE_SENSORS_DATA_PERIOD);

    /* Remote control commands/responses */
    m_smpServer.subscribeToChannel(COMMAND_CHANNEL_NAME, App_cmdChannelCallback);
    m_smpChannelIdRemoteCtrlRsp =
        m_smpServer.createChannel(COMMAND_RESPONSE_CHANNEL_NAME, COMMAND_RESPONSE_CHANNEL_DLC);

    /* Receiving linear motor speed left/right */
    m_smpServer.subscribeToChannel(SPEED_SETPOINT_CHANNEL_NAME, App_motorSpeedsChannelCallback);

    /* Providing line sensor data */
    m_smpChannelIdLineSensors = m_smpServer.createChannel(LINE_SENSOR_CHANNEL_NAME, LINE_SENSOR_CHANNEL_DLC);
}

void App::loop()
{
    Board::getInstance().process();
    m_smpServer.process(millis());
    Speedometer::getInstance().process();

    if (true == m_controlInterval.isTimeout())
    {
        DifferentialDrive::getInstance().process(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);

        /* The odometry unit needs to detect motor speed changes to be able to
         * calculate correct values. Therefore it shall be processed right after
         * the differential drive control.
         */
        Odometry::getInstance().process();

        m_controlInterval.restart();
    }

    m_systemStateMachine.process();

    /* Determine whether the robot can be remote controlled or not. */
    if (&RemoteCtrlState::getInstance() == m_systemStateMachine.getState())
    {
        gIsRemoteCtrlActive = true;
    }
    else
    {
        gIsRemoteCtrlActive = false;
    }

    /* Send periodically line sensor data. */
    if (true == m_sendLineSensorsDataInterval.isTimeout())
    {
        sendLineSensorsData();

        m_sendLineSensorsDataInterval.restart();
    }

    /* Send remote control command responses. */
    sendRemoteControlResponses();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::sendRemoteControlResponses()
{
    RemoteCtrlState::RspId remoteControlRspId = RemoteCtrlState::getInstance().getCmdRsp();

    /* Send only on change. */
    if (remoteControlRspId != m_lastRemoteControlRspId)
    {
        const CommandResponse* payload = reinterpret_cast<const CommandResponse*>(&remoteControlRspId);

        (void)m_smpServer.sendData(m_smpChannelIdRemoteCtrlRsp, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));

        m_lastRemoteControlRspId = remoteControlRspId;
    }
}

void App::sendLineSensorsData() const
{
    ILineSensors&   lineSensors      = Board::getInstance().getLineSensors();
    uint8_t         maxLineSensors   = lineSensors.getNumLineSensors();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues();
    uint8_t         lineSensorIdx    = 0U;
    LineSensorData  payload;

    if (LINE_SENSOR_CHANNEL_DLC == maxLineSensors * sizeof(uint16_t))
    {
        while (maxLineSensors > lineSensorIdx)
        {
            payload.lineSensorData[lineSensorIdx] = lineSensorValues[lineSensorIdx];

            ++lineSensorIdx;
        }
    }

    (void)m_smpServer.sendData(m_smpChannelIdLineSensors, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Receives remote control commands over SerialMuxProt channel.
 *
 * @param[in] payload       Command id
 * @param[in] payloadSize   Size of command id
 * @param[in] userData      User data
 */
static void App_cmdChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    (void)userData;
    if ((nullptr != payload) && (sizeof(RemoteCtrlState::CmdId) == payloadSize))
    {
        RemoteCtrlState::CmdId cmdId = *reinterpret_cast<const RemoteCtrlState::CmdId*>(payload);

        RemoteCtrlState::getInstance().execute(cmdId);
    }
}

/**
 * Receives motor speeds over SerialMuxProt channel.
 *
 * @param[in] payload       Motor speed left/right
 * @param[in] payloadSize   Size of twice motor speeds
 * @param[in] userData      User data
 */
static void App_motorSpeedsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    (void)userData;
    if ((nullptr != payload) && (SPEED_SETPOINT_CHANNEL_DLC == payloadSize) && (true == gIsRemoteCtrlActive))
    {
        const SpeedData* motorSpeedData = reinterpret_cast<const SpeedData*>(payload);
        DifferentialDrive::getInstance().setLinearSpeed(motorSpeedData->left, motorSpeedData->right);
    }
}
