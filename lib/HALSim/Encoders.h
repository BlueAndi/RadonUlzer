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
 * @brief  Encoders realization
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALSim
 *
 * @{
 */

#ifndef ENCODERS_H
#define ENCODERS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IEncoders.h"
#include "SimTime.h"

#include <webots/PositionSensor.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the Zumo target encoders. */
class Encoders : public IEncoders
{
public:
    /**
     * Constructs the encoders adapter.
     *
     * @param[in] posSensorLeft         The left position sensor
     * @param[in] posSensorRight        The right position sensor
     */
    Encoders(webots::PositionSensor* posSensorLeft, webots::PositionSensor* posSensorRight) :
        IEncoders(),
        m_posSensorLeft(posSensorLeft),
        m_posSensorRight(posSensorRight),
        m_lastResetValueLeft(0.0f),
        m_lastResetValueRight(0.0f)
    {
    }

    /**
     * Destroys the encoders adapter.
     */
    ~Encoders()
    {
    }

    /**
     * Initialize or re-initialize the encoders.
     * This is used e.g. to re-initialize the encoders after the simulation world
     * was reset, but the ext. robot is still active.
     */
    void init() final;

    /**
     * Returns the counts of the encoder of the left-side motor.
     * Internal calculation are done because the positionSensor
     * in webots that acts as an encoder only returns the driven
     * way in m/s and not the counted steps.
     *
     * @return Encoder steps left
     */
    int16_t getCountsLeft() final;

    /**
     * Returns the counts of the encoder of the right-side motor.
     * Internal calculation are done because the positionSensor
     * in webots that acts as an encoder only returns the driven
     * way in m/s and not the counted steps.
     *
     * @return Encoder steps right
     */
    int16_t getCountsRight() final;

    /**
     * This function is just like getCountsLeft() except it also clears the
     * counts before returning.  If you call this frequently enough, you will
     * not have to worry about the count overflowing.
     *
     * @return Encoder steps left
     */
    int16_t getCountsAndResetLeft() final;

    /**
     * This function is just like getCountsRight() except it also clears the
     * counts before returning.  If you call this frequently enough, you will
     * not have to worry about the count overflowing.
     *
     * @return Encoder steps right
     */
    int16_t getCountsAndResetRight() final;

private:
    /** The position sensor of the left motor in the robot simulation. */
    webots::PositionSensor* m_posSensorLeft;

    /** The position sensor of the right motor in the robot simulation. */
    webots::PositionSensor* m_posSensorRight;

    /** Last position value of the left sensor in [m], used as reference. */
    double m_lastResetValueLeft;

    /** Last position value of the right sensor in [m], used as reference. */
    double m_lastResetValueRight;

    /**
     * The position sensor provides a distance as double and in [m].
     * The target system provides the distance as int16_t and in [steps].
     * Calling this method will prevent that the difference between the
     * reference point and the current position will overflow the target
     * data type.
     *
     * @param[in,out]   lastPos   Last position in [m]
     * @param[in]       pos       Current position in [m]
     */
    void overflowProtection(double& lastPos, double pos);

    /**
     * Calculate the absolute number of encoder steps by position change.
     *
     * @param[in]   lastPos   Last position in [m]
     * @param[in]   pos       Current position in [m]
     *
     * @return Absolute number of encoder steps
     */
    int16_t calculateSteps(double lastPos, double pos) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ENCODERS_H */
/** @} */
