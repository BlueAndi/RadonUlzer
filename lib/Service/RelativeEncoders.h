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
 * @brief  Relative encoders
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef RELATIVE_ENCODERS_H
#define RELATIVE_ENCODERS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Board.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The relative encoders provides always the step difference between a reference
 * point (absolute number of encoder steps) to the current position (current
 * number of encoder steps).
 *
 * Additional it provides the direction of the movement.
 */
class RelativeEncoders
{
public:
    /**
     * Constructs relative encoders.
     * The reference point is set to 0 steps.
     *
     * @param[in] absEncoders   The absolute encoders interface.
     */
    RelativeEncoders(IEncoders& absEncoders) :
        m_absEncoders(absEncoders),
        m_referencePointLeft(m_absEncoders.getCountsLeft()),
        m_referencePointRight(m_absEncoders.getCountsRight()),
        m_lastRelEncoderStepsLeft(0),
        m_lastRelEncoderStepsRight(0)
    {
    }

    /**
     * Destroys the relative encoder instance.
     */
    ~RelativeEncoders()
    {
    }

    /**
     * Clear the relative encoder steps left and right by setting it to 0.
     */
    void clear();

    /**
     * Clear the relative encoder steps left by setting it to 0.
     */
    void clearLeft();

    /**
     * Clear the relative encoder steps right by setting it to 0.
     */
    void clearRight();

    /**
     * Get the relative number of encoder steps left, since last clearance.
     *
     * @return Delta encoder steps
     */
    int16_t getCountsLeft() const;

    /**
     * Get the relative number of encoder steps right, since last clearance.
     *
     * @return Delta encoder steps
     */
    int16_t getCountsRight() const;

    /**
     * Direction of movement.
     */
    enum Direction
    {
        DIRECTION_STOPPED = 0, /**< Stopped */
        DIRECTION_POSTIVE,     /**< Moving to positive direction */
        DIRECTION_NEGATIVE,    /**< Moving to negative direction */
    };

    /**
     * Get the direction of movement left.
     *
     * @return Direction
     */
    Direction getDirectionLeft();

    /**
     * Get the direction of movement right.
     *
     * @return Direction
     */
    Direction getDirectionRight();

private:
    /**
     *  Absolute encoders
     */
    IEncoders& m_absEncoders;

    /**
     * Last absolute number of encoder steps left, which is used as reference point.
     */
    int16_t m_referencePointLeft;

    /**
     * Last absolute number of encoder steps right, which is used as reference point.
     */
    int16_t m_referencePointRight;

    /**
     * Last calculated relative encoder steps left, used to determine movement
     * direction.
     */
    int16_t m_lastRelEncoderStepsLeft;

    /**
     * Last calculated relative encoder steps right, used to determine movement
     * direction.
     */
    int16_t m_lastRelEncoderStepsRight;

    /* Not allowed. */
    RelativeEncoders();
    RelativeEncoders(const RelativeEncoders& relEncoder);
    RelativeEncoders& operator=(const RelativeEncoders& relEncoder);

    /**
     * Calculate relative number of encoder steps between the reference point
     * and the current absolute number of encoder steps.
     * 
     * @param[in] absSteps  Absolute number of encoder steps
     * @param[in] refPoint  Reference point
     * 
     * @return Relative number of encoder steps
     */
    int16_t calculate(int16_t absSteps, int16_t refPoint) const;

    /**
     * Get the direction of movement.
     *
     * @return Direction
     */
    Direction getDirection(int16_t lastRelSteps, int16_t currentRelSteps) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ENCODERS_H */
/** @} */
