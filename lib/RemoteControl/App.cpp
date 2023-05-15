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
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
#include <Board.h>

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

static void App_cmdChannelCallback(const uint8_t* payload, const uint8_t payloadSize);
static void App_motorSpeedsChannelCallback(const uint8_t* payload, const uint8_t payloadSize);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize channel name for receiving commands. */
const char* App::CH_NAME_CMD = "REMOTE_CMD";

/* Initialize channel name for sending command responses. */
const char* App::CH_NAME_RSP = "REMOTE_RSP";

/* Initialize channel name for receiving commands. */
const char* App::CH_NAME_MOTOR_SPEEDS = "MOTOR_SPEEDS";

/* Initialize channel name for sending line sensors data. */
const char* App::CH_NAME_LINE_SENSORS = "LINE_SENSORS";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void App::setup()
{
    Board::getInstance().init();
    m_systemStateMachine.setState(&StartupState::getInstance());
    m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);
    m_sendLineSensorsDataInterval.start(SEND_LINE_SENSORS_DATA_PERIOD);

    /* Remote control commands/responses */
    m_yapServer.subscribeToChannel(CH_NAME_CMD, App_cmdChannelCallback);
    m_yapChannelIdRemoteCtrlRsp = m_yapServer.createChannel(CH_NAME_RSP, sizeof(RemoteCtrlState::RspId));

    /* Receiving linear motor speed left/right */
    m_yapServer.subscribeToChannel(CH_NAME_MOTOR_SPEEDS, App_motorSpeedsChannelCallback);

    /* Providing line sensor data */
    m_yapChannelIdLineSensors = m_yapServer.createChannel(CH_NAME_LINE_SENSORS, sizeof(RemoteCtrlState::RspId));
}

void App::loop()
{
    m_yapServer.process(millis());
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

    if (true == m_sendLineSensorsDataInterval.isTimeout())
    {
        sendLineSensorsData();

        m_sendLineSensorsDataInterval.restart();
    }
    
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
        const uint8_t* payload = reinterpret_cast<const uint8_t*>(&remoteControlRspId);

        (void)m_yapServer.sendData(m_yapChannelIdRemoteCtrlRsp, payload, sizeof(remoteControlRspId));

        m_lastRemoteControlRspId = remoteControlRspId;
    }
}

void App::sendLineSensorsData() const
{
    ILineSensors&   lineSensors      = Board::getInstance().getLineSensors();
    uint8_t         maxLineSensors   = lineSensors.getNumLineSensors();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues();
    uint8_t         payload[maxLineSensors * sizeof(uint16_t)];
    uint8_t         lineSensorIdx    = 0U;

    while(maxLineSensors > lineSensorIdx)
    {
        Util::uint16ToByteArray(&payload[lineSensorIdx * sizeof(uint16_t)], sizeof(uint16_t), lineSensorValues[lineSensorIdx]);

        ++lineSensorIdx;
    }

    (void)m_yapServer.sendData(m_yapChannelIdLineSensors, payload, sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Receives remote control commands over YAP channel.
 *
 * @param[in] payload       Command id
 * @param[in] payloadSize   Size of command id
 */
static void App_cmdChannelCallback(const uint8_t* payload, const uint8_t payloadSize)
{
    if ((nullptr != payload) && (sizeof(RemoteCtrlState::CmdId) == payloadSize))
    {
        RemoteCtrlState::CmdId cmdId = *reinterpret_cast<const RemoteCtrlState::CmdId*>(payload);

        RemoteCtrlState::getInstance().execute(cmdId);
    }
}

/**
 * Receives motor speeds over YAP channel.
 *
 * @param[in] payload       Motor speed left/right
 * @param[in] payloadSize   Size of twice motor speeds
 */
static void App_motorSpeedsChannelCallback(const uint8_t* payload, const uint8_t payloadSize)
{
    if ((nullptr != payload) && ((2U * sizeof(uint16_t)) == payloadSize))
    {
        int16_t linearSpeedLeft;
        int16_t linearSpeedRight;
        bool convResultLSL = Util::byteArrayToInt16(&payload[0U * sizeof(int16_t)], sizeof(int16_t), linearSpeedLeft);
        bool convResultLSR = Util::byteArrayToInt16(&payload[1U * sizeof(int16_t)], sizeof(int16_t), linearSpeedRight);

        if ((true == convResultLSL) && (true == convResultLSR))
        {
            DifferentialDrive::getInstance().setLinearSpeed(linearSpeedLeft, linearSpeedRight);
        }
    }
}
