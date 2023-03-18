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
 * @brief  Odometry
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Odometry.h>
#include <Board.h>
#include <RobotConstants.h>
#include <FPMath.h>
#include <Logging.h>

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

/**
 * Logging source.
 */
static const char* TAG = "Odometry";

/* Initialize static constant data. */
const int16_t Odometry::STEPS_THRESHOLD = static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Odometry::process()
{
    IMotors& motors        = Board::getInstance().getMotors();
    int16_t  relStepsLeft  = m_relEncoders.getCountsLeft();
    int16_t  relStepsRight = m_relEncoders.getCountsRight();
    uint16_t absStepsLeft  = abs(relStepsLeft);  /* Positive amount of delta steps left */
    uint16_t absStepsRight = abs(relStepsRight); /* Positive amount of delta steps right*/

    updateMileage(absStepsLeft, absStepsRight);

    /* Orientation shall not be calculated from stand still to driving,
     * because there can not be any orientation change before.
     */
    if ((0 < m_lastMotorSpeedLeft) || (0 < m_lastMotorSpeedRight))
    {
        /* The calculation of the orientation/movement depends on the driven distance.
         * If a distance threshold is exceeded, it will be calculated.
         */
        if ((STEPS_THRESHOLD <= absStepsLeft) || (STEPS_THRESHOLD <= absStepsRight))
        {
            int16_t alpha       = 0;
            int16_t stepsCenter = (relStepsLeft + relStepsRight) / 2;
            int16_t dX          = 0;
            int16_t dY          = 0;

            m_orientation = calculateOrientation(m_orientation, relStepsLeft, relStepsRight, alpha);

            calculateDeltaPos(stepsCenter, alpha, dX, dY);
            m_posX += dX;
            m_posY += dY;

            LOG_DEBUG_VAL(TAG, "Steps left : ", relStepsLeft);
            LOG_DEBUG_VAL(TAG, "Steps right: ", relStepsRight);
            LOG_DEBUG_VAL(TAG, "Orientation (deg): ", MRAD2DEG(m_orientation));

            /* Reset to be able to calculate the next delta. */
            m_lastAbsRelEncStepsLeft  = 0;
            m_lastAbsRelEncStepsRight = 0;
            m_relEncoders.clear();
        }
    }

    /* Remember current motor speeds to be able to check next time whether
     * the robot just started or not.
     */
    m_lastMotorSpeedLeft  = motors.getLeftSpeed();
    m_lastMotorSpeedRight = motors.getRightSpeed();
}

uint32_t Odometry::getMileageCenter() const
{
    uint32_t encoderSteps = (m_absEncStepsLeft + m_absEncStepsRight) / 2;

    return encoderSteps / RobotConstants::ENCODER_STEPS_PER_MM;
}

void Odometry::clearMileage()
{
    m_absEncStepsLeft  = 0;
    m_absEncStepsRight = 0;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Odometry::updateMileage(uint16_t absStepsLeft, uint16_t absStepsRight)
{
    /* Calculate absolute accumulated number steps for the left encoder. */
    m_absEncStepsLeft += absStepsLeft - m_lastAbsRelEncStepsLeft;
    m_lastAbsRelEncStepsLeft = absStepsLeft;

    /* Calculate absolute accumulated number steps for the right encoder. */
    m_absEncStepsRight += absStepsRight - m_lastAbsRelEncStepsRight;
    m_lastAbsRelEncStepsRight = absStepsRight;
}

int16_t Odometry::calculateOrientation(int16_t orientation, int16_t stepsLeft, int16_t stepsRight, int16_t& alpha) const
{
    /* The alpha is approximated for performance reason. */
    int32_t orientation32 = static_cast<int32_t>(orientation);
    int32_t stepsLeft32   = static_cast<int32_t>(stepsLeft);
    int32_t stepsRight32  = static_cast<int32_t>(stepsRight);
    int32_t alpha32       = (stepsRight32 - stepsLeft32) * 1000; /* 1000 * [steps] */

    alpha32 /= static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM); /* 1000 * [mm] */
    alpha32 /= static_cast<int32_t>(RobotConstants::WHEEL_BASE);           /* [mrad] */
    alpha32 %= FP_2PI();                                                   /* -2*PI < alpha < +2*PI */

    alpha = static_cast<int16_t>(alpha32);

    /* Calculate orientation */
    orientation32 += alpha32;
    orientation32 %= FP_2PI(); /* -2*PI < orientation < +2*PI */

    return static_cast<int16_t>(orientation32);
}

void Odometry::calculateDeltaPos(int16_t stepsCenter, int16_t alpha, int16_t& dX, int16_t& dY) const
{
    int16_t distCenter = stepsCenter / static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM); /* [mm] */

    dX = static_cast<int16_t>((-static_cast<float>(distCenter)) * sinf(alpha));
    dY = static_cast<int16_t>(static_cast<float>(distCenter) * cosf(alpha));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
