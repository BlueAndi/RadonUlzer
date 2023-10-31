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
    App() : m_smpChannelIdSensorData(1U),
            m_systemStateMachine(),
            m_controlInterval(),
            m_sendSensorsDataInterval(),
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
    /** Indices of axis x, y and z. */
    static const uint8_t AXIS_INDEX_X = 0U;
    static const uint8_t AXIS_INDEX_Y = 1U;
    static const uint8_t AXIS_INDEX_Z = 2U;

    /** Sending Data period in ms. */
    static const uint32_t SEND_SENSORS_DATA_PERIOD = 20;
    
    /** Differential drive control period in ms. */
    static const uint32_t DIFFERENTIAL_DRIVE_CONTROL_PERIOD = 5U;

    /** Baudrate for Serial Communication */
    static const uint32_t SERIAL_BAUDRATE = 115200U;
         
    /** Channel id sending sensor data used for sensor fusion. */
    uint8_t m_smpChannelIdSensorData;

    /** The system state machine. */
    StateMachine m_systemStateMachine;

    /** Timer used for differential drive control processing. */
    SimpleTimer m_controlInterval;

    /** Timer used for sending data periodically. */
    SimpleTimer m_sendSensorsDataInterval;

    /**
     * SerialMuxProt Server Instance
     *
     * @tparam tMaxChannels set to 10, as App does not require
     * more channels for external communication.
     */
    SerialMuxProtServer<10U> m_smpServer;
    
     /**
     * Send the following Sensor data via SerialMuxProt:
     * Acceleration in X
     * Acceleration in Y
     * TurnRate around Z
     * Magnetometer value in X 
     * Magnetometer value in Y 
     * Angle calculated by Odometry
     * Position in X calculated by Odometry
     * Position in Y calculated by Odometry
     */
    void sendSensorData() const;

    App(const App& app);
    App& operator=(const App& app);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* APP_H */
/** @} */
