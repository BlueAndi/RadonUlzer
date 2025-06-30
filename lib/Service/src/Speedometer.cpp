/* MIT License
 *
 * Copyright (c) 2023 - 2025 Andreas Merkle <web@blue-andi.de>
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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Speedometer.h>
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

Speedometer Speedometer::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Speedometer::process()
{
    IMotors&      motors         = Board::getInstance().getMotors();
    uint32_t      timestamp      = millis();                       /* [ms] */
    int32_t       diffStepsLeft  = m_relEncoders.getCountsLeft();  /* [steps] */
    int32_t       diffStepsRight = m_relEncoders.getCountsRight(); /* [steps] */
    const int32_t ONE_SECOND     = 1000;                           /* 1s in ms */
    bool          resetLeft      = false;
    bool          resetRight     = false;

    if (0 == motors.getLeftSpeed())
    {
        resetLeft = true;
    }
    else
    {
        Direction currentDrivingDirection = getDirectionLeft();

        if (currentDrivingDirection != m_lastDirectionLeft)
        {
            resetLeft = true;
        }

        m_lastDirectionLeft = currentDrivingDirection;
    }

    if (0 == motors.getRightSpeed())
    {
        resetRight = true;
    }
    else
    {
        Direction currentDrivingDirection = getDirectionRight();

        if (currentDrivingDirection != m_lastDirectionRight)
        {
            resetRight = true;
        }

        m_lastDirectionRight = currentDrivingDirection;
    }

    /* If a motor is stopped, it is assumed that the robot has no movement on the
     * corresponding motor side.
     * All other effects which may lead the robot to move are ignored for simplicity.
     * If the application needs more accuracy, the application shall handle that
     * by its own.
     *
     * If the driving direction changed, the relative encoders will be reset to
     * avoid using a invalid driving distance for speed calculation.
     */
    if (true == resetLeft)
    {
        m_linearSpeedLeft = 0;
        m_timestampLeft   = timestamp;

        m_relEncoders.clearLeft();
    }
    /* Moved long enough to be able to calculate the linear speed? */
    else if (MIN_ENCODER_COUNT <= abs(diffStepsLeft))
    {
        uint32_t dTimeLeft = timestamp - m_timestampLeft; /* [ms] */

        m_linearSpeedLeft = diffStepsLeft * ONE_SECOND / static_cast<int32_t>(dTimeLeft);
        m_timestampLeft   = timestamp;

        m_relEncoders.clearLeft();
    }
    else
    {
        ;
    }

    if (true == resetRight)
    {
        m_linearSpeedRight = 0;
        m_timestampRight   = timestamp;

        m_relEncoders.clearRight();
    }
    /* Moved long enough to be able to calculate the linear speed? */
    else if (MIN_ENCODER_COUNT <= abs(diffStepsRight))
    {
        uint32_t dTimeRight = timestamp - m_timestampRight; /* [ms] */

        m_linearSpeedRight = diffStepsRight * ONE_SECOND / static_cast<int32_t>(dTimeRight);
        m_timestampRight   = timestamp;

        m_relEncoders.clearRight();
    }
    else
    {
        ;
    }
    }

int16_t Speedometer::getLinearSpeedCenter() const
{
    int32_t linearSpeedLeft   = static_cast<int32_t>(m_linearSpeedLeft);
    int32_t linearSpeedRight  = static_cast<int32_t>(m_linearSpeedRight);
    int32_t linearSpeedCenter = (linearSpeedLeft + linearSpeedRight) / 2;

    return linearSpeedCenter;
}

int16_t Speedometer::getLinearSpeedLeft() const
{
    return m_linearSpeedLeft;
}

int16_t Speedometer::getLinearSpeedRight() const
{
    return m_linearSpeedRight;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

Speedometer::Direction Speedometer::getDirectionLeft()
{
    IMotors& motors = Board::getInstance().getMotors();
    int16_t  speed  = motors.getLeftSpeed();

    return getDirectionByMotorSpeed(speed);
}

Speedometer::Direction Speedometer::getDirectionRight()
{
    IMotors& motors = Board::getInstance().getMotors();
    int16_t  speed  = motors.getRightSpeed();

    return getDirectionByMotorSpeed(speed);
}

Speedometer::Direction Speedometer::getDirectionByMotorSpeed(int16_t motorSpeed)
{
    Direction direction = DIRECTION_STOPPED;

    if (0 < motorSpeed)
    {
        direction = DIRECTION_POSTIVE;
    }
    else if (0 > motorSpeed)
    {
        direction = DIRECTION_NEGATIVE;
    }
    else
    {
        ;
    }

    return direction;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
