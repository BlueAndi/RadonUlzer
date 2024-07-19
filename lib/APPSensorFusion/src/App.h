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
 * @brief  SensorFusion application
 * @author Juliane Kerpe <juliane.kerpe@web.de>
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
#include <Arduino.h>
#include <SerialMuxProtServer.hpp>
#include "SerialMuxChannels.h"
#include "DrivingState.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The SensorFusion application. */
class App
{
public:
    /**
     * Construct the SensorFusion application.
     */
    App() :
        m_smpChannelIdSensorData(0U),
        m_systemStateMachine(),
        m_controlInterval(),
        m_sendSensorDataInterval(),
        m_smpServer(Serial)
    {
    }

    /**
     * Destroy the SensorFusion application.
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
    /** Sending Data period in ms. */
    static const uint32_t SEND_SENSOR_DATA_PERIOD = 25U;

    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;

    /** Channel id for sending sensor data used for sensor fusion. */
    uint8_t m_smpChannelIdSensorData;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /** Timer used for sending data periodically. */
    SimpleTimer m_sendSensorDataInterval;

    /**
     * SerialMuxProt Server Instance
     *
     * @tparam tMaxChannels set to MAX_CHANNELS, defined in SerialMuxChannels.h.
     */
    SerialMuxProtServer<MAX_CHANNELS> m_smpServer;

    /**
     * Send the Sensor data as a SensorData struct via SerialMuxProt.
     */
    void sendSensorData();

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
