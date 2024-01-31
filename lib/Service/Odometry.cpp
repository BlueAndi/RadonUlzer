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
 * @brief  Odometry
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Odometry.h>
#include <Board.h>
#include <RobotConstants.h>

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

/* Initialize static constant data. */
const uint16_t Odometry::STEPS_THRESHOLD = static_cast<uint16_t>(10U * RobotConstants::ENCODER_STEPS_PER_MM);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Odometry::process()
{
    int16_t  relStepsLeft  = m_relEncoders.getCountsLeft();  /* [steps] */
    int16_t  relStepsRight = m_relEncoders.getCountsRight(); /* [steps] */
    uint16_t absStepsLeft  = abs(relStepsLeft);              /* Positive amount of delta steps left */
    uint16_t absStepsRight = abs(relStepsRight);             /* Positive amount of delta steps right*/
    bool     isNoMovement  = detectStandStill(absStepsLeft, absStepsRight);

    /* Orientation shall not be calculated from stand still to driving,
     * because there can not be any orientation change before.
     */
    if (false == isNoMovement)
    {
        /* The calculation of the orientation/movement depends on the driven distance.
         * If a distance threshold is exceeded, it will be calculated.
         */
        if ((STEPS_THRESHOLD <= absStepsLeft) || (STEPS_THRESHOLD <= absStepsRight))
        {
            int16_t stepsCenter = (relStepsLeft + relStepsRight) / 2; /* [steps] */
            int16_t dXSteps     = 0;                                  /* [steps] */
            int16_t dYSteps     = 0;                                  /* [steps] */

            /* Mileage accuracy depends on STEPS_THRESHOLD. */
            m_mileage = calculateMileage(m_mileage, stepsCenter);

            m_orientation = calculateOrientation(m_orientation, relStepsLeft, relStepsRight);

            /* Calculate in steps to avoid loosing precision by divison. */
            calculateDeltaPos(stepsCenter, m_orientation, dXSteps, dYSteps);
            m_countingXSteps += dXSteps;
            m_countingYSteps += dYSteps;

            /* For large areas, its important to have the position in mm and not in steps.
             * Therefore the position in mm is continously calculated from the counted steps
             * on each axis.
             */
            m_posX += m_countingXSteps / static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM);
            m_posY += m_countingYSteps / static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM);
            m_countingXSteps %= static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM);
            m_countingYSteps %= static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM);

            /* Reset to be able to calculate the next delta. */
            m_lastAbsRelEncStepsLeft  = 0;
            m_lastAbsRelEncStepsRight = 0;
            m_relEncoders.clear();
        }
    }
}

uint32_t Odometry::getMileageCenter() const
{
    int16_t  relStepsLeft  = m_relEncoders.getCountsLeft();      /* [steps] */
    int16_t  relStepsRight = m_relEncoders.getCountsRight();     /* [steps] */
    int16_t  stepsCenter   = (relStepsLeft + relStepsRight) / 2; /* [steps] */
    uint32_t mileage       = calculateMileage(m_mileage, stepsCenter);

    /* For higher accuracy use the current relative steps left and right.
     * The m_mileage will only be updated every STEPS_THRESHOLD, which
     * will reset the relative encoders to 0.
     */
    return mileage / RobotConstants::ENCODER_STEPS_PER_MM;
}

int32_t Odometry::getOrientation() const
{
    int16_t relStepsLeft  = m_relEncoders.getCountsLeft();  /* [steps] */
    int16_t relStepsRight = m_relEncoders.getCountsRight(); /* [steps] */

    /* For higher accuracy use the current relative steps left and right.
     * The m_orientation will only be updated every STEPS_THRESHOLD, which
     * will reset the relative encoders to 0.
     */
    return calculateOrientation(m_orientation, relStepsLeft, relStepsRight);
}

void Odometry::clearPosition()
{
    m_posX           = 0;
    m_posY           = 0;
    m_countingXSteps = 0;
    m_countingYSteps = 0;
}

void Odometry::clearMileage()
{
    m_mileage = 0;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool Odometry::detectStandStill(uint16_t absStepsLeft, uint16_t absStepsRight)
{
    bool isStandStill = false;

    /* No encoder (left/right) change detected? */
    if (absStepsLeft == m_lastAbsRelEncStepsLeft)
    {
        if (absStepsRight == m_lastAbsRelEncStepsRight)
        {
            isStandStill = true;
        }
    }

    /* Is robot moving? */
    if (false == isStandStill)
    {
        m_isStandstill = false;
        m_timer.stop();
    }
    /* Robot seems to not to move anymore, lets debounce. */
    else if (false == m_isStandstill)
    {
        /* The first time of no movement, start the debounce timer. */
        if (false == m_timer.isRunning())
        {
            m_timer.start(STANDSTILL_DETECTION_PERIOD);
        }
        /* If over the while debounce time there is no movement, it will be considered as standstill. */
        else if (true == m_timer.isTimeout())
        {
            m_isStandstill = true;
        }
    }
    /* Robot standstill. */
    else
    {
        /* Nothing to do. */
        ;
    }

    return m_isStandstill;
}

int32_t Odometry::calculateMileage(uint32_t mileage, int16_t stepsCenter) const
{
    return mileage + static_cast<uint32_t>(abs(stepsCenter));
}

int32_t Odometry::calculateOrientation(int32_t orientation, int16_t stepsLeft, int16_t stepsRight) const
{
    /* The alpha is approximated for performance reason. */
    int32_t stepsLeft32  = static_cast<int32_t>(stepsLeft);
    int32_t stepsRight32 = static_cast<int32_t>(stepsRight);
    int32_t alpha        = (stepsRight32 - stepsLeft32) * 1000; /* 1000 * [steps] */

    alpha /= static_cast<int32_t>(RobotConstants::ENCODER_STEPS_PER_MM); /* 1000 * [mm] */
    alpha /= static_cast<int32_t>(RobotConstants::WHEEL_BASE);           /* [mrad] */
    alpha %= FP_2PI();                                                   /* -2*PI < alpha < +2*PI */

    /* Calculate orientation */
    orientation += alpha;
    orientation %= FP_2PI(); /* -2*PI < orientation < +2*PI */

    return orientation;
}

void Odometry::calculateDeltaPos(int16_t stepsCenter, int32_t orientation, int16_t& dXSteps, int16_t& dYSteps) const
{
    float fDistCenter  = static_cast<float>(stepsCenter);           /* [steps] */
    float fOrientation = static_cast<float>(orientation) / 1000.0F; /* [rad] */
    float fDeltaPosX   = fDistCenter * cosf(fOrientation);          /* [steps] */
    float fDeltaPosY   = fDistCenter * sinf(fOrientation);          /* [steps] */

    if (0.0F <= fDeltaPosX)
    {
        fDeltaPosX += 0.5F;
    }
    else
    {
        fDeltaPosX -= 0.5F;
    }

    if (0.0F <= fDeltaPosY)
    {
        fDeltaPosY += 0.5F;
    }
    else
    {
        fDeltaPosY -= 0.5F;
    }

    dXSteps = static_cast<int16_t>(fDeltaPosX); /* [steps] */
    dYSteps = static_cast<int16_t>(fDeltaPosY); /* [steps] */
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
