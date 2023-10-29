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

/* Initialize channel name for sending sensor data. */
const char* App::CH_NAME_SENSORDATA = "SENSOR_DATA";

/* Number of sensors values to send via SerialMuxProt. */
const uint8_t NUMBER_OF_SENSOR_DATA = 8;

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
    uint8_t sensorDataChannelDlc = NUMBER_OF_SENSOR_DATA * sizeof(int16_t);
    
    /* Providing Sensor data */
    m_smpChannelIdSensorData = m_smpServer.createChannel(CH_NAME_SENSORDATA, sensorDataChannelDlc);
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

    /* Send sensor data periodically. */
    if (true == m_sendSensorsDataInterval.isTimeout())
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
    uint8_t         payload[NUMBER_OF_SENSOR_DATA * sizeof(int16_t)];
    IIMU& imu                          = Board::getInstance().getIMU();
    Odometry& odometry                 = Odometry::getInstance();    
    int32_t positionXOdometry;
    int32_t positionYOdometry;
    int32_t orientationOdometry;
    
    /* Get the current values from the Odometry. */
    odometry.getPosition(positionXOdometry, positionYOdometry);
    orientationOdometry = odometry.getOrientation();

    /* Cast the Odometry values from 32 to 16 bit integers (the range is enough since the robot won't drive further than +/-32 meters)  */
    int16_t positionXOdometryInt16      = static_cast<int16_t>(positionXOdometry);
    int16_t positionYOdometryInt16      = static_cast<int16_t>(positionYOdometry);
    int16_t orientationOdometryInt16    = static_cast<int16_t>(orientationOdometry);

    /* Get the current values from the IMU.  */
    int16_t accelerationValues[3];
    int16_t turnRates[3];
    int16_t magnetometerValues[3];
    
    /* Wait until new accelerometer data is available and then read the data. */
    while(!imu.accDataReady()) {}
    imu.readAcc();
    imu.getAccelerationValues(accelerationValues);

    /* Wait until new gyro data is available and then read the data. */
    while(!imu.gyroDataReady()){}
    imu.readGyro();
    imu.getTurnRates(turnRates);

    /* Wait until new magnetometer data is available and then read the data. */
    while(!imu.magDataReady()){}
    imu.readMag();
    imu.getMagnetometerValues(magnetometerValues);

    /**
     * Write the values into a Byte Array in this order:
     * Acceleration in X
     * Acceleration in Y
     * TurnRate around Z
     * Magnetometer value in X 
     * Magnetometer value in Y 
     * Angle calculated by Odometry
     * Position in X calculated by Odometry
     * Position in Y calculated by Odometry
     */
    Util::int16ToByteArray(&payload[0 * sizeof(int16_t)], sizeof(int16_t), accelerationValues[AXIS_INDEX_X]);      
    Util::int16ToByteArray(&payload[1 * sizeof(int16_t)], sizeof(int16_t), accelerationValues[AXIS_INDEX_Y]);
    Util::int16ToByteArray(&payload[2 * sizeof(int16_t)], sizeof(int16_t), turnRates[AXIS_INDEX_Z]);
    Util::int16ToByteArray(&payload[3 * sizeof(int16_t)], sizeof(int16_t), magnetometerValues[AXIS_INDEX_X]);
    Util::int16ToByteArray(&payload[4 * sizeof(int16_t)], sizeof(int16_t), magnetometerValues[AXIS_INDEX_Y]);
    Util::int16ToByteArray(&payload[5 * sizeof(int16_t)], sizeof(int16_t), orientationOdometryInt16);
    Util::int16ToByteArray(&payload[6 * sizeof(int16_t)], sizeof(int16_t), positionXOdometryInt16);
    Util::int16ToByteArray(&payload[7 * sizeof(int16_t)], sizeof(int16_t), positionYOdometryInt16);

    (void)m_smpServer.sendData(m_smpChannelIdSensorData, payload, sizeof(payload));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/