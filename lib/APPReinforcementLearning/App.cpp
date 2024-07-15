
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
#include <Util.h>
#include <Sound.h>
#include <Logging.h>
#include <WebotsSerialDrv.h>
#include <ReadyState.h>

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
        /* Setup the state machine with the first state. */
        m_systemStateMachine.setState(&StartupState::getInstance());
        m_statusTimer.start(SEND_STATUS_TIMER_INTERVAL);
        m_sendLineSensorsDataInterval.start(SEND_LINE_SENSORS_DATA_PERIOD);
        /* Setup the periodically processing of robot control.  */
        m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);
    }

#ifdef DEBUG_ODOMETRY
    /* Reset supervisor which set its observed position and orientation to 0. */
    Board::getInstance().getSender().send("RST");
#endif /* DEBUG_ODOMETRY */

    /* Surprise the audience. */
    Sound::playMelody(Sound::MELODY_WELCOME);
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

#ifdef DEBUG_ODOMETRY
        {
            Odometry&    odo         = Odometry::getInstance();
            int32_t      posX        = 0;
            int32_t      posY        = 0;
            int32_t      orientation = odo.getOrientation();
            const size_t BUFFER_SIZE = 128U;
            char         buffer[BUFFER_SIZE];

            odo.getPosition(posX, posY);

            snprintf(buffer, BUFFER_SIZE, "ODO,%d,%d,%d", posX, posY, orientation);

            Board::getInstance().getSender().send(buffer);
        }
#endif /* DEBUG_ODOMETRY */

        m_controlInterval.restart();
    }

    if ((true == m_statusTimer.isTimeout()) && (true == m_smpServer.isSynced())&& (&DrivingState::getInstance() == m_systemStateMachine.getState()))
    {
        Status payload = {SMPChannelPayload::Status::NOT_DONE};
        ILineSensors&   lineSensors      = Board::getInstance().getLineSensors();
        uint8_t         maxLineSensors   = lineSensors.getNumLineSensors();
        const uint16_t* lineSensorValues = lineSensors.getSensorValues();
        bool m_isTrackLost = DrivingState::getInstance().isNoLineDetected(lineSensorValues, maxLineSensors);
        if (DrivingState::getInstance().isAbortRequired(m_isTrackLost) == true)
        {
            payload = {SMPChannelPayload::Status::DONE};
            m_systemStateMachine.setState(&ReadyState::getInstance());
        }
        (void)m_smpServer.sendData(m_serialMuxProtChannelIdStatus, &payload, sizeof(payload));
        m_statusTimer.restart();
    }
    /* Send periodically line sensor data. */
    if (true == m_sendLineSensorsDataInterval.isTimeout() && (&DrivingState::getInstance() == m_systemStateMachine.getState()) )
    {
        sendLineSensorsData();

        m_sendLineSensorsDataInterval.restart();
    }

    if (&ReadyState::getInstance() == m_systemStateMachine.getState())
    {   
        Mode payload = {SMPChannelPayload::Mode::TRAINING_MODE};   
        if(ReadyState::getInstance().selectedMode() == 0)
        {
            payload = {SMPChannelPayload::Mode::DRIVING_MODE};
        }
        (void)m_smpServer.sendData(m_serialMuxProtChannelIdMode, &payload, sizeof(payload));
    }

    
    m_smpServer.process(millis());

    m_systemStateMachine.process();
}


/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool App::setupSerialMuxProt()
{
    bool isSuccessful = false;
    m_smpServer.subscribeToChannel(SPEED_SETPOINT_CHANNEL_NAME, App_motorSpeedSetpointsChannelCallback);
    /* Channel creation. */
    m_serialMuxProtChannelIdStatus = m_smpServer.createChannel(STATUS_CHANNEL_NAME, STATUS_CHANNEL_DLC);
    m_serialMuxProtChannelIdLineSensors = m_smpServer.createChannel(LINE_SENSOR_CHANNEL_NAME, LINE_SENSOR_CHANNEL_DLC);
    m_serialMuxProtChannelIdMode = m_smpServer.createChannel(MODE_CHANNEL_NAME, MODE_CHANNEL_DLC);
    

    /* Channels succesfully created? */
    if ((0U != m_serialMuxProtChannelIdStatus) && (0U != m_serialMuxProtChannelIdLineSensors) && (0U != m_serialMuxProtChannelIdMode))
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

void App_motorSpeedSetpointsChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{

    (void)userData;
    if ((nullptr != payload) && (SPEED_SETPOINT_CHANNEL_DLC == payloadSize))
    {
        StateMachine m_systemStateMachine;
        const SpeedData* motorSpeedData = reinterpret_cast<const SpeedData*>(payload);
        DrivingState::getInstance().setTargetSpeeds(motorSpeedData->left, motorSpeedData->right);
        if((motorSpeedData->left == 0) && (motorSpeedData->right == 0))
        {
            m_systemStateMachine.setState(&StartupState::getInstance());
            printf("StartupState");
        }
    }
}