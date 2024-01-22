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
 *  @brief  Channel structure definition for the SerialMuxProt.
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

#ifndef SERIAL_MUX_CHANNELS_H_
#define SERIAL_MUX_CHANNELS_H_

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Maximum number of SerialMuxProt Channels. */
#define MAX_CHANNELS (10U)

/** Name of Channel to send Commands to. */
#define COMMAND_CHANNEL_NAME "CMD"

/** DLC of Command Channel. */
#define COMMAND_CHANNEL_DLC (sizeof(Command))

/** Name of Channel to receive Command Responses from. */
#define COMMAND_RESPONSE_CHANNEL_NAME "CMD_RSP"

/** DLC of Command Response Channel. */
#define COMMAND_RESPONSE_CHANNEL_DLC (sizeof(CommandResponse))

/** Name of Channel to send Motor Speed Setpoints to. */
#define SPEED_SETPOINT_CHANNEL_NAME "SPEED_SET"

/** DLC of Speedometer Channel */
#define SPEED_SETPOINT_CHANNEL_DLC (sizeof(SpeedData))

/** Name of the Channel to receive Line Sensor Data from. */
#define LINE_SENSOR_CHANNEL_NAME "LINE_SENS"

/** DLC of Line Sensor Channel */
#define LINE_SENSOR_CHANNEL_DLC (sizeof(LineSensorData))

/** Name of Channel to send Current Vehicle Data to. */
#define CURRENT_VEHICLE_DATA_CHANNEL_NAME "CURR_DATA"

/** DLC of Current Vehicle Data Channel */
#define CURRENT_VEHICLE_DATA_CHANNEL_DLC (sizeof(VehicleData))

/** Name of Channel to send Initial Vehicle Data to. */
#define INITIAL_VEHICLE_DATA_CHANNEL_NAME "INIT_DATA"

/** DLC of Initial Vehicle Data Channel */
#define INITIAL_VEHICLE_DATA_CHANNEL_DLC (sizeof(VehicleData))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Struct of the "Command" channel payload. */
typedef struct _Command
{
    uint8_t commandId; /**< Command ID */
} __attribute__((packed)) Command;

/** Struct of the "Command Response" channel payload. */
typedef struct _CommandResponse
{
    uint8_t commandId;  /**< Command ID */
    uint8_t responseId; /**< Response to the command */

    /** Response Payload. */
    union
    {
        int16_t maxMotorSpeed; /**< Max speed [steps/s]. */
    };
} __attribute__((packed)) CommandResponse;

/** Struct of the "Speed" channel payload. */
typedef struct _SpeedData
{
    int16_t left;  /**< Left motor speed [steps/s] */
    int16_t right; /**< Right motor speed [steps/s] */
} __attribute__((packed)) SpeedData;

/** Struct of the "Line Sensor" channel payload. */
typedef struct _LineSensorData
{
    uint16_t lineSensorData[5U]; /**< Line sensor data [digits] normalized to max 1000 digits. */
} __attribute__((packed)) LineSensorData;

/** Struct of the "Current Vehicle Data" channel payload. */
typedef struct _VehicleData
{
    int32_t xPos;        /**< X position [mm]. */
    int32_t yPos;        /**< Y position [mm]. */
    int32_t orientation; /**< Orientation [mrad]. */
    int16_t left;        /**< Left motor speed [steps/s]. */
    int16_t right;       /**< Right motor speed [steps/s]. */
    int16_t center;      /**< Center speed [steps/s]. */
} __attribute__((packed)) VehicleData;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SERIAL_MUX_CHANNELS_H_ */