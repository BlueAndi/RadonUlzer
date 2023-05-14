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
#include <YAPServer.hpp>

#include "RemoteCtrlState.h"

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
        m_systemStateMachine(),
        m_controlInterval(),
        m_yapServer(Serial),
        m_yapChannelIdRemoteCtrlRsp(0U),
        m_yapChannelIdLineSensors(0U),
        m_lastRemoteControlRspId(RemoteCtrlState::RSP_ID_OK)
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

private:
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5;

    /** YAP channel name for receiving commands. */
    static const char* CH_NAME_CMD;

    /** YAP channel name for sending command responses. */
    static const char* CH_NAME_RSP;

    /** YAP channel name for receiving motor sppeds. */
    static const char* CH_NAME_MOTOR_SPEEDS;

    /** YAP channel name for sending line sensors data. */
    static const char* CH_NAME_LINE_SENSORS;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /**
     * YAP Server Instance
     *
     * @tparam tMaxChannels set to 10, as App does not require
     * more channels for external communication.
     */
    YAPServer<10U> m_yapServer;

    /** Channel id sending remote control command responses. */
    uint8_t m_yapChannelIdRemoteCtrlRsp;

    /** Channel id sending line sensors data. */
    uint8_t m_yapChannelIdLineSensors;

    /** Last remote control response id */
    RemoteCtrlState::RspId m_lastRemoteControlRspId;

    App(const App& app);
    App& operator=(const App& app);

    /**
     * Send remote control command responses on change.
     */
    void sendRemoteControlResponses();

    /**
     * Send line sensors data via YAP.
     */
    void sendLineSensorsData() const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */
