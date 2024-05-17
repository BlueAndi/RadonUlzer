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
 * @brief  Robot device names in Webots
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALInterfaces
 *
 * @{
 */

#ifndef ROBOT_DEVICE_NAMES_H
#define ROBOT_DEVICE_NAMES_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Robot device names in Webots.
 */
namespace RobotDeviceNames
{
    /** Name of the speaker in the robot simulation. */
    static const char* SPEAKER_NAME = "speaker";

    /** Name of the display in the robot simulation. */
    static const char* DISPLAY_NAME = "robot_display";

    /** Name of the left motor in the robot simulation. */
    static const char* LEFT_MOTOR_NAME = "motor_left";

    /** Name of the right motor in the robot simulation. */
    static const char* RIGHT_MOTOR_NAME = "motor_right";

    /** Name of the infrared emitter 0 in the robot simulation. */
    static const char* EMITTER_0_NAME = "emitter_l";

    /** Name of the infrared emitter 1 in the robot simulation. */
    static const char* EMITTER_1_NAME = "emitter_lm";

    /** Name of the infrared emitter 2 in the robot simulation. */
    static const char* EMITTER_2_NAME = "emitter_m";

    /** Name of the infrared emitter 3 in the robot simulation. */
    static const char* EMITTER_3_NAME = "emitter_rm";

    /** Name of the infrared emitter 4 in the robot simulation. */
    static const char* EMITTER_4_NAME = "emitter_r";

    /** Name of the position sensor of the left motor in the robot simulation. */
    static const char* POS_SENSOR_LEFT_NAME = "position_sensor_left";

    /** Name of the position sensor of the right motor in the robot simulation. */
    static const char* POS_SENSOR_RIGHT_NAME = "position_sensor_right";

    /** Name of the light sensor 0 in the robot simulation. */
    static const char* LIGHT_SENSOR_0_NAME = "lightsensor_l";

    /** Name of the light sensor 1 in the robot simulation. */
    static const char* LIGHT_SENSOR_1_NAME = "lightsensor_lm";

    /** Name of the light sensor 2 in the robot simulation. */
    static const char* LIGHT_SENSOR_2_NAME = "lightsensor_m";

    /** Name of the light sensor 3 in the robot simulation. */
    static const char* LIGHT_SENSOR_3_NAME = "lightsensor_rm";

    /** Name of the light sensor 4 in the robot simulation. */
    static const char* LIGHT_SENSOR_4_NAME = "lightsensor_r";

    /** Name of the red LED in the robot simulation. */
    static const char* LED_RED_NAME = "led_red";

    /** Name of the yellow LED in the robot simulation. */
    static const char* LED_YELLOW_NAME = "led_yellow";

    /** Name of the green LED in the robot simulation. */
    static const char* LED_GREEN_NAME = "led_green";

    /** Name of the front left proximity sensor in the robot simulation. */
    static const char* PROXIMITY_SENSOR_FRONT_LEFT_NAME = "proxim_sensor_fl";

    /** Name of the front right proximity sensor in the robot simulation. */
    static const char* PROXIMITY_SENSOR_FRONT_RIGHT_NAME = "proxim_sensor_fr";

    /** Name of the accelerometer in the robot simulation. */
    static const char* ACCELEROMETER_NAME = "accelerometer";

    /** Name of the gyro in the robot simulation. */
    static const char* GYRO_NAME = "gyro";

    /** Name of the magnetometer in the robot simulation. */
    static const char* MAGNETOMETER_NAME = "magnetometer";

    /** Name of the serial emitter in the RadonUlzer simulation. */
    static const char* EMITTER_NAME_SERIAL = "serialComTx";

    /** Name of the serial receiver in the RadonUlzer simulation. */
    static const char* RECEIVER_NAME_SERIAL = "serialComRx";

}; /* namespace RobotDeviceNames */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ROBOT_DEVICE_NAMES_H */
/** @} */
