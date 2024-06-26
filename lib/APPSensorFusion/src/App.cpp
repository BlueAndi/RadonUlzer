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
#include <Logging.h>

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
    Logging::disable();

    /* Initialize HAL */
    Board::getInstance().init();

    /* Setup the state machine with the first state. */
    m_systemStateMachine.setState(&StartupState::getInstance());

    /* Setup the periodically processing of robot control.  */
    m_controlInterval.start(DIFFERENTIAL_DRIVE_CONTROL_PERIOD);

    /* Periodically send Send Data via SerialMuxProt. */
    m_sendSensorDataInterval.start(SEND_SENSOR_DATA_PERIOD);

    /* Providing Sensor data */
    m_smpChannelIdSensorData = m_smpServer.createChannel(SENSORDATA_CHANNEL_NAME, SENSORDATA_CHANNEL_DLC);
}

void App::loop()
{
    Board::getInstance().process();
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

        /* Read the IMU so when the Measurement Timer runs out the Sensor Data can be accessed directly without having
         * to wait for the reading. */
        IIMU& imu = Board::getInstance().getIMU();
        imu.readGyro();
        imu.readAccelerometer();

        m_controlInterval.restart();
    }

    /* Send sensor data periodically if new data is available. */
    if (true == m_sendSensorDataInterval.isTimeout())
    {
        m_sendSensorDataInterval.restart();

        /* Send Sensor Data if the application is currently in the Driving state. */
        if (&DrivingState::getInstance() == m_systemStateMachine.getState())
        {
            sendSensorData();
        }
    }

    m_systemStateMachine.process();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void App::sendSensorData()
{
    SensorData payload;
    IIMU&      imu      = Board::getInstance().getIMU();
    Odometry&  odometry = Odometry::getInstance();
    int32_t    positionOdometryX;
    int32_t    positionOdometryY;

    /* Get the current values from the Odometry. */
    odometry.getPosition(positionOdometryX, positionOdometryY);

    /* Access the Accelerometer Data (the Accelerometer is read out during the Control Interval Timeout). */
    IMUData accelerationValues;
    imu.getAccelerationValues(&accelerationValues);

    /* Access the Gyro Data (the Gyro is read out during the Control Interval Timeout). */
    IMUData turnRates;
    imu.getTurnRates(&turnRates);

    /* Write the sensor data in the SensorData Struct. */
    payload.positionOdometryX   = positionOdometryX;
    payload.positionOdometryY   = positionOdometryY;
    payload.orientationOdometry = odometry.getOrientation();
    payload.accelerationX       = accelerationValues.valueX;
    payload.turnRate            = turnRates.valueZ;
    payload.timePeriod          = static_cast<uint16_t>(SEND_SENSOR_DATA_PERIOD);

    /* Send the sensor data via the SerialMuxProt. */
    (void)m_smpServer.sendData(m_smpChannelIdSensorData, reinterpret_cast<uint8_t*>(&payload), sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/