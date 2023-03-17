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

const int16_t Odometry::STEPS_THRESHOLD = static_cast<int16_t>(2 * RobotConstants::ENCODER_STEPS_PER_MM);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Odometry::process()
{
    IMotors& motors        = Board::getInstance().getMotors();
    int16_t  relStepsLeft  = m_relEncoders.getCountsLeft();
    int16_t  relStepsRight = m_relEncoders.getCountsRight();
    uint16_t absStepsLeft  = abs(relStepsLeft);
    uint16_t absStepsRight = abs(relStepsRight);

    /* Calculate absolute accumulated number steps for the left encoder. */
    m_absEncStepsLeft += absStepsLeft - m_lastAbsRelEncStepsLeft;
    m_lastAbsRelEncStepsLeft = absStepsLeft;

    /* Calculate absolute accumulated number steps for the right encoder. */
    m_absEncStepsRight += absStepsRight - m_lastAbsRelEncStepsRight;
    m_lastAbsRelEncStepsRight = absStepsRight;

    /* Orientation shall not be calculated from stand still to driving,
     * because there can not be any orientation change before.
     */
    if ((0 == m_lastMotorSpeedLeft) && (0 == m_lastMotorSpeedRight))
    {
        /* Calculate it with the next motor speed change. */
        m_lastMotorSpeedLeft  = motors.getLeftSpeed();
        m_lastMotorSpeedRight = motors.getRightSpeed();
    }
    /* The calculation of the orientation/movement depends on the driven distance.
     * If a distance threshold is exceeded, it will be calculated.
     */
    else if ((STEPS_THRESHOLD <= absStepsLeft) || (STEPS_THRESHOLD <= absStepsRight))
    {
        int16_t alpha       = 0;
        int16_t stepsCenter = (relStepsLeft + relStepsRight) / 2;
        int16_t dX          = 0;
        int16_t dY          = 0;

        m_orientation = calculateOrientation(m_orientation, relStepsLeft, relStepsRight, alpha);

        calculateDeltaPos(stepsCenter, alpha, dX, dY);
        m_posX += dX;
        m_posY += dY;

        /* Reset to be able to calculate the next delta. */
        m_lastMotorSpeedLeft      = motors.getLeftSpeed();
        m_lastMotorSpeedRight     = motors.getRightSpeed();
        m_lastAbsRelEncStepsLeft  = 0;
        m_lastAbsRelEncStepsRight = 0;

        m_relEncoders.clear();
    }
}

uint32_t Odometry::getMileageCenter() const
{
    uint32_t encoderSteps = (m_absEncStepsLeft + m_absEncStepsRight) / 2;

    return encoderSteps / RobotConstants::ENCODER_STEPS_PER_MM;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

int16_t Odometry::calculateOrientation(int16_t orientation, int16_t stepsLeft, int16_t stepsRight, int16_t& alpha) const
{
    /* The alpha is approximated for performance reason. */
    alpha = 1000 * (stepsRight - stepsLeft);
    alpha /= static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM);
    alpha /= static_cast<int16_t>(RobotConstants::WHEEL_BASE); /* [mrad] */
    alpha %= FP_2PI();                                         /* -2*PI < alpha < +2*PI */

    /* Calculate orientation */
    orientation += alpha;
    orientation %= FP_2PI(); /* -2*PI < orientation < +2*PI */

    return orientation;
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
