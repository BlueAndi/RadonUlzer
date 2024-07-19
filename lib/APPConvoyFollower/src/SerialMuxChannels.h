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
#include <SerialMuxProtServer.hpp>

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

/** Name of Channel to send Current Vehicle Data to. */
#define CURRENT_VEHICLE_DATA_CHANNEL_NAME "CURR_DATA"

/** DLC of Current Vehicle Data Channel */
#define CURRENT_VEHICLE_DATA_CHANNEL_DLC (sizeof(VehicleData))

/** Name of Channel to send system status to. */
#define STATUS_CHANNEL_NAME "STATUS"

/** DLC of Status Channel */
#define STATUS_CHANNEL_DLC (sizeof(Status))

/** Name of the Channel to receive Line Sensor Data from. */
#define LINE_SENSOR_CHANNEL_NAME "LINE_SENS"

/** DLC of Line Sensor Channel */
#define LINE_SENSOR_CHANNEL_DLC (sizeof(LineSensorData))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** SerialMuxProt Server with fixed template argument. */
typedef SerialMuxProtServer<MAX_CHANNELS> SMPServer;

/** Channel payload constants. */
namespace SMPChannelPayload
{
    /** Remote control commands. */
    typedef enum : uint8_t
    {
        CMD_ID_IDLE = 0,                /**< Nothing to do. */
        CMD_ID_START_LINE_SENSOR_CALIB, /**< Start line sensor calibration. */
        CMD_ID_START_MOTOR_SPEED_CALIB, /**< Start motor speed calibration. */
        CMD_ID_REINIT_BOARD,            /**< Re-initialize the board. Required for webots simulation. */
        CMD_ID_GET_MAX_SPEED,           /**< Get maximum speed. */
        CMD_ID_START_DRIVING,           /**< Start driving. */
        CMD_ID_SET_INIT_POS             /**< Set initial position. */

    } CmdId; /**< Command ID */

    /** Remote control command responses. */
    typedef enum : uint8_t
    {
        RSP_ID_OK = 0,  /**< Command successful executed. */
        RSP_ID_PENDING, /**< Command is pending. */
        RSP_ID_ERROR    /**< Command failed. */

    } RspId; /**< Response ID */

    /** Status flags. */
    typedef enum : uint8_t
    {
        STATUS_FLAG_OK = 0, /**< Everything is fine. */
        STATUS_FLAG_ERROR   /**< Something is wrong. */

    } Status; /**< Status flag */

    /**
     * Range in which a detected object may be.
     * Equivalent to the brightness levels.
     * Values estimated from user's guide.
     */
    typedef enum : uint8_t
    {
        RANGE_NO_OBJECT = 0U, /**< No object detected */
        RANGE_25_30,          /**< Object detected in range 25 to 30 cm */
        RANGE_20_25,          /**< Object detected in range 20 to 25 cm */
        RANGE_15_20,          /**< Object detected in range 15 to 20 cm */
        RANGE_10_15,          /**< Object detected in range 10 to 15 cm */
        RANGE_5_10,           /**< Object detected in range 5 to 10 cm */
        RANGE_0_5             /**< Object detected in range 0 to 5 cm */

    } Range; /**< Proximity Sensor Ranges */

} /* namespace SMPChannelPayload */

/** Struct of the "Command" channel payload. */
typedef struct _Command
{
    SMPChannelPayload::CmdId commandId; /**< Command ID */

    /** Command payload. */
    union
    {
        /** Init data command payload. */
        struct
        {
            int32_t xPos;        /**< X position [mm]. */
            int32_t yPos;        /**< Y position [mm]. */
            int32_t orientation; /**< Orientation [mrad]. */
        };
    };

} __attribute__((packed)) Command;

/** Struct of the "Command Response" channel payload. */
typedef struct _CommandResponse
{
    SMPChannelPayload::CmdId commandId;  /**< Command ID */
    SMPChannelPayload::RspId responseId; /**< Response to the command */

    /** Response Payload. */
    union
    {
        int16_t maxMotorSpeed; /**< Max speed [steps/s]. */
    };
} __attribute__((packed)) CommandResponse;

/** Struct of the "Speed" channel payload. */
typedef struct _SpeedData
{
    int16_t left;   /**< Left motor speed [steps/s] */
    int16_t right;  /**< Right motor speed [steps/s] */
    int16_t center; /**< Center motor speed [steps/s] */
} __attribute__((packed)) SpeedData;

/** Struct of the "Current Vehicle Data" channel payload. */
typedef struct _VehicleData
{
    int32_t                  xPos;        /**< X position [mm]. */
    int32_t                  yPos;        /**< Y position [mm]. */
    int32_t                  orientation; /**< Orientation [mrad]. */
    int16_t                  left;        /**< Left motor speed [steps/s]. */
    int16_t                  right;       /**< Right motor speed [steps/s]. */
    int16_t                  center;      /**< Center speed [steps/s]. */
    SMPChannelPayload::Range proximity;   /**< Range at which object is found [range]. */
} __attribute__((packed)) VehicleData;

/** Struct of the "Status" channel payload. */
typedef struct _Status
{
    SMPChannelPayload::Status status; /**< Status */
} __attribute__((packed)) Status;

/** Struct of the "Line Sensor" channel payload. */
typedef struct _LineSensorData
{
    uint16_t lineSensorData[5U]; /**< Line sensor data [digits] normalized to max 1000 digits. */
} __attribute__((packed)) LineSensorData;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SERIAL_MUX_CHANNELS_H_ */