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
 * @brief  Odometry unit
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Service
 *
 * @{
 */

#ifndef ODOMETRY_H
#define ODOMETRY_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Board.h>
#include <RelativeEncoders.h>
#include <SimpleTimer.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class provides odometry data, based on the encoder informations.
 * Odometry data:
 * - orientation and delta orientation
 * - position and delta position
 * - mileage
 */
class Odometry
{
public:
    /**
     * Get odometry instance.
     *
     * @return Odometry instance.
     */
    static Odometry& getInstance()
    {
        static Odometry instance; /* idiom */

        return instance;
    }

    /**
     * Process the odometry unit periodically.
     */
    void process();

    /**
     * Get center mileage in mm.
     *
     * @return Mileage in mm
     */
    uint32_t getMileageCenter() const;

    /**
     * Get current orientation in mrad.
     *
     * @return Orientation in mrad
     */
    int32_t getOrientation() const
    {
        return m_orientation;
    }

    /**
     * Get absolute position in coordinatesystem.
     * The x- and y-axis unit is mm.
     *
     * @param[out] posX x-coordinate
     * @param[out] posY y-coordinate
     */
    void getPosition(int32_t& posX, int32_t& posY) const
    {
        posX = m_posX;
        posY = m_posY;
    }

    /**
     * Clear position and orientation.
     */
    void clearPositionAndOrientation();

    /**
     * Clear mileage.
     */
    void clearMileage();

    /**
     * Is standstill detected?
     *
     * @return If standstill, it will return true otherwise false.
     */
    bool isStandStill()
    {
        return m_isStandstill;
    }

private:
    /**
     * If at least one side moved about 2 mm, a new calculation shall be done.
     *
     * Use a multiple of RobotConstants::ENCODER_STEPS_PER_MM for better
     * accuracy.
     */
    static const uint16_t STEPS_THRESHOLD;

    /**
     * Time period in ms for standstill detection.
     * If there is no encoder change during this period, it is assumed that
     * the robot stopped.
     */
    static const uint32_t STANDSTILL_DETECTION_PERIOD = 10;

    /**
     * Last number of relative encoder steps left. Its used to avoid permanent
     * clearing of the relative encoders.
     */
    uint16_t m_lastAbsRelEncStepsLeft;

    /**
     * Last number of relative encoder steps right. Its used to avoid permanent
     * clearing of the relative encoders.
     */
    uint16_t m_lastAbsRelEncStepsRight;

    /** Mileage in encoder steps. */
    uint32_t m_mileage;

    /** Relative encoders left/right. */
    RelativeEncoders m_relEncoders;

    /** Absolute orientation in mrad. 0 mrad means the robot drives parallel to the y-axis.  */
    int32_t m_orientation;

    /** Absolute position on x-axis. Unit is mm. */
    int32_t m_posX;

    /** Absolute position on y-axis. Unit is mm. */
    int32_t m_posY;

    /** Timer used to detect standstill. */
    SimpleTimer m_timer;

    /** Is robot stopped? */
    bool m_isStandstill;

    /**
     * Construct the odometry instance.
     */
    Odometry() :
        m_lastAbsRelEncStepsLeft(0),
        m_lastAbsRelEncStepsRight(0),
        m_mileage(0),
        m_relEncoders(Board::getInstance().getEncoders()),
        m_orientation(0),
        m_posX(0),
        m_posY(0),
        m_timer(),
        m_isStandstill(true)
    {
    }

    /**
     * Destroy the odometry instance.
     */
    ~Odometry()
    {
    }

    Odometry(const Odometry& value);
    Odometry& operator=(const Odometry& value);

    /**
     * Is the robot standstill?
     *
     * @param[in] relStepsLeft      Relative absolute steps left
     * @param[in] relStepsRight     Relative absolute steps right
     */

    bool detectStandStill(uint16_t absStepsLeft, uint16_t absStepsRight);

    /**
     * Calculate the mileage in mm.
     * 
     * @param[in]   mileage     Mileage in mm
     * @param[in]   stepsCenter Number of steps center
     * 
     * @return Mileage in mm
     */
    int32_t calculateMileage(uint32_t mileage, int16_t stepsCenter);

    /**
     * Calculate the orientation in mrad.
     *
     * @param[in] orientation   Orientation in mrad
     * @param[in] stepsLeft     Number of encoder steps left
     * @param[in] stepsRight    Number of encoder steps right
     *
     * @return Orientation in mrad
     */
    int32_t calculateOrientation(int32_t orientation, int16_t stepsLeft, int16_t stepsRight) const;

    /**
     * Calculate the vector from last position to new position.
     *
     * @param[in]   stepsCenter Number of steps center
     * @param[in]   orientation Orientation in mrad
     * @param[out]  dX          Delta x-position on x-axis
     * @param[out]  dY          Delta y-position on y-axis
     */
    void calculateDeltaPos(int16_t stepsCenter, int32_t orientation, int16_t& dX, int16_t& dY) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ODOMETRY_H */