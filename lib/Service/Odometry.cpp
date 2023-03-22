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
const uint16_t Odometry::STEPS_THRESHOLD = static_cast<uint16_t>(RobotConstants::ENCODER_STEPS_PER_MM);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Odometry::process()
{
    int16_t  relStepsLeft  = m_relEncoders.getCountsLeft();
    int16_t  relStepsRight = m_relEncoders.getCountsRight();
    uint16_t absStepsLeft  = abs(relStepsLeft);  /* Positive amount of delta steps left */
    uint16_t absStepsRight = abs(relStepsRight); /* Positive amount of delta steps right*/
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
            int16_t dX          = 0;                                  /* [mm] */
            int16_t dY          = 0;                                  /* [mm] */

            /* Mileage accuracy depends on STEPS_THRESHOLD. */
            m_mileage += abs(stepsCenter);

            m_orientation = calculateOrientation(m_orientation, relStepsLeft, relStepsRight);

            calculateDeltaPos(stepsCenter, m_orientation, dX, dY);
            m_posX += dX;
            m_posY += dY;

            LOG_DEBUG_VAL(TAG, "x: ", m_posX);
            LOG_DEBUG_VAL(TAG, "y: ", m_posY);
            LOG_DEBUG_VAL(TAG, "O: ", MRAD2DEG(m_orientation));

            /* Reset to be able to calculate the next delta. */
            m_lastAbsRelEncStepsLeft  = 0;
            m_lastAbsRelEncStepsRight = 0;
            m_relEncoders.clear();
        }
    }
}

uint32_t Odometry::getMileageCenter() const
{
    return m_mileage / RobotConstants::ENCODER_STEPS_PER_MM;
}

void Odometry::clearPositionAndOrientation()
{
    m_posX        = 0;
    m_posY        = 0;
    m_orientation = 0;
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

void Odometry::calculateDeltaPos(int16_t stepsCenter, int32_t orientation, int16_t& dX, int16_t& dY) const
{
    int16_t distCenter   = stepsCenter / static_cast<int16_t>(RobotConstants::ENCODER_STEPS_PER_MM); /* [mm] */
    float   fOrientation = static_cast<float>(orientation) / 1000.0f;                                /* [rad] */

    dX = static_cast<int16_t>((-static_cast<float>(distCenter)) * sinf(fOrientation));
    dY = static_cast<int16_t>(static_cast<float>(distCenter) * cosf(fOrientation));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
