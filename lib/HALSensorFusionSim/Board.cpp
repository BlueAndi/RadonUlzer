/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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

/* Name of the speaker in the robot simulation. */
const char* Board::SPEAKER_NAME = "speaker";

/* Name of the left motor in the robot simulation. */
const char* Board::LEFT_MOTOR_NAME = "motor_left";

/* Name of the right motor in the robot simulation. */
const char* Board::RIGHT_MOTOR_NAME = "motor_right";

/* Name of the infrared emitter 0 in the robot simulation. */
const char* Board::EMITTER_0_NAME = "emitter_l";

/* Name of the infrared emitter 1 in the robot simulation. */
const char* Board::EMITTER_1_NAME = "emitter_lm";

/* Name of the infrared emitter 2 in the robot simulation. */
const char* Board::EMITTER_2_NAME = "emitter_m";

/* Name of the infrared emitter 3 in the robot simulation. */
const char* Board::EMITTER_3_NAME = "emitter_rm";

/* Name of the infrared emitter 4 in the robot simulation. */
const char* Board::EMITTER_4_NAME = "emitter_r";

/** Name of the position sensor of the left motor in the robot simulation. */
const char* Board::POS_SENSOR_LEFT_NAME = "position_sensor_left";

/** Name of the position sensor of the right motor in the robot simulation. */
const char* Board::POS_SENSOR_RIGHT_NAME = "position_sensor_right";

/* Name of the light sensor 0 in the robot simulation. */
const char* Board::LIGHT_SENSOR_0_NAME = "lightsensor_l";

/* Name of the light sensor 1 in the robot simulation. */
const char* Board::LIGHT_SENSOR_1_NAME = "lightsensor_lm";

/* Name of the light sensor 2 in the robot simulation. */
const char* Board::LIGHT_SENSOR_2_NAME = "lightsensor_m";

/* Name of the light sensor 3 in the robot simulation. */
const char* Board::LIGHT_SENSOR_3_NAME = "lightsensor_rm";

/* Name of the light sensor 4 in the robot simulation. */
const char* Board::LIGHT_SENSOR_4_NAME = "lightsensor_r";

/* Name of the yellow LED in the robot simulation. */
const char* Board::LED_YELLOW_NAME = "led_yellow";

/* Name of the accelerometer in the robot simulation. */
const char* Board::ACCELEROMETER_NAME = "accelerometer";

/* Name of the gyro in the robot simulation. */
const char* Board::GYRO_NAME = "gyro";

/* Name of the magnetometer in the robot simulation. */
const char* Board::MAGNETOMETER_NAME = "magnetometer";

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
    m_buzzer(m_robot.getSpeaker(SPEAKER_NAME)),
    m_encoders(m_robot.getPositionSensor(POS_SENSOR_LEFT_NAME), m_robot.getPositionSensor(POS_SENSOR_RIGHT_NAME)),
    m_lineSensors(m_robot.getEmitter(EMITTER_0_NAME), m_robot.getEmitter(EMITTER_1_NAME),
                  m_robot.getEmitter(EMITTER_2_NAME), m_robot.getEmitter(EMITTER_3_NAME),
                  m_robot.getEmitter(EMITTER_4_NAME), m_robot.getDistanceSensor(LIGHT_SENSOR_0_NAME),
                  m_robot.getDistanceSensor(LIGHT_SENSOR_1_NAME), m_robot.getDistanceSensor(LIGHT_SENSOR_2_NAME),
                  m_robot.getDistanceSensor(LIGHT_SENSOR_3_NAME), m_robot.getDistanceSensor(LIGHT_SENSOR_4_NAME)),
    m_motors(m_robot.getMotor(LEFT_MOTOR_NAME), m_robot.getMotor(RIGHT_MOTOR_NAME)),
    m_ledYellow(m_robot.getLED(LED_YELLOW_NAME)),
    m_imu(m_robot.getAccelerometer(ACCELEROMETER_NAME), m_robot.getGyro(GYRO_NAME),
          m_robot.getCompass(MAGNETOMETER_NAME)),
    m_settings()
{
}

void Board::enableSimulationDevices()
{
    const int timeStep = m_simTime.getTimeStep();

    m_robot.getKeyboard()->enable(timeStep);
    m_robot.getPositionSensor(POS_SENSOR_LEFT_NAME)->enable(timeStep);
    m_robot.getPositionSensor(POS_SENSOR_RIGHT_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(LIGHT_SENSOR_0_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(LIGHT_SENSOR_1_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(LIGHT_SENSOR_2_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(LIGHT_SENSOR_3_NAME)->enable(timeStep);
    m_robot.getDistanceSensor(LIGHT_SENSOR_4_NAME)->enable(timeStep);
    m_robot.getAccelerometer(ACCELEROMETER_NAME)->enable(timeStep);
    m_robot.getGyro(GYRO_NAME)->enable(timeStep);
    m_robot.getCompass(MAGNETOMETER_NAME)->enable(timeStep);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
