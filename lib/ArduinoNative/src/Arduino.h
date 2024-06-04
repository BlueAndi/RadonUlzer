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
 * @brief  Arduino native
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup HAL
 *
 * @{
 */

#ifndef ARDUINO_H
#define ARDUINO_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

#define _USE_MATH_DEFINES

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include <math.h>
#include "Serial.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#ifndef PSTR
#define PSTR
#endif

#ifndef PROGMEM
#define PROGMEM
#endif

#define PI  M_PI

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This type defines the prototype to get the system tick counter in ms.
 */
typedef unsigned long (*GetSystemTick)();

/**
 * This type defines the prototype to delay for a specific time in ms.
 */
typedef void (*SystemDelay)(unsigned long ms);

/******************************************************************************
 * Functions
 *****************************************************************************/

/** Arduino internal specific functionality.*/
namespace Arduino
{
    /**
     * Setup the Arduino library and call the application setup() function.
     */
    void setup(GetSystemTick getSystemTickFunc, SystemDelay systemDelayFunc);

    /**
     * Loop shall be called periodically and handles Arduino library
     * functionality and call the application loop() function.
     */
    void loop();
}

/**
 * Returns the number of milliseconds passed since the system start.
 * 
 * @return The number of milliseconds.
 */
extern unsigned long millis();

/**
 * Delays the program for the specified amount of milliseconds. In the mean time the 
 * simulation still steps to prevent an endless loop.
 * 
 * @param[in] ms The amount of milliseconds that the program should be delayed by.
 */
extern void delay(unsigned long ms);

#endif  /* ARDUINO_H */