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
 * @brief  The physical robot board realization.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALTarget
 *
 * @{
 */
#ifndef BOARD_H
#define BOARD_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ButtonA.h>
#include <Buzzer.h>
#include <Display.h>
#include <Encoders.h>
#include <LineSensors.h>
#include <Motors.h>
#include <LedYellow.h>
#include <IMU.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The concrete physical board.
 */
class Board
{
public:
    /**
     * Get board instance.
     *
     * @return Board instance
     */
    static Board& getInstance()
    {
        static Board instance; /* idiom */

        return instance;
    }

    /**
     * Initialize the hardware.
     */
    void init();

    /**
     * Get button A driver.
     *
     * @return Button A driver.
     */
    IButton& getButtonA()
    {
        return m_buttonA;
    }

    /**
     * Get buzzer driver.
     *
     * @return Buzzer driver.
     */
    IBuzzer& getBuzzer()
    {
        return m_buzzer;
    }

    /**
     * Get encoders.
     *
     * @return Encoders driver.
     */
    IEncoders& getEncoders()
    {
        return m_encoders;
    }

    /**
     * Get line sensors driver.
     *
     * @return Line sensor driver.
     */
    ILineSensors& getLineSensors()
    {
        return m_lineSensors;
    }

    /**
     * Get motor driver.
     *
     * @return Motor driver.
     */
    IMotors& getMotors()
    {
        return m_motors;
    }

    /**
     * Get yellow LED driver.
     *
     * @return Yellow LED driver.
     */
    ILed& getYellowLed()
    {
        return m_ledYellow;
    }

    /**
     * Get IMU (Inertial Measurement Unit) driver.
     *
     * @return IMU driver
     */
    IIMU& getIMU()
    {
        return m_imu;
    }

protected:
private:
    /** Button A driver */
    ButtonA m_buttonA;

    /** Buzzer driver */
    Buzzer m_buzzer;

    /** Encoders driver */
    Encoders m_encoders;

    /** Line sensors driver */
    LineSensors m_lineSensors;

    /** Motors driver */
    Motors m_motors;

    /** Yellow LED driver */
    LedYellow m_ledYellow;

    /** IMU Driver */
    IMU m_imu;

    /**
     * Constructs the concrete board.
     */
    Board() :
        m_buttonA(),
        m_buzzer(),
        m_encoders(),
        m_lineSensors(),
        m_motors(),
        m_ledYellow(),
        m_imu()
    {
    }

    /**
     * Destroys the concrete board.
     */
    ~Board()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BOARD_H */
/** @} */