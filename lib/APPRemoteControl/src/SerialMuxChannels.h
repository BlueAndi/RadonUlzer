/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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
#define MOTOR_SPEED_SETPOINT_CHANNEL_NAME "MOTOR_SET"

/** DLC of Motor Speed Setpoint Channel */
#define MOTOR_SPEED_SETPOINT_CHANNEL_DLC (sizeof(MotorSpeed))

/** Name of the Channel to send Robot Speed Setpoints to. */
#define ROBOT_SPEED_SETPOINT_CHANNEL_NAME "ROBOT_SET"

/** DLC of Robot Speed Setpoint Channel */
#define ROBOT_SPEED_SETPOINT_CHANNEL_DLC (sizeof(RobotSpeed))

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

/** Name of the Channel to receive Time Sync Request from the DCS. */
#define TIME_SYNC_REQUEST_CHANNEL_NAME "TIME_SYNC_REQ"

/** DLC of Time Sync Request Channel */
#define TIME_SYNC_REQUEST_CHANNEL_DLC (sizeof(TimeSyncRequest))

/** Name of the Channel to send Time Sync Response to the DCS. */
#define TIME_SYNC_RESPONSE_CHANNEL_NAME "TIME_SYNC_RSP"

/** DLC of Time Sync Response Channel */
#define TIME_SYNC_RESPONSE_CHANNEL_DLC (sizeof(TimeSyncResponse))

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
        int32_t maxMotorSpeed; /**< Max speed [mm/s]. */
    };
} __attribute__((packed)) CommandResponse;

/** Struct of the "Motor Speed Setpoints" channel payload. */
typedef struct _MotorSpeed
{
    int32_t left;  /**< Left motor speed [mm/s] */
    int32_t right; /**< Right motor speed [mm/s] */
} __attribute__((packed)) MotorSpeed;

/** Struct of the "Robot Speed Setpoints" channel payload. */
typedef struct _RobotSpeed
{
    int32_t linearCenter; /**< Linear speed of the vehicle center. [mm/s] */
    int32_t angular;      /**< Angular speed. [mrad/s] */
} __attribute__((packed)) RobotSpeed;

/** Struct of the "Current Vehicle Data" channel payload. */
typedef struct _VehicleData
{
    uint32_t                 timestamp;   /**< Timestamp [ms]. */
    int32_t                  xPos;        /**< X position [mm]. */
    int32_t                  yPos;        /**< Y position [mm]. */
    int32_t                  orientation; /**< Orientation [mrad]. */
    int32_t                  left;        /**< Left motor speed [mm/s]. */
    int32_t                  right;       /**< Right motor speed [mm/s]. */
    int32_t                  center;      /**< Center speed [mm/s]. */
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

/** Struct of the "Time Sync Request" channel payload. */
typedef struct _TimeSyncRequest
{
    uint32_t seq;  /**< Sequence number to match request/response. */
    uint32_t t1_ms;/**< Timestamp at sender when request left [ms]. */
} __attribute__((packed)) TimeSyncRequest;

/** Struct of the "Time Sync Response" channel payload. */
typedef struct _TimeSyncResponse
{
    uint32_t seq;   /**< Sequence number to match request/response. */
    uint32_t t1_ms; /**< Echo of request timestamp [ms]. */
    uint32_t t2_ms; /**< Timestamp at receiver when request arrived [ms]. */
    uint32_t t3_ms; /**< Timestamp at receiver when response sent [ms]. */
} __attribute__((packed)) TimeSyncResponse;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SERIAL_MUX_CHANNELS_H_ */
