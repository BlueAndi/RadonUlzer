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
 * @brief  Relative encoder
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
#include "IEncoders.h"
#include <Board.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides access to the Zumo target encoders. */
class RelativeEncoders
{
public:
    /**
     * Constructs the encoders adapter.
     */
    RelativeEncoders(IEncoders& encoders) : 
    m_encoders(encoders),
    m_lastResetValueLeft(m_encoders.getCountsLeft()),
    m_lastResetValueRight(m_encoders.getCountsRight()),
    m_lastSpeedLeft(Board::getInstance().getMotors().getLeftSpeed()),
    m_lastSpeedRight(Board::getInstance().getMotors().getRightSpeed())
    {
    }

    /**
     * Destroys the encoders adapter.
     */
    ~RelativeEncoders()
    {
    }

    /**
     * Returns the counts of the encoder of the left-side motor.
     */
    int16_t getCountsLeft()
    {
        int16_t currentValue = m_encoders.getCountsLeft();
        int16_t ret = 0;

        int16_t lastSpeed = m_lastSpeedLeft;
        m_lastSpeedLeft = Board::getInstance().getMotors().getLeftSpeed();

        /** 
         * Handling the postive overflow (from 32767 to -32768).
         * Currently uses the area of +-10% of the max values for detection.
         */
        if((lastSpeed > 0) && (currentValue < m_lastResetValueLeft))
        {
            ret = (INT16_MAX - m_lastResetValueLeft) + (currentValue - INT16_MIN);
        }
        /** 
         * Handling the negative overflow (from -32768 to 32767).
         * Currently uses the area of +-10% of the max values for detection.
         */
        else if((lastSpeed < 0) && (currentValue > m_lastResetValueLeft))
        {
            ret = (INT16_MIN - m_lastResetValueLeft) - (INT16_MAX - currentValue);
        }
        else
        {
            ret = currentValue - m_lastResetValueLeft;
        }

        return ret;
    }

    /**
     * This function is just like getCountsLeft() except it applies to the
     * right-side encoder.
     */
    int16_t getCountsRight()
    {
        int16_t currentValue = m_encoders.getCountsRight();
        int16_t ret = 0;

        int16_t lastSpeed = m_lastSpeedRight;
        m_lastSpeedRight = Board::getInstance().getMotors().getRightSpeed();

        /** 
         * Handling the postive overflow (from 32767 to -32768).
         * Currently uses the area of +-10% of the max values for detection.
         */
        if((lastSpeed > 0) && (currentValue < m_lastResetValueRight))
        {
            ret = (INT16_MAX - m_lastResetValueRight) + (currentValue - INT16_MIN);
        }
        /** 
         * Handling the negative overflow (from -32768 to 32767).
         * Currently uses the area of +-10% of the max values for detection.
         */
        else if((lastSpeed < 0) && (currentValue > m_lastResetValueRight))
        {
            ret = (INT16_MIN - m_lastResetValueRight) - (INT16_MAX - currentValue);
        }
        else
        {
            ret = currentValue - m_lastResetValueRight;
        }

        return ret;
    }

    /**
     * This function is just like getCountsLeft() except it also clears the
     * counts before returning.
     */
    int16_t getCountsAndResetLeft() 
    {
        int16_t currentValue = m_encoders.getCountsLeft();
        int16_t ret = 0;

        int16_t lastSpeed = m_lastSpeedLeft;
        m_lastSpeedLeft = Board::getInstance().getMotors().getLeftSpeed();

        /** 
         * Handling the postive overflow (from 32767 to -32768).
         * Currently uses the area of +-10% of the max values for detection.
         */
        if((lastSpeed > 0) && (currentValue < m_lastResetValueLeft))
        {
            ret = (INT16_MAX - m_lastResetValueLeft) + (currentValue - INT16_MIN);
        }
        /** 
         * Handling the negative overflow (from -32768 to 32767).
         * Currently uses the area of +-10% of the max values for detection.
         */
        else if((lastSpeed < 0) && (currentValue > m_lastResetValueLeft))
        {
            ret = (INT16_MIN - m_lastResetValueLeft) - (INT16_MAX - currentValue);
        }
        else
        {
            ret = currentValue - m_lastResetValueLeft;
        }

        m_lastResetValueLeft = currentValue;

        return ret;
    }

    /**
     * This function is just like getCountsAndResetLeft() except it applies to
     * the right-side encoder.
     */
    int16_t getCountsAndResetRight()
    {
        int16_t currentValue = m_encoders.getCountsRight();
        int16_t ret = 0;

        int16_t lastSpeed = m_lastSpeedRight;
        m_lastSpeedRight = Board::getInstance().getMotors().getRightSpeed();

        /** 
         * Handling the postive overflow (from 32767 to -32768).
         * Currently uses the area of +-10% of the max values for detection.
         */
        if((lastSpeed > 0) && (currentValue < m_lastResetValueRight))
        {
            ret = (INT16_MAX - m_lastResetValueRight) + (currentValue - INT16_MIN);
        }
        /** 
         * Handling the negative overflow (from -32768 to 32767).
         * Currently uses the area of +-10% of the max values for detection.
         */
        else if((lastSpeed < 0) && (currentValue > m_lastResetValueRight))
        {
            ret = (INT16_MIN - m_lastResetValueRight) - (INT16_MAX - currentValue);
        }
        else
        {
            ret = currentValue - m_lastResetValueRight;
        }

        m_lastResetValueRight = currentValue;

        return ret;
    }

private:

    IEncoders& m_encoders;          /**< Encoder implementation that provides continuous count. */

    int16_t m_lastResetValueLeft;  /**< The last value when the left counter was reset. */ 
    int16_t m_lastResetValueRight; /**< The last value when the left counter was reset. */ 

    int16_t m_lastSpeedLeft;       /**< The last measured speed of the left motor. */
    int16_t m_lastSpeedRight;      /**< The last measured speed of the right motor. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ENCODERS_H */