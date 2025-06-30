/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  The simulation robot board realization.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Board.h>
#include <RobotDeviceNames.h>

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

void Board::init()
{
    m_encoders.init();
    m_lineSensors.init();
    m_motors.init();
    /*  TODO: TD084	React if IMU initialization fails */
    (void)m_imu.init();
    m_imu.enableDefault();
    m_imu.configureForTurnSensing();
    m_imu.calibrate();
    m_settings.init();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

Board::Board() :
    m_robot(),
    m_simTime(m_robot),
    m_keyboard(m_simTime, m_robot.getKeyboard()),
    m_buttonA(m_keyboard),
    m_buzzer(m_robot.getSpeaker(RobotDeviceNames::SPEAKER_NAME)),
    m_encoders(m_robot.getPositionSensor(RobotDeviceNames::POS_SENSOR_LEFT_NAME),
               m_robot.getPositionSensor(RobotDeviceNames::POS_SENSOR_RIGHT_NAME)),
    m_lineSensors(
        m_robot.getEmitter(RobotDeviceNames::EMITTER_0_NAME), m_robot.getEmitter(RobotDeviceNames::EMITTER_1_NAME),
        m_robot.getEmitter(RobotDeviceNames::EMITTER_2_NAME), m_robot.getEmitter(RobotDeviceNames::EMITTER_3_NAME),
        m_robot.getEmitter(RobotDeviceNames::EMITTER_4_NAME),
        m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_0_NAME),
        m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_1_NAME),
        m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_2_NAME),
        m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_3_NAME),
        m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_4_NAME)),
    m_motors(m_robot.getMotor(RobotDeviceNames::LEFT_MOTOR_NAME), m_robot.getMotor(RobotDeviceNames::RIGHT_MOTOR_NAME)),
    m_ledYellow(m_robot.getLED(RobotDeviceNames::LED_YELLOW_NAME)),
    m_imu(m_robot.getAccelerometer(RobotDeviceNames::ACCELEROMETER_NAME), m_robot.getGyro(RobotDeviceNames::GYRO_NAME),
          m_robot.getCompass(RobotDeviceNames::MAGNETOMETER_NAME)),
    m_serialDrv(m_robot.getEmitter(RobotDeviceNames::EMITTER_NAME_SERIAL),
                m_robot.getReceiver(RobotDeviceNames::RECEIVER_NAME_SERIAL)),
    m_settings()
{
}

void Board::enableSimulationDevices()
{
    const int timeStep = m_simTime.getTimeStep();

    m_robot.getKeyboard()->enable(timeStep);
    m_robot.getPositionSensor(RobotDeviceNames::POS_SENSOR_LEFT_NAME)->enable(timeStep);
    m_robot.getPositionSensor(RobotDeviceNames::POS_SENSOR_RIGHT_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_0_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_1_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_2_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_3_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(RobotDeviceNames::LIGHT_SENSOR_4_NAME)->enable(timeStep);
    m_robot.getAccelerometer(RobotDeviceNames::ACCELEROMETER_NAME)->enable(timeStep);
    m_robot.getGyro(RobotDeviceNames::GYRO_NAME)->enable(timeStep);
    m_robot.getCompass(RobotDeviceNames::MAGNETOMETER_NAME)->enable(timeStep);
    m_robot.getReceiver(RobotDeviceNames::RECEIVER_NAME_SERIAL)->enable(timeStep);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
