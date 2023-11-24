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
 * @brief  LineFollower application
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
#include <Arduino.h>
#include <Board.h>

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
        m_serialMuxProtChannelIdOdometry(0U),
        m_systemStateMachine(),
        m_controlInterval(),
        m_reportTimer(),
        m_smpServer(Serial)
    {
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

private:
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;

    /** Default size of the JSON Document for parsing. */
    static const uint32_t JSON_DOC_DEFAULT_SIZE = 1024U;

    /** SerialMuxProt Channel id sending coordinates. */
    uint8_t m_serialMuxProtChannelIdOdometry;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /** Timer for reporting current data through SerialMuxProt. */
    SimpleTimer m_reportTimer;

    /**
     * SerialMuxProt Server Instance
     *
     * @tparam tMaxChannels set to 10, as App does not require
     * more channels for external communication.
     */
    SerialMuxProtServer<MAX_CHANNELS> m_smpServer;

    /* Not allowed. */
    App(const App& app);            /**< Copy construction of an instance. */
    App& operator=(const App& app); /**< Assignment of an instance. */

    /**
     * Send coordinates via SerialMuxProt.
     */
    void sendCoordinates();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */
