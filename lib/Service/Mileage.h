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
 * @brief  Mileage
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup Service
 *
 * @{
 */

#ifndef MILEAGE_H
#define MILEAGE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <SimpleTimer.h>
#include <RelativeEncoder.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides the mileage and speed, based on the encoder informations. */
class Mileage
{
public:
    /**
     * Get mileage/speed instance.
     *
     * @return Mileage/Speed instance.
     */
    static Mileage& getInstance()
    {
        return m_instance;
    }

    /**
     * Clear mileage.
     */
    void clear();

    /**
     * Update mileage and speed, based on the measured encoder steps.
     * Call this function cyclic.
     */
    void process();

    /**
     * Get center mileage in mm.
     *
     * @return Mileage in mm
     */
    uint32_t getMileageCenter() const;

    /**
     * Get the center speed in steps/s.
     *
     * @return Speed in steps/s.
     */
    int16_t getSpeedCenter() const;

    /**
     * Get the left speed in steps/s.
     *
     * @return Speed in steps/s.
     */
    int16_t getSpeedLeft() const;

    /**
     * Get the right speed in steps/s.
     *
     * @return Speed in steps/s.
     */
    int16_t getSpeedRight() const;

private:

    /** Min. period in ms, after which the mileage and speed shall be calculated. */
    static const uint32_t MIN_PERIOD = 5;

    /** Mileage/Speed instance */
    static Mileage m_instance;

    /** Timer used to control processing of encoders. */
    SimpleTimer m_timer;

    /** Mileage left in encoder steps. */
    uint32_t m_encoderStepsLeft;

    /** Mileage right in encoder steps. */
    uint32_t m_encoderStepsRight;

    /** Left speed in steps/s */
    int16_t m_speedLeft;

    /** Right speed in steps/s */
    int16_t m_speedRight;

    /** Relative encoder left */
    RelativeEncoder m_relEncLeft;

    /** Relative encoder right */
    RelativeEncoder m_relEncRight;

    /**
     * Construct the mileage instance.
     */
    Mileage() : 
        m_timer(), 
        m_encoderStepsLeft(0), 
        m_encoderStepsRight(0), 
        m_speedLeft(0), 
        m_speedRight(0),
        m_relEncLeft(),
        m_relEncRight()
    {
    }

    /**
     * Destroy the mileage instance.
     */
    ~Mileage()
    {
    }

    Mileage(const Mileage& value);
    Mileage& operator=(const Mileage& value);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MILEAGE_H */