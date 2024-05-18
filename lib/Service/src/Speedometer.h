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
 * @brief  Speedometer
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Board.h>
#include <RelativeEncoders.h>
#include <RobotConstants.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class provides the linear speed in [steps/s], based on the encoder informations. */
class Speedometer
{
public:
    /**
     * Get speedometer instance.
     *
     * @return Speedometer instance.
     */
    static Speedometer& getInstance()
    {
        return m_instance;
    }

    /**
     * Update linear speed, based on the measured encoder steps.
     * Call this function cyclic.
     */
    void process();

    /**
     * Get linear speed center in steps/s.
     *
     * @return Linear speed center in steps/s
     */
    int16_t getLinearSpeedCenter() const;

    /**
     * Get linear speed left in steps/s.
     *
     * @return Linear speed left in steps/s
     */
    int16_t getLinearSpeedLeft() const;

    /**
     * Get linear speed right in steps/s.
     *
     * @return Linear speed right in steps/s
     */
    int16_t getLinearSpeedRight() const;

private:
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
     * The minimum number of counted encoder steps until the speed is measured.
     * It shall avoid a noisy speed.
     */
    static const int32_t MIN_ENCODER_COUNT = static_cast<int32_t>(RobotConstants::ENCODER_RESOLUTION / 2U);

    /** Speedometer instance */
    static Speedometer m_instance;

    /** Relative encoder left/right */
    RelativeEncoders m_relEncoders;

    /** Timestamp of last left speed calculation. */
    uint32_t m_timestampLeft;

    /** Timestamp of last right speed calculation. */
    uint32_t m_timestampRight;

    /** Linear speed left in steps/s */
    int16_t m_linearSpeedLeft;

    /** Linear speed right in steps/s */
    int16_t m_linearSpeedRight;

    /** Last determined driving direction left. */
    Direction m_lastDirectionLeft;

    /** Last determined driving direction left. */
    Direction m_lastDirectionRight;

    /**
     * Construct the mileage instance.
     */
    Speedometer() :
        m_relEncoders(Board::getInstance().getEncoders()),
        m_timestampLeft(0),
        m_timestampRight(0),
        m_linearSpeedLeft(0),
        m_linearSpeedRight(0),
        m_lastDirectionLeft(DIRECTION_STOPPED),
        m_lastDirectionRight(DIRECTION_STOPPED)
    {
    }

    /**
     * Destroy the mileage instance.
     */
    ~Speedometer()
    {
    }

    /* Not allowed. */
    Speedometer(const Speedometer& value);            /**< Copy construction of an instance. */
    Speedometer& operator=(const Speedometer& value); /**< Assignment of an instance. */

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

    /**
     * Determine the direction of the movement by motor speed.
     *
     * @param[in] motorSpeed    Motor speed in digits
     *
     * @return Direction of movement.
     */
    Direction getDirectionByMotorSpeed(int16_t motorSpeed);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SPEEDOMETER_H */
/** @} */
