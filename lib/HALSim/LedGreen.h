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
 * @brief  Green LED realization
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup HALSim
 *
 * @{
 */

#ifndef LEDGREEN_H
#define LEDGREEN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ILed.h"

#include <webots/LED.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the Zumo target green LED. */
class LedGreen : public ILed
{
public:
    /**
     * Constructs the green LED adapter.
     *
     * @param[in] led   The green LED of the simulated robot.
     */
    LedGreen(webots::LED* led) : 
        ILed(), 
        m_led(led)
    {
    }

    /**
     * Destroys the green LED adapter.
     */
    ~LedGreen()
    {
    }

    /**
     * Enables/Disables the LED.
     *
     * @param[in] enableIt  Enable LED with true, disable it with false.
     */
    void enable(bool enableIt) final;

private:
    /** The intensity of the monochromatic LED in the off state. */
    static const int COLOR_OFF = 0;

    /** The intensity of the monochromatic LED in the on state */
    static const int COLOR_ON = 255;

    webots::LED* m_led; /**< The LED on the simulated robot. */

    /* Default constructor not allowed. */
    LedGreen();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* LEDGREEN_H */
/** @} */
