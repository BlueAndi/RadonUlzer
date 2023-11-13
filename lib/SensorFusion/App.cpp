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
    IIMU& imu = Board::getInstance().getIMU();

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

    /* Send sensor data periodically if new data is available. */
    if ((true == m_sendSensorsDataInterval.isTimeout()) && (true == imu.accelerometerDataReady()) &&
        (true == imu.gyroDataReady()) && (true == imu.magnetometerDataReady()))
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
    int32_t    positionOdometryX;
    int32_t    positionOdometryY;

    /* Get the current values from the Odometry. */
    odometry.getPosition(positionOdometryX, positionOdometryY);

    /* Read the accelerometer. */
    IMUData accelerationValues;
    imu.readAccelerometer();
    imu.getAccelerationValues(&accelerationValues);

    /* Read the magnetometer. */
    IMUData magnetometerValues;
    imu.readMagnetometer();
    imu.getMagnetometerValues(&magnetometerValues);

    /* Read the gyro. */
    IMUData turnRates;
    imu.readGyro();
    imu.getTurnRates(&turnRates);

    /* Write the sensor data in the SensorData Struct. */
    payload.positionOdometryX   = positionOdometryX;
    payload.positionOdometryY   = positionOdometryY;
    payload.orientationOdometry = odometry.getOrientation();
    payload.accelerationX       = accelerationValues.valueX;
    payload.accelerationY       = accelerationValues.valueY;
    payload.magnetometerValueX  = magnetometerValues.valueX;
    payload.magnetometerValueY  = magnetometerValues.valueY;
    payload.turnRate            = turnRates.valueZ;

    /* Send the sensor data via the SerialMuxProt. */
    (void)m_smpServer.sendData(m_smpChannelIdSensorData, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/