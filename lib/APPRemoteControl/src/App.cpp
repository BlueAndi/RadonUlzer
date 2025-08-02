/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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
#include "ErrorState.h"
#include "DrivingState.h"
#include "LineSensorsCalibrationState.h"
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
#include <Logging.h>
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

static void App_cmdChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
static void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
static void App_statusChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
static void App_robotSpeedSetpointChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

App::App() :
    m_serialMuxProtChannelIdRemoteCtrlRsp(0U),
    m_serialMuxProtChannelIdCurrentVehicleData(0U),
    m_serialMuxProtChannelIdStatus(0U),
    m_serialMuxProtChannelIdLineSensors(0U),
    m_systemStateMachine(),
    m_controlInterval(),
    m_reportTimer(),
    m_statusTimer(),
    m_statusTimeoutTimer(),
    m_sendLineSensorsDataInterval(),
    m_smpServer(Serial, this),
    m_isLineSensorCalibPending(false),
    m_movAvgProximitySensor()
{
}

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
        m_statusTimer.start(SEND_STATUS_TIMER_INTERVAL);
        m_sendLineSensorsDataInterval.start(SEND_LINE_SENSORS_DATA_PERIOD);
        m_systemStateMachine.setState(&StartupState::getInstance());

#if CONFIG_SUPERVISOR != 0
#if CONFIG_ODOMETRY_TO_SUPERVISOR != 0
        /* Reset supervisor which set its observed position and orientation to 0. */
        Board::getInstance().getSupervisorSerialDrv().print("RST");
#endif /* CONFIG_ODOMETRY_TO_SUPERVISOR != 0 */
#endif /* CONFIG_SUPERVISOR != 0 */
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

#if CONFIG_SUPERVISOR != 0
#if CONFIG_ODOMETRY_TO_SUPERVISOR != 0
        {
            Odometry&    odo         = Odometry::getInstance();
            int32_t      posX        = 0;
            int32_t      posY        = 0;
            int32_t      orientation = odo.getOrientation();
            const size_t BUFFER_SIZE = 128U;
            char         buffer[BUFFER_SIZE];

            odo.getPosition(posX, posY);

            snprintf(buffer, BUFFER_SIZE, "ODO,%d,%d,%d", posX, posY, orientation);

            Board::getInstance().getSupervisorSerialDrv().print(buffer);
        }
#endif /* CONFIG_ODOMETRY_TO_SUPERVISOR != 0 */
#endif /* CONFIG_SUPERVISOR != 0 */

        m_controlInterval.restart();
    }

    if ((true == m_reportTimer.isTimeout()) && (true == m_smpServer.isSynced()))
    {
        /* Send current data to SerialMuxProt Client */
        reportVehicleData();

        m_reportTimer.restart();
    }

    if ((true == m_statusTimer.isTimeout()) && (true == m_smpServer.isSynced()))
    {
        Status payload = {SMPChannelPayload::Status::STATUS_FLAG_OK};

        if (&ErrorState::getInstance() == m_systemStateMachine.getState())
        {
            payload.status = SMPChannelPayload::Status::STATUS_FLAG_ERROR;
        }

        /* Ignoring return value, as error handling is not available. */
        (void)m_smpServer.sendData(m_serialMuxProtChannelIdStatus, &payload, sizeof(payload));

        m_statusTimer.restart();
    }

    if (true == m_statusTimeoutTimer.isTimeout())
    {
        if (false == m_smpServer.isSynced())
        {
            /* Not receiving status from DCS. Go to error state. */
            ErrorState::getInstance().setErrorMsg("DCS_TO");
            m_systemStateMachine.setState(&ErrorState::getInstance());
            m_statusTimeoutTimer.stop();
        }
        else
        {
            /* Start status timeout timer once SMP is synced the first time. */
            m_statusTimeoutTimer.start(STATUS_TIMEOUT_TIMER_INTERVAL);
        }
    }

    /* Send periodically line sensor data. */
    if ((true == m_sendLineSensorsDataInterval.isTimeout()) && (true == m_smpServer.isSynced()))
    {
        sendLineSensorsData();

        m_sendLineSensorsDataInterval.restart();
    }

    m_smpServer.process(millis());

    m_systemStateMachine.process();

    /* If line sensor calibration is completed, send response to the remote driver. */
    if ((true == m_isLineSensorCalibPending) &&
        (&LineSensorsCalibrationState::getInstance() != m_systemStateMachine.getState()))
    {
        CommandResponse rsp = {SMPChannelPayload::CmdId::CMD_ID_START_LINE_SENSOR_CALIB, SMPChannelPayload::RSP_ID_OK};

        (void)m_smpServer.sendData(m_serialMuxProtChannelIdRemoteCtrlRsp, &rsp, sizeof(rsp));

        m_isLineSensorCalibPending = false;
    }
}

void App::handleRemoteCommand(const Command& cmd)
{
    CommandResponse rsp = {cmd.commandId, SMPChannelPayload::RSP_ID_OK};

    switch (cmd.commandId)
    {
    case SMPChannelPayload::CmdId::CMD_ID_IDLE:
        /* Nothing to do. */
        break;

    case SMPChannelPayload::CmdId::CMD_ID_START_LINE_SENSOR_CALIB:
        rsp.responseId             = SMPChannelPayload::RSP_ID_PENDING;
        m_isLineSensorCalibPending = true;
        m_systemStateMachine.setState(&LineSensorsCalibrationState::getInstance());
        break;

    case SMPChannelPayload::CmdId::CMD_ID_START_MOTOR_SPEED_CALIB:
        /* Not supported. */
        break;

    case SMPChannelPayload::CmdId::CMD_ID_REINIT_BOARD:
        /* Ensure that the motors are stopped, before re-initialize the board. */
        DifferentialDrive::getInstance().setLinearSpeed(0, 0);

        /* Re-initialize the board. This is required for the webots simulation in
         * case the world is reset by a supervisor without restarting the RadonUlzer
         * controller executable.
         */
        Board::getInstance().init();
        break;

    case SMPChannelPayload::CmdId::CMD_ID_GET_MAX_SPEED:
        rsp.maxMotorSpeed =
            Util::stepsPerSecondToMillimetersPerSecond(Board::getInstance().getSettings().getMaxSpeed());
        break;

    case SMPChannelPayload::CmdId::CMD_ID_START_DRIVING:
        /* Not supported. */
        break;

    case SMPChannelPayload::CmdId::CMD_ID_SET_INIT_POS:
        Odometry::getInstance().clearPosition();
        Odometry::getInstance().clearMileage();
        Odometry::getInstance().setPosition(cmd.xPos, cmd.yPos);
        Odometry::getInstance().setOrientation(cmd.orientation);
        break;

    default:
        /* Command not known/relevant to the application. */
        rsp.responseId = SMPChannelPayload::RSP_ID_ERROR;
        break;
    }

    /* Ignoring return value, as error handling is not available. */
    (void)m_smpServer.sendData(m_serialMuxProtChannelIdRemoteCtrlRsp, &rsp, sizeof(rsp));
}

void App::systemStatusCallback(SMPChannelPayload::Status status)
{
    switch (status)
    {
    case SMPChannelPayload::STATUS_FLAG_OK:
        /* Nothing to do. All good. */
        break;

    case SMPChannelPayload::STATUS_FLAG_ERROR:
        ErrorState::getInstance().setErrorMsg("DCS_ERR");
        m_systemStateMachine.setState(&ErrorState::getInstance());
        m_statusTimeoutTimer.stop();
        break;

    default:
        break;
    }

    m_statusTimeoutTimer.start(STATUS_TIMEOUT_TIMER_INTERVAL);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::reportVehicleData()
{
    IProximitySensors& proximitySensors = Board::getInstance().getProximitySensors();
    Odometry&          odometry         = Odometry::getInstance();
    Speedometer&       speedometer      = Speedometer::getInstance();
    VehicleData        payload;
    int32_t            xPos          = 0;
    int32_t            yPos          = 0;
    uint8_t            maxCounts     = 0U;
    uint8_t            averageCounts = 0U;
    uint8_t            leftCounts    = 0U;
    uint8_t            rightCounts   = 0U;
    int16_t            leftSpeed     = speedometer.getLinearSpeedLeft();
    int16_t            rightSpeed    = speedometer.getLinearSpeedRight();
    int16_t            centerSpeed   = speedometer.getLinearSpeedCenter();

    proximitySensors.read();
    leftCounts  = proximitySensors.countsFrontWithLeftLeds();
    rightCounts = proximitySensors.countsFrontWithRightLeds();

    /* Use the sensor value with the maximum counts. */
    maxCounts     = leftCounts > rightCounts ? leftCounts : rightCounts;
    averageCounts = m_movAvgProximitySensor.write(maxCounts);

    odometry.getPosition(xPos, yPos);
    payload.xPos        = xPos;
    payload.yPos        = yPos;
    payload.orientation = odometry.getOrientation();
    payload.left        = Util::stepsPerSecondToMillimetersPerSecond(leftSpeed);
    payload.right       = Util::stepsPerSecondToMillimetersPerSecond(rightSpeed);
    payload.center      = Util::stepsPerSecondToMillimetersPerSecond(centerSpeed);
    payload.proximity   = static_cast<SMPChannelPayload::Range>(averageCounts);

    /* Ignoring return value, as error handling is not available. */
    (void)m_smpServer.sendData(m_serialMuxProtChannelIdCurrentVehicleData, &payload, sizeof(VehicleData));
}

bool App::setupSerialMuxProt()
{
    bool isSuccessful = false;

    /* Channel subscription. */
    m_smpServer.subscribeToChannel(COMMAND_CHANNEL_NAME, App_cmdChannelCallback);
    m_smpServer.subscribeToChannel(MOTOR_SPEED_SETPOINT_CHANNEL_NAME, App_motorSpeedSetpointsChannelCallback);
    m_smpServer.subscribeToChannel(STATUS_CHANNEL_NAME, App_statusChannelCallback);
    m_smpServer.subscribeToChannel(ROBOT_SPEED_SETPOINT_CHANNEL_NAME, App_robotSpeedSetpointChannelCallback);

    /* Channel creation. */
    m_serialMuxProtChannelIdRemoteCtrlRsp =
        m_smpServer.createChannel(COMMAND_RESPONSE_CHANNEL_NAME, COMMAND_RESPONSE_CHANNEL_DLC);
    m_serialMuxProtChannelIdCurrentVehicleData =
        m_smpServer.createChannel(CURRENT_VEHICLE_DATA_CHANNEL_NAME, CURRENT_VEHICLE_DATA_CHANNEL_DLC);
    m_serialMuxProtChannelIdStatus      = m_smpServer.createChannel(STATUS_CHANNEL_NAME, STATUS_CHANNEL_DLC);
    m_serialMuxProtChannelIdLineSensors = m_smpServer.createChannel(LINE_SENSOR_CHANNEL_NAME, LINE_SENSOR_CHANNEL_DLC);

    /* Channels succesfully created? */
    if ((0U != m_serialMuxProtChannelIdCurrentVehicleData) && (0U != m_serialMuxProtChannelIdRemoteCtrlRsp) &&
        (0U != m_serialMuxProtChannelIdStatus) && (0U != m_serialMuxProtChannelIdLineSensors))
    {
        isSuccessful = true;
    }

    return isSuccessful;
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

    (void)m_smpServer.sendData(m_serialMuxProtChannelIdLineSensors, &payload, sizeof(payload));
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
    if ((nullptr != payload) && (MOTOR_SPEED_SETPOINT_CHANNEL_DLC == payloadSize))
    {
        const MotorSpeed* motorSpeedData  = reinterpret_cast<const MotorSpeed*>(payload);
        int16_t           leftMotorSpeed  = Util::millimetersPerSecondToStepsPerSecond(motorSpeedData->left);
        int16_t           rightMotorSpeed = Util::millimetersPerSecondToStepsPerSecond(motorSpeedData->right);
        DrivingState::getInstance().setMotorSpeeds(leftMotorSpeed, rightMotorSpeed);
    }
}

/**
 * Receives current status of the DCS over SerialMuxProt channel.
 *
 * @param[in] payload       Status of the DCS.
 * @param[in] payloadSize   Size of the Status Flag
 * @param[in] userData      Instance of App class.
 */
void App_statusChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    if ((nullptr != payload) && (STATUS_CHANNEL_DLC == payloadSize) && (nullptr != userData))
    {
        const Status* currentStatus = reinterpret_cast<const Status*>(payload);
        App*          application   = reinterpret_cast<App*>(userData);
        application->systemStatusCallback(currentStatus->status);
    }
}

/**
 * Receives robot speed setpoints over SerialMuxProt channel.
 *
 * @param[in] payload       Linear and angular speed setpoints in a RobotSpeed structure.
 * @param[in] payloadSize   Size of the RobotSpeed structure.
 * @param[in] userData      Instance of App class.
 */
void App_robotSpeedSetpointChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    (void)userData;
    if ((nullptr != payload) && (ROBOT_SPEED_SETPOINT_CHANNEL_DLC == payloadSize))
    {
        const RobotSpeed* robotSpeedData = reinterpret_cast<const RobotSpeed*>(payload);
        int16_t           angularSpeed   = static_cast<int16_t>(robotSpeedData->angular);

        /* Convert to [steps/s] */
        int16_t centerSpeed = Util::millimetersPerSecondToStepsPerSecond(robotSpeedData->linearCenter);

        /* Set the robot speeds. */
        DrivingState::getInstance().setRobotSpeeds(centerSpeed, angularSpeed);
    }
}