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
        m_referencePointLeft(0),
        m_referencePointRight(0)
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
     * Default constructor.
     * Not allowed.
     */
    RelativeEncoders();

    /**
     * Copy construction of an instance.
     * Not allowed.
     *
     * @param[in] relEncoder Relative encoders instance, which shall be copied.
     */
    RelativeEncoders(const RelativeEncoders& relEncoder);

    /**
     * Assignment of an instance.
     * Not allowed.
     *
     * @param[in] relEncoder Relative encoders instance, which shall be assigned.
     *
     * @return Reference to relative encoders instance
     */
    RelativeEncoders& operator=(const RelativeEncoders& relEncoder);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ENCODERS_H */
/** @} */
