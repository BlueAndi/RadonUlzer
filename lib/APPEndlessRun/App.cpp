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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "App.h"

#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
#include <Util.h>
#include <Logging.h>

#include "StartupState.h"
#include "ColorState.h"
#include "DrivingState.h"

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

static void App_trafficLightColorsCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void App::setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Logging::disable();
    Board::getInstance().init();

    /* Jump to StartupState. */
    m_systemStateMachine.setState(&StartupState::getInstance());

    m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);

    /* Create Tx SMP Coordinates channel. */
    m_serialMuxProtChannelIdOdometry = m_smpServer.createChannel(ODOMETRY_CHANNEL_NAME, ODOMETRY_CHANNEL_DLC);

    /* Subscribe to TrafficLight SMP channel. */
    m_smpServer.subscribeToChannel(TRAFFIC_LIGHT_COLORS_NAME, App_trafficLightColorsCallback);

    /** Channel successfuly created? */
    if (0U != m_serialMuxProtChannelIdOdometry)
    {
        m_reportTimer.start(100U);
    }
    else
    {
        /** Could not create SMP channel. */
    }
    // m_reportTimer.start(150U);
}

void App::loop()
{
    m_smpServer.process(millis());
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

        m_controlInterval.restart();
    }

    /** Send only when driving. */
    if ((true == m_reportTimer.isTimeout()) && (&DrivingState::getInstance() == m_systemStateMachine.getState()))
    {
        /* Send current data to SerialMuxProt Client */
        sendCoordinates();

        m_reportTimer.restart();
    }

    m_systemStateMachine.process();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/**
 * Type: Tx SMP
 * Name: sendCoordinates
 *
 * Sends x and y position values over SerialMuxProt.
 *
 */
void App::sendCoordinates()
{
    Odometry&    odometry = Odometry::getInstance();
    OdometryData payload;
    int32_t      xPos = 0;
    int32_t      yPos = 0;

    odometry.getPosition(xPos, yPos);
    payload.xPos        = xPos;
    payload.yPos        = yPos;
    payload.orientation = odometry.getOrientation();

    /* Ignoring return value, as error handling is not available. */
    (void)m_smpServer.sendData(m_serialMuxProtChannelIdOdometry, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Type: Rx SMP
 * Name: TrafficLightColorsCallback
 *
 * Receives traffic light color values over SerialMuxProt channel.
 *
 * @param[in] payload Traffic light (integer) color value.
 * @param[in] payloadSize Size of color value.
 */
static void App_trafficLightColorsCallback(const uint8_t* payload, const uint8_t payloadSize, void* userData)
{
    (void)userData;
    if ((nullptr != payload) && (TRAFFIC_LIGHT_COLORS_DLC == payloadSize))
    {
        ColorState::TLCId tlcId = *reinterpret_cast<const ColorState::TLCId*>(payload);

        ColorState::getInstance().storeColorID(tlcId);
    }
}