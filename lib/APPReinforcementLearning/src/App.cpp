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
 * @brief  LineFollower application with Reinforcement Learning
 * @author Akram Bziouech
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "App.h"
#include "DrivingState.h"
#include "StartupState.h"
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include "ErrorState.h"
#include <Odometry.h>
#include "ReadyState.h"
#include <Logging.h>
#include "TrainingState.h"

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

static void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
static void App_cmdChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void App::setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    /* Initialize HAL */
    Board::getInstance().init();
    Logging::disable();

    if (false == setupSerialMuxProt())
    {
        ErrorState::getInstance().setErrorMsg("SMP=0");
        m_systemStateMachine.setState(&ErrorState::getInstance());
    }
    else
    {
        m_statusTimer.start(SEND_STATUS_TIMER_INTERVAL);
        m_sendLineSensorsDataInterval.start(SEND_LINE_SENSORS_DATA_PERIOD);
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

    if ((true == m_statusTimer.isTimeout()) && (true == m_smpServer.isSynced()) &&
        (&DrivingState::getInstance() == m_systemStateMachine.getState()))
    {
        Status payload = {SMPChannelPayload::Status::NOT_DONE};

        if (true == DrivingState::getInstance().isAbortRequired())
        {
            payload = {SMPChannelPayload::Status::DONE};
        }

        m_data_sent = m_smpServer.sendData(m_serialMuxProtChannelIdStatus, &payload, sizeof(payload));

        m_statusTimer.restart();
    }

    /* Send periodically line sensor data. */
    if (true == m_sendLineSensorsDataInterval.isTimeout() &&
        (&DrivingState::getInstance() == m_systemStateMachine.getState()))
    {
        sendLineSensorsData();

        m_sendLineSensorsDataInterval.restart();
    }

    /* Send Mode selected to The Supervisor. */
    if (&ReadyState::getInstance() == m_systemStateMachine.getState() && (false == m_modeSelectionSent))
    {
        uint8_t mode_options = ReadyState::getInstance().getSelectedMode();

        if (0U < mode_options)
        {
            SMPChannelPayload::Mode payload =
                (1 == mode_options) ? SMPChannelPayload::Mode::DRIVING_MODE : SMPChannelPayload::Mode::TRAINING_MODE;

            m_data_sent = m_smpServer.sendData(m_serialMuxProtChannelIdMode, &payload, sizeof(payload));

            m_modeSelectionSent = true;
        }
    }

    if (false == m_data_sent)
    {
        /* Failed to send data to the supervisor. Go to error state. */
        ErrorState::getInstance().setErrorMsg("DSF");
        m_systemStateMachine.setState(&ErrorState::getInstance());
    }

    m_smpServer.process(millis());

    m_systemStateMachine.process();
}

void App::handleRemoteCommand(const Command& cmd)
{

    switch (cmd.commandId)
    {
    case SMPChannelPayload::CmdId::CMD_ID_IDLE:
        /* Nothing to do. */
        break;

    case SMPChannelPayload::CmdId::CMD_ID_SET_READY_STATE:
        m_systemStateMachine.setState(&ReadyState::getInstance());
        m_modeSelectionSent = false;
        break;

    case SMPChannelPayload::CmdId::CMD_ID_SET_TRAINING_STATE:
        Odometry::getInstance().clearPosition();
        Odometry::getInstance().clearMileage();

        m_systemStateMachine.setState(&TrainingState::getInstance());
        break;

    default:
        /* Nothing to do. */
        break;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::sendLineSensorsData() 
{
    ILineSensors&   lineSensors      = Board::getInstance().getLineSensors();
    uint8_t         maxLineSensors   = lineSensors.getNumLineSensors();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues();
    uint8_t         lineSensorIdx    = 0U;
    LineSensorData  payload;

    if (LINE_SENSOR_CHANNEL_DLC == (maxLineSensors * sizeof(uint16_t)))
    {
        while (maxLineSensors > lineSensorIdx)
        {
            payload.lineSensorData[lineSensorIdx] = lineSensorValues[lineSensorIdx];

            ++lineSensorIdx;
        }
    }

    /* Ignoring return value, as error handling is not available. */
    m_data_sent = m_smpServer.sendData(m_serialMuxProtChannelIdLineSensors, &payload, sizeof(payload));
}

bool App::setupSerialMuxProt()
{
    bool isSuccessful = false;

    /* Channel subscription. */
    m_smpServer.subscribeToChannel(SPEED_SETPOINT_CHANNEL_NAME, App_motorSpeedSetpointsChannelCallback);
    m_smpServer.subscribeToChannel(COMMAND_CHANNEL_NAME, App_cmdChannelCallback);

    /* Channel creation. */
    m_serialMuxProtChannelIdStatus      = m_smpServer.createChannel(STATUS_CHANNEL_NAME, STATUS_CHANNEL_DLC);
    m_serialMuxProtChannelIdLineSensors = m_smpServer.createChannel(LINE_SENSOR_CHANNEL_NAME, LINE_SENSOR_CHANNEL_DLC);
    m_serialMuxProtChannelIdMode        = m_smpServer.createChannel(MODE_CHANNEL_NAME, MODE_CHANNEL_DLC);

    /* Channels succesfully created? */
    if ((0U != m_serialMuxProtChannelIdStatus) && (0U != m_serialMuxProtChannelIdLineSensors) &&
        (0U != m_serialMuxProtChannelIdMode))
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
    if ((nullptr != payload) && (COMMAND_CHANNEL_DLC == payloadSize) && (nullptr != userData))
    {
        App*          application = reinterpret_cast<App*>(userData);
        const Command cmd         = *reinterpret_cast<const Command*>(payload);
        application->handleRemoteCommand(cmd);
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
        DrivingState::getInstance().setTargetSpeeds(motorSpeedData->left, motorSpeedData->right);
    }
}
