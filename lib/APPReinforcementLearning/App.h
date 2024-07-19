
/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  LineFollower application
 * @author Akram Bziouech
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef APP_H
#define APP_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SerialMuxChannels.h"
#include <StateMachine.h>
#include <SimpleTimer.h>
#include <Arduino.h>


/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The line follower application. */
class App
{
public:
    /**
     * Construct the line follower application.
     */
    App() : 
        m_systemStateMachine(),
        m_controlInterval(),
        m_serialMuxProtChannelIdStatus(0U),
        m_serialMuxProtChannelIdLineSensors(0U),
        m_serialMuxProtChannelIdMode(0U),
        m_statusTimer(),
        m_sendLineSensorsDataInterval(),
        m_sentmode(false),
        m_smpServer(Serial, nullptr)
    {
        appInstance = this;
    }

    /**
     * Destroy the line follower application.
     */
    ~App()
    {
    }

    /**
     * Setup the application.
     */
    void setup();

    /**
     * Process the application periodically.
     */
    void loop();
    /**
     * System Status callback.
     *
     * @param[in] status    System status
     */
    void systemStatusCallback(SMPChannelPayload::Status status);
private:
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;

    /** Saving the APP instance */
    static App* appInstance;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /**
    * Timer for sending system status to DCS.
    */
    SimpleTimer m_statusTimer;

    /** Timer used for sending data periodically. */
    SimpleTimer m_sendLineSensorsDataInterval;

    /** Send status timer interval in ms. */
    static const uint32_t SEND_STATUS_TIMER_INTERVAL = 1000U;

    /** Sending Data period in ms. */
    static const uint32_t SEND_LINE_SENSORS_DATA_PERIOD = 20U;

    /** SerialMuxProt Channel id for sending system status. */
    uint8_t m_serialMuxProtChannelIdStatus;

    /** SerialMuxProt Channel id for sending LineSensors. */
    uint8_t m_serialMuxProtChannelIdLineSensors;

    /** SerialMuxProt Channel id for sending Mode Selected. */
    uint8_t m_serialMuxProtChannelIdMode;

    /** SerialMuxProt Server Instance. */
    SMPServer m_smpServer;

    /* Ensue that the mode is only sent once*/
    bool m_sentmode;
   
    /**
     * Setup the SerialMuxProt channels.
     *
     * @return If successful returns true, otherwise false.
     */
    bool setupSerialMuxProt();
    
    /**
     * Send line sensors data via SerialMuxProt.
    */
    void sendLineSensorsData() const;

    static void App_statusChannelCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);
    /* Not allowed. */
    App(const App& app);            /**< Copy construction of an instance. */
    App& operator=(const App& app); /**< Assignment of an instance. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */