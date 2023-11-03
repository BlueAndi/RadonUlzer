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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RelativeEncoders.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void RelativeEncoders::clear()
{
    m_referencePointLeft       = m_absEncoders.getCountsLeft();
    m_referencePointRight      = m_absEncoders.getCountsRight();
    m_lastRelEncoderStepsLeft  = 0;
    m_lastRelEncoderStepsRight = 0;
}

void RelativeEncoders::clearLeft()
{
    m_referencePointLeft      = m_absEncoders.getCountsLeft();
    m_lastRelEncoderStepsLeft = 0;
}

void RelativeEncoders::clearRight()
{
    m_referencePointRight      = m_absEncoders.getCountsRight();
    m_lastRelEncoderStepsRight = 0;
}

int16_t RelativeEncoders::getCountsLeft() const
{
    return calculate(m_absEncoders.getCountsLeft(), m_referencePointLeft);
}

int16_t RelativeEncoders::getCountsRight() const
{
    return calculate(m_absEncoders.getCountsRight(), m_referencePointRight);
}

RelativeEncoders::Direction RelativeEncoders::getDirectionLeft()
{
    int16_t   diffSteps = getCountsLeft();
    Direction direction = getDirection(m_lastRelEncoderStepsLeft, diffSteps);

    m_lastRelEncoderStepsLeft = diffSteps;

    return direction;
}

RelativeEncoders::Direction RelativeEncoders::getDirectionRight()
{
    int16_t   diffSteps = getCountsRight();
    Direction direction = getDirection(m_lastRelEncoderStepsRight, diffSteps);

    m_lastRelEncoderStepsRight = diffSteps;

    return direction;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

int16_t RelativeEncoders::calculate(int16_t absSteps, int16_t refPoint) const
{
    int16_t diffSteps = absSteps - refPoint;

    /* Wrap around in forward or backward direction?
     * In both cases the difference must be multiplied with -1.
     */
    if (((0 > absSteps) && (0 < refPoint)) || ((0 < absSteps) && (0 > refPoint)))
    {
        diffSteps *= -1;
    }

    return diffSteps;
}

RelativeEncoders::Direction RelativeEncoders::getDirection(int16_t lastRelSteps, int16_t currentRelSteps) const
{
    Direction direction = DIRECTION_STOPPED;

    if (lastRelSteps < currentRelSteps)
    {
        direction = DIRECTION_POSTIVE;
    }
    else if (lastRelSteps > currentRelSteps)
    {
        direction = DIRECTION_NEGATIVE;
    }
    else
    {
        ;
    }

    return direction;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
