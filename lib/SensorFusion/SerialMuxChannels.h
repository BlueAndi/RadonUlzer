/* MIT License

Copyright (c) 2023 Andreas Merkle <web@blue-andi.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

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

/** Name of Channel to send Sensor Data to. */
#define SENSORDATA_CHANNEL_NAME "SENSOR_DATA"

/** DLC of Sensordata Channel */
#define SENSORDATA_CHANNEL_DLC (sizeof(SensorData))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Struct of the Sensor Data channel payload. */
typedef struct _SensorData
{
    int32_t positionXOdometry;
    int32_t positionYOdometry;
    int32_t orientationOdometry;
    int16_t accelerationX;
    int16_t accelerationY;
    int16_t magnetometerValueX;
    int16_t magnetometerValueY;
    int16_t turnRate;
} __attribute__((packed)) SensorData;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SERIAL_MUX_CHANNELS_H_ */