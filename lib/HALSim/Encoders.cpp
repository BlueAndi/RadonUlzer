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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Encoders.h"
#include "RobotConstants.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Encoders::init()
{
    if (nullptr != m_posSensorLeft)
    {
        m_posSensorLeft->enable(m_simTime.getTimeStep());
    }

    if (nullptr != m_posSensorRight)
    {
        m_posSensorRight->enable(m_simTime.getTimeStep());
    }
}

int16_t Encoders::getCountsLeft()
{
    int16_t steps = 0;

    /* The position sensor provides the driven distance in [m].
     * The encoder steps are dervived from it.
     */
    if (nullptr != m_posSensorLeft)
    {
        double currentPos = m_posSensorLeft->getValue();

        overflowProtection(m_lastResetValueLeft, currentPos);
        steps = calculateSteps(m_lastResetValueLeft, currentPos);
    }

    return steps;
}

int16_t Encoders::getCountsRight()
{
    int16_t steps = 0;

    /* The position sensor provides the driven distance in [m].
     * The encoder steps are dervived from it.
     */
    if (nullptr != m_posSensorRight)
    {
        double currentPos = m_posSensorRight->getValue();

        overflowProtection(m_lastResetValueLeft, currentPos);
        steps = calculateSteps(m_lastResetValueLeft, currentPos);
    }

    return steps;
}

int16_t Encoders::getCountsAndResetLeft()
{
    int16_t steps = getCountsLeft();

    m_lastResetValueLeft = m_posSensorLeft->getValue();

    return steps;
}

int16_t Encoders::getCountsAndResetRight()
{
    int16_t steps = getCountsRight();

    m_lastResetValueRight = m_posSensorRight->getValue();

    return steps;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Encoders::overflowProtection(double& lastPos, double pos)
{
    /* Position sensor provides NAN until the first simulation step was perfomed. */
    if (false == isnan(pos))
    {
        const double CONV_FACTOR_M_TO_MM = 1000.0F;
        const double OVERFLOW_DELTA_POS  = static_cast<double>(UINT16_MAX) /
                                          static_cast<double>(RobotConstants::ENCODER_STEPS_PER_MM) /
                                          static_cast<double>(CONV_FACTOR_M_TO_MM); /* [m] */
        double deltaPosM = pos - lastPos;                                           /* [m] */

        /* Protect against delta position overflow (16-bit). */
        if (0.0F <= deltaPosM)
        {
            if (OVERFLOW_DELTA_POS <= deltaPosM)
            {
                lastPos += OVERFLOW_DELTA_POS;
            }
        }
        else
        {
            if (OVERFLOW_DELTA_POS <= (-deltaPosM))
            {
                lastPos -= OVERFLOW_DELTA_POS;
            }
        }
    }
}

int16_t Encoders::calculateSteps(double lastPos, double pos) const
{
    int16_t steps = 0; /* [steps] */

    /* Position sensor provides NAN until the first simulation step was perfomed. */
    if (false == isnan(pos))
    {
        const double CONV_FACTOR_M_TO_MM = 1000.0F;
        double       deltaPosM           = pos - lastPos;                                             /* [m] */
        double       deltaPosMM          = deltaPosM * CONV_FACTOR_M_TO_MM;                           /* [mm] */
        double encoderSteps = deltaPosMM * static_cast<double>(RobotConstants::ENCODER_STEPS_PER_MM); /* [steps] */

        steps = static_cast<int16_t>(encoderSteps); /* [steps] */
    }

    return steps;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
