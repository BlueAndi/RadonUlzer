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
 * @brief  Abstract proximity sensors interface
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup HALInterfaces
 *
 * @{
 */
#ifndef IPROXIMITYSENSORS_H
#define IPROXIMITYSENSORS_H

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

/** The abstract proximity sensors interface. */
class IProximitySensors
{
public:
    /**
     * Destroys the interface.
     */
    virtual ~IProximitySensors()
    {
    }

    /**
     * Initialize only the front proximity sensor.
     */
    virtual void initFrontSensor() = 0;

    /**
     * Returns the number of sensors.
     * 
     * @return Number of sensors
     */
    virtual uint8_t getNumSensors() const = 0;

    /**
     * Emits IR pulses and gets readings from the sensors.
     */
    virtual void read() = 0;

    /**
     * Returns the number of brightness levels for the left LEDs that
     * activated the front proximity sensor.
     * 
     * @return Number of brightness levels 
     */
    virtual uint8_t countsFrontWithLeftLeds() const = 0;

    /**
     * Returns the number of brightness levels for the right LEDs that
     * activated the front proximity sensor.
     * 
     * @return Number of brightness levels 
     */
    virtual uint8_t countsFrontWithRightLeds() const = 0;

    /**
     * Returns the maximum number of brightness levels.
     * 
     * @return Number of brightness levels.
     */
    virtual uint8_t getNumBrightnessLevels() const = 0;

protected:
    /**
     * Constructs the interface.
     */
    IProximitySensors()
    {
    }

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IPROXIMITYSENSORS_H */