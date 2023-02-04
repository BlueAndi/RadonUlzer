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
     */
    Encoders(uint16_t gearRatio, uint32_t wheelSize, webots::PositionSensor* encoderLeft, webots::PositionSensor* encoderRight) : 
    IEncoders(), 
    m_gearRatio(gearRatio),
    m_wheelSize(wheelSize),
    m_encoderLeft(encoderLeft),
    m_encoderRight(encoderRight),
    m_lastResetValueLeft(0.0),
    m_lastResetValueRight(0.0),
    m_stepsPerMM((10UL * RESOLUTION * m_gearRatio) / m_wheelSize)
    {
    }

    /**
     * Destroys the encoders adapter.
     */
    ~Encoders()
    {
    }

    /**
     * Returns the counts of the encoder of the left-side motor.
     * Internal calculation are done because the positionSensor
     * in webots that acts as an encoder only returns the driven
     * way in m/s and not the counted steps.
     * 
     * @return Encoder steps left
     */
    int16_t getCountsLeft() final
    {
        /** 
         * Value needs to be multiplied with 1000 to get from m to mm.
         * It then needs to be multiplied with 8 to get the steps.
         */
        return static_cast<int16_t>((m_encoderLeft->getValue() - m_lastResetValueLeft) * 
                                    MILLIMETER * m_stepsPerMM);
    }

    /**
     * Returns the counts of the encoder of the right-side motor.
     * Internal calculation are done because the positionSensor
     * in webots that acts as an encoder only returns the driven
     * way in m/s and not the counted steps.
     * 
     * @return Encoder steps right
     */
    int16_t getCountsRight() final
    {
        return static_cast<int16_t>((m_encoderRight->getValue() - m_lastResetValueRight) * 
                                    MILLIMETER * m_stepsPerMM);
    }

    /**
     * This function is just like getCountsLeft() except it also clears the
     * counts before returning.  If you call this frequently enough, you will
     * not have to worry about the count overflowing.
     * 
     * @return Encoder steps left
     */
    int16_t getCountsAndResetLeft() final
    {
        uint16_t ret = static_cast<int16_t>((m_encoderLeft->getValue() - m_lastResetValueLeft) * 
                                            MILLIMETER * m_stepsPerMM);
        m_lastResetValueLeft = m_encoderLeft->getValue();
        return ret;
    }

    /**
     * This function is just like getCountsRight() except it also clears the
     * counts before returning.  If you call this frequently enough, you will
     * not have to worry about the count overflowing.
     * 
     * @return Encoder steps right
     */
    int16_t getCountsAndResetRight() final
    {
        uint16_t ret = static_cast<int16_t>((m_encoderRight->getValue() - m_lastResetValueRight) * 
                                            MILLIMETER * m_stepsPerMM);
        m_lastResetValueRight = m_encoderRight->getValue();
        return ret;
    }

    /**
     * Get encoder resolution.
     *
     * @return Encoder resolution in counts per revolution of the motor shaft.
     */
    uint16_t getResolution() const final
    {
        return RESOLUTION;
    }

private:
    /**
     * Encoder resolution in counts per revolution of the motor shaft.
     */
    static const uint16_t   RESOLUTION = 12;

    /* Conversion factor from mm to m. */
    static const uint16_t   MILLIMETER = 1000;

    /* The gear ratio of the motor. */
    uint16_t m_gearRatio;

    /* The size of the wheels in mm. */
    uint32_t m_wheelSize;

    /* The encoder of the left motot in the robot simulation. */
    webots::PositionSensor* m_encoderLeft;

    /* The encoder of the left motot in the robot simulation. */
    webots::PositionSensor* m_encoderRight;

    /* Reset value needed to calculate the steps. */
    double m_lastResetValueLeft;

    /* Reset value needed to calculate the steps. */
    double m_lastResetValueRight;

    /* The encoder steps the real robot makes per mm driven. */
    uint8_t m_stepsPerMM;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ENCODERS_H */