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
 * @brief  RemoteControl application
 * @author Andreas Merkle <web@blue-andi.de>
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
#include <MovAvg.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The remote control application. */
class App
{
public:
    /**
     * Construct the remote control application.
     */
    App() :
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
        m_movAvgProximitySensor()
    {
    }

    /**
     * Destroy the remote control application.
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
     * Handle remote command received via SerialMuxProt.
     *
     * @param[in] cmd Command to handle.
     */
    void handleRemoteCommand(const Command& cmd);

    /**
     * System Status callback.
     *
     * @param[in] status    System status
     */
    void systemStatusCallback(SMPChannelPayload::Status status);

private:
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Current data reporting period in ms. */
    static const uint32_t REPORTING_PERIOD = 50U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;

    /** Send status timer interval in ms. */
    static const uint32_t SEND_STATUS_TIMER_INTERVAL = 1000U;

    /** Status timeout timer interval in ms. */
    static const uint32_t STATUS_TIMEOUT_TIMER_INTERVAL = 2U * SEND_STATUS_TIMER_INTERVAL;

    /** Sending Data period in ms. */
    static const uint32_t SEND_LINE_SENSORS_DATA_PERIOD = 20U;

    /**
     * Number of measurements for proximity sensors moving average filter.
     */
    static const uint8_t MOVAVG_PROXIMITY_SENSOR_NUM_MEASUREMENTS = 3U;

    /** SerialMuxProt Channel id for sending remote control command responses. */
    uint8_t m_serialMuxProtChannelIdRemoteCtrlRsp;

    /** SerialMuxProt Channel id for sending the current vehicle data. */
    uint8_t m_serialMuxProtChannelIdCurrentVehicleData;

    /** SerialMuxProt Channel id for sending system status. */
    uint8_t m_serialMuxProtChannelIdStatus;

    /** SerialMuxProt Channel id for sending line sensors data. */
    uint8_t m_serialMuxProtChannelIdLineSensors;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /** Timer for reporting current data through SerialMuxProt. */
    SimpleTimer m_reportTimer;

    /** Timer for sending system status to DCS. */
    SimpleTimer m_statusTimer;

    /** Timer for timeout of system status of DCS. */
    SimpleTimer m_statusTimeoutTimer;

    /** Timer used for sending data periodically. */
    SimpleTimer m_sendLineSensorsDataInterval;

    /** SerialMuxProt Server Instance. */
    SMPServer m_smpServer;

    /**
     * Moving average filter for proximity sensors.
     */
    MovAvg<uint8_t, MOVAVG_PROXIMITY_SENSOR_NUM_MEASUREMENTS> m_movAvgProximitySensor;

    /**
     * Report the current vehicle data.
     * Report the current position and heading of the robot using the Odometry data.
     * Report the current motor speeds of the robot using the Speedometer data.
     * Sends data through the SerialMuxProtServer.
     */
    void reportVehicleData();

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

    /* Not allowed. */
    App(const App& app);            /**< Copy construction of an instance. */
    App& operator=(const App& app); /**< Assignment of an instance. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */
