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
 * @brief  Encoders realization
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALTarget
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
#include "IEncodersTest.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the Zumo target encoders. */
class Encoders : public IEncoders, public IEncodersTest
{
public:
    /**
     * Constructs the encoders adapter.
     */
    Encoders() : IEncoders(), m_stepsLeft(0), m_stepsRight(0)
    {
    }

    /**
     * Destroys the encoders adapter.
     */
    ~Encoders()
    {
    }

    /**
     * Returns the number of counts that have been detected from the left-side
     * encoder.  These counts start at 0. Positive counts correspond to forward
     * movement of the left side of the robot, while negative counts correspond
     * to backwards movement.
     *
     * The count is returned as a signed 16-bit integer. When the count goes
     * over 32767, it will overflow down to -32768.  When the count goes below
     * -32768, it will overflow up to 32767.
     *
     * @return Encoder steps left
     */
    int16_t getCountsLeft() final
    {
        return m_stepsLeft;
    }

    /**
     * Returns the number of counts that have been detected from the right-side
     * encoder.  These counts start at 0. Positive counts correspond to forward
     * movement of the left side of the robot, while negative counts correspond
     * to backwards movement.
     *
     * The count is returned as a signed 16-bit integer. When the count goes
     * over 32767, it will overflow down to -32768.  When the count goes below
     * -32768, it will overflow up to 32767.
     *
     * @return Encoder steps right
     */
    int16_t getCountsRight() final
    {
        return m_stepsRight;
    }

    /**
     * This function is just like getCountsLeft() except it also clears the
     * counts before returning. If you call this frequently enough, you will
     * not have to worry about the count overflowing.
     *
     * @return Encoder steps left
     */
    int16_t getCountsAndResetLeft() final
    {
        int16_t stepsLeft = m_stepsLeft;

        m_stepsLeft = 0;

        return stepsLeft;
    }

    /**
     * This function is just like getCountsRight() except it also clears the
     * counts before returning. If you call this frequently enough, you will
     * not have to worry about the count overflowing.
     *
     * @return Encoder steps right
     */
    int16_t getCountsAndResetRight() final
    {
        int16_t stepsRight = m_stepsRight;

        m_stepsRight = 0;

        return stepsRight;
    }

    /* ---------- Test Interface ---------- */

    /**
     * Set encoder steps left.
     *
     * @param[in] steps Encoder steps
     */
    void setCountsLeft(int16_t steps) final
    {
        m_stepsLeft = steps;
    }

    /**
     * Set encoder steps right.
     *
     * @param[in] steps Encoder steps
     */
    void setCountsRight(int16_t steps) final
    {
        m_stepsRight = steps;
    }

private:
    int16_t m_stepsLeft;  /**< Encoder steps left */
    int16_t m_stepsRight; /**< Encoder steps right*/
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ENCODERS_H */
/** @} */
