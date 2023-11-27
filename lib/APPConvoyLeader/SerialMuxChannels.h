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
 * @brief  Channel structure definition for the SerialMuxProt.
 * @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 *
 * @addtogroup Application
 *
 * @{
 */
#ifndef SERIAL_MUX_CHANNELS_H
#define SERIAL_MUX_CHANNELS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Maximum number of SerialMuxProt Channels. */
#define MAX_CHANNELS (10U)

/** Name of Channel to send Current Vehicle Data to. */
#define CURRENT_VEHICLE_DATA_CHANNEL_DLC_CHANNEL_NAME "CURR_DATA"

/** DLC of Current Vehicle Data Channel */
#define CURRENT_VEHICLE_DATA_CHANNEL_DLC (sizeof(VehicleData))

/** Name of Channel to send Motor Speed Setpoints to. */
#define SPEED_SETPOINT_CHANNEL_NAME "SPEED_SET"

/** DLC of Speedometer Channel */
#define SPEED_SETPOINT_CHANNEL_DLC (sizeof(SpeedData))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Struct of the "Current Vehicle Data" channel payload. */
typedef struct _VehicleData
{
    int32_t xPos;        /**< X position [mm]. */
    int32_t yPos;        /**< Y position [mm]. */
    int32_t orientation; /**< Orientation [mrad]. */
    int16_t left;        /**< Left motor speed [steps/s]. */
    int16_t right;       /**< Right motor speed [steps/s]. */
} __attribute__((packed)) VehicleData;

/** Struct of the "Speed" channel payload. */
typedef struct _SpeedData
{
    int16_t left;  /**< Left motor speed [steps/s]. */
    int16_t right; /**< Right motor speed [steps/s]. */
} __attribute__((packed)) SpeedData;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SERIAL_MUX_CHANNELS_H */
/** @} */
