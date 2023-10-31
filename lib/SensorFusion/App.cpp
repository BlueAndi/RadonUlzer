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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "App.h"
#include "StartupState.h"
#include <Board.h>
#include <Speedometer.h>
#include <DifferentialDrive.h>
#include <Odometry.h>
#include <Util.h>
#include "SerialMuxChannels.h"

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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void App::setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    Board::getInstance().init();
    m_systemStateMachine.setState(&StartupState::getInstance());
    m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);

    m_sendSensorsDataInterval.start(SEND_SENSORS_DATA_PERIOD);

    /* Providing Sensor data */
    m_smpChannelIdSensorData = m_smpServer.createChannel(SENSORDATA_CHANNEL_NAME, SENSORDATA_CHANNEL_DLC);
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

    IIMU& imu = Board::getInstance().getIMU();
    /* Send sensor data periodically. */
    if ((true == m_sendSensorsDataInterval.isTimeout()) && (true == imu.accDataReady()) &&
        (true == imu.gyroDataReady()) && (true == imu.magDataReady()))
    {
        m_sendSensorsDataInterval.restart();
        sendSensorData();
    }
    m_systemStateMachine.process();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::sendSensorData() const
{
    SensorData payload;
    IIMU&      imu      = Board::getInstance().getIMU();
    Odometry&  odometry = Odometry::getInstance();
    int32_t    positionXOdometry;
    int32_t    positionYOdometry;

    /* Get the current values from the Odometry. */
    odometry.getPosition(positionXOdometry, positionYOdometry);
    payload.positionXOdometry   = positionXOdometry;
    payload.positionYOdometry   = positionYOdometry;
    payload.orientationOdometry = odometry.getOrientation();

    /* Get the current values from the IMU.  */
    int16_t accelerationValues[3];
    int16_t turnRates[3];
    int16_t magnetometerValues[3];

    /* Read the accelerometer. */
    imu.readAcc();
    imu.getAccelerationValues(accelerationValues);
    payload.accelerationX   = accelerationValues[AXIS_INDEX_X];
    payload.accelerationY   = accelerationValues[AXIS_INDEX_Y];

    /* Read the gyro. */
    imu.readGyro();
    imu.getTurnRates(turnRates);
    payload.turnRate   = turnRates[AXIS_INDEX_Z];

    /* Read the magnetometer. */
    imu.readMag();
    imu.getMagnetometerValues(magnetometerValues);
    payload.magnetometerValueX   = magnetometerValues[AXIS_INDEX_X];
    payload.magnetometerValueY   = magnetometerValues[AXIS_INDEX_Y];

    (void)m_smpServer.sendData(m_smpChannelIdSensorData, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/