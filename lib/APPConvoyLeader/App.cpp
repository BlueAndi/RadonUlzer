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
 * @brief  ConvoyLeader application
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "App.h"
#include "StartupState.h"
#include "ErrorState.h"
#include "DrivingState.h"
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
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
static void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
static void App_initialDataChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);

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

    if (false == setupSerialMuxProt())
    {
        ErrorState::getInstance().setErrorMsg("SMP=0");
        m_systemStateMachine.setState(&ErrorState::getInstance());
    }
    else
    {
        m_reportTimer.start(REPORTING_PERIOD);
        m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);
        m_systemStateMachine.setState(&StartupState::getInstance());
    }
}

void App::loop()
{
    Board::getInstance().process();
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
        reportVehicleData();

        m_reportTimer.restart();
    }

    /* Determine whether the robot can be remote controlled or not. */
    if (&RemoteCtrlState::getInstance() == m_systemStateMachine.getState())
    {
        gIsRemoteCtrlActive = true;
    }
    else
    {
        gIsRemoteCtrlActive = false;
    }

    m_smpServer.process(millis());

    m_systemStateMachine.process();

    /* Send remote control command responses. */
    sendRemoteControlResponses();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::reportVehicleData()
{
    Odometry&    odometry    = Odometry::getInstance();
    Speedometer& speedometer = Speedometer::getInstance();
    VehicleData  payload;
    int32_t      xPos = 0;
    int32_t      yPos = 0;

    odometry.getPosition(xPos, yPos);
    payload.xPos        = xPos;
    payload.yPos        = yPos;
    payload.orientation = odometry.getOrientation();
    payload.left        = speedometer.getLinearSpeedLeft();
    payload.right       = speedometer.getLinearSpeedRight();
    payload.center      = speedometer.getLinearSpeedCenter();

    /* Ignoring return value, as error handling is not available. */
    (void)m_smpServer.sendData(m_serialMuxProtChannelIdCurrentVehicleData, &payload, sizeof(VehicleData));
}

void App::sendRemoteControlResponses()
{
    CommandResponse remoteControlRspId = RemoteCtrlState::getInstance().getCmdRsp();

    /* Send only on change. */
    if ((remoteControlRspId.responseId != m_lastRemoteControlRspId.responseId) ||
        (remoteControlRspId.commandId != m_lastRemoteControlRspId.commandId))
    {
        if (true == m_smpServer.sendData(m_serialMuxProtChannelIdRemoteCtrlRsp, &remoteControlRspId,
                                         sizeof(remoteControlRspId)))
        {
            m_lastRemoteControlRspId = remoteControlRspId;
        }
    }
}

bool App::setupSerialMuxProt()
{
    bool isSuccessful = false;

    /* Channel subscription. */
    m_smpServer.subscribeToChannel(COMMAND_CHANNEL_NAME, App_cmdChannelCallback);
    m_smpServer.subscribeToChannel(SPEED_SETPOINT_CHANNEL_NAME, App_motorSpeedSetpointsChannelCallback);
    m_smpServer.subscribeToChannel(INITIAL_VEHICLE_DATA_CHANNEL_NAME, App_initialDataChannelCallback);

    /* Channel creation. */
    m_serialMuxProtChannelIdRemoteCtrlRsp =
        m_smpServer.createChannel(COMMAND_RESPONSE_CHANNEL_NAME, COMMAND_RESPONSE_CHANNEL_DLC);
    m_serialMuxProtChannelIdCurrentVehicleData =
        m_smpServer.createChannel(CURRENT_VEHICLE_DATA_CHANNEL_NAME, CURRENT_VEHICLE_DATA_CHANNEL_DLC);

    /* Channels succesfully created? */
    if ((0U != m_serialMuxProtChannelIdCurrentVehicleData) && (0U != m_serialMuxProtChannelIdRemoteCtrlRsp))
    {
        isSuccessful = true;
    }

    return isSuccessful;
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
    if ((nullptr != payload) && (sizeof(RemoteCtrlState::CmdId) == payloadSize) && (true == gIsRemoteCtrlActive))
    {
        RemoteCtrlState::CmdId cmdId = *reinterpret_cast<const RemoteCtrlState::CmdId*>(payload);

        RemoteCtrlState::getInstance().execute(cmdId);
    }
}

/**
 * Receives motor speed setpoints over SerialMuxProt channel.
 *
 * @param[in] payload       Motor speed left/right
 * @param[in] payloadSize   Size of twice motor speeds
 * @param[in] userData      User data
 */
void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    (void)userData;
    if ((nullptr != payload) && (SPEED_SETPOINT_CHANNEL_DLC == payloadSize))
    {
        const SpeedData* motorSpeedData = reinterpret_cast<const SpeedData*>(payload);
        DrivingState::getInstance().setTopSpeed(motorSpeedData->center);
    }
}

/**
 * Receives initial data over SerialMuxProt channel.
 *
 * @param[in] payload       Initial vehicle data. Position coordinates, orientation, and motor speeds
 * @param[in] payloadSize   Size of VehicleData struct.
 * @param[in] userData      User data
 */
static void App_initialDataChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    (void)userData;
    if ((nullptr != payload) && (INITIAL_VEHICLE_DATA_CHANNEL_DLC == payloadSize))
    {
        const VehicleData* vehicleData = reinterpret_cast<const VehicleData*>(payload);
        Odometry&          odometry    = Odometry::getInstance();

        odometry.clearPosition();
        odometry.clearMileage();
        odometry.setPosition(vehicleData->xPos, vehicleData->yPos);
        odometry.setOrientation(vehicleData->orientation);

        StartupState::getInstance().notifyInitialDataIsSet();
    }
}
