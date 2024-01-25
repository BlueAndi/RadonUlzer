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
#include <StateMachine.h>
#include <SimpleTimer.h>
#include <SerialMuxProtServer.hpp>
#include "SerialMuxChannels.h"
#include "RemoteCtrlState.h"
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The convoy leader application. */
class App
{
public:
    /**
     * Construct the convoy leader application.
     */
    App() :
        m_serialMuxProtChannelIdRemoteCtrlRsp(0U),
        m_serialMuxProtChannelIdCurrentVehicleData(0U),
        m_systemStateMachine(),
        m_controlInterval(),
        m_reportTimer(),
        m_smpServer(Serial),
        m_lastRemoteControlRspId{RemoteCtrlState::CMD_ID_IDLE, RemoteCtrlState::RSP_ID_OK, 0}
    {
    }

    /**
     * Destroy the convoy leader application.
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

private:
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Current data reporting period in ms. */
    static const uint32_t REPORTING_PERIOD = 50U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;

    /** SerialMuxProt Channel id for sending remote control command responses. */
    uint8_t m_serialMuxProtChannelIdRemoteCtrlRsp;

    /** SerialMuxProt Channel id for sending the current vehicle data. */
    uint8_t m_serialMuxProtChannelIdCurrentVehicleData;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /** Timer for reporting current data through SerialMuxProt. */
    SimpleTimer m_reportTimer;

    /**
     * SerialMuxProt Server Instance
     */
    SMPServer m_smpServer;

    /** Last remote control response id */
    CommandResponse m_lastRemoteControlRspId;

    /**
     * Report the current vehicle data.
     * Report the current position and heading of the robot using the Odometry data.
     * Report the current motor speeds of the robot using the Speedometer data.
     * Sends data through the SerialMuxProtServer.
     */
    void reportVehicleData();

    /**
     * Send remote control command responses on change.
     */
    void sendRemoteControlResponses();

    /**
     * Setup the SerialMuxProt channels.
     *
     * @return If successful returns true, otherwise false.
     */
    bool setupSerialMuxProt();

    /* Not allowed. */
    App(const App& app);            /**< Copy construction of an instance. */
    App& operator=(const App& app); /**< Assignment of an instance. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */
