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

/** The line follower with reinforcement learning application. */
class App
{
public:
    /**
     * Construct the line follower with reinforcement learning application.
     */
    App() :
        m_systemStateMachine(),
        m_controlInterval(),
        m_serialMuxProtChannelIdStatus(0U),
        m_serialMuxProtChannelIdLineSensors(0U),
        m_serialMuxProtChannelIdMode(0U),
        m_statusTimer(),
        m_sendLineSensorsDataInterval(),
        m_modeSelectionSent(false),
        m_smpServer(Serial, this)
    {
    }

    /**
     * Destroy the line follower with reinforcement learning application.
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

private:
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /** Timer for sending system status to DCS. */
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

    /** Ensue that the mode is only sent once */
    bool m_modeSelectionSent;

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

    /**
     * Copy construction of an instance.
     * Not allowed.
     *
     * @param[in] app Source instance.
     */
    App(const App& app);

    /**
     * Assignment of an instance.
     * Not allowed.
     *
     * @param[in] app Source instance.
     *
     * @returns Reference to App instance.
     */
    App& operator=(const App& app);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */
