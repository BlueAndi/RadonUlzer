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
 * @brief  Driving state
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DrivingState.h"
#include <Board.h>
#include <Sound.h>
#include <DifferentialDrive.h>
#include <StateMachine.h>
#include <Odometry.h>
#include "ReadyState.h"
#include "ParameterSets.h"

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

/* Calculate the position set point to be generic. */
const int16_t DrivingState::POSITION_SET_POINT = (Board::getInstance().getLineSensors().getSensorValueMax() *
                                                  (Board::getInstance().getLineSensors().getNumLineSensors() - 1)) /
                                                 2;

/* Initialize the required sensor IDs to be generic. */
const uint8_t DrivingState::SENSOR_ID_MOST_LEFT  = 0U;
const uint8_t DrivingState::SENSOR_ID_MIDDLE     = Board::getInstance().getLineSensors().getNumLineSensors() / 2U;
const uint8_t DrivingState::SENSOR_ID_MOST_RIGHT = Board::getInstance().getLineSensors().getNumLineSensors() - 1U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void DrivingState::entry()
{
    IDisplay&                          display   = Board::getInstance().getDisplay();
    const ParameterSets::ParameterSet& parSet    = ParameterSets::getInstance().getParameterSet();
    DifferentialDrive&                 diffDrive = DifferentialDrive::getInstance();
    const int16_t                      maxSpeed  = diffDrive.getMaxMotorSpeed(); /* [steps/s] */

    m_observationTimer.start(OBSERVATION_DURATION);
    m_pidProcessTime.start(0); /* Immediate */
    m_lineStatus              = LINE_STATUS_NO_START_STOP_LINE_DETECTED;
    m_isStartStopLineDetected = false;
    m_trackStatus             = TRACK_STATUS_ON_TRACK; /* Assume that the robot is placed on track. */

    /* Configure PID controller with selected parameter set. */
    m_topSpeed = parSet.topSpeed;
    m_pidCtrl.clear();
    m_pidCtrl.setPFactor(parSet.kPNumerator, parSet.kPDenominator);
    m_pidCtrl.setIFactor(parSet.kINumerator, parSet.kIDenominator);
    m_pidCtrl.setDFactor(parSet.kDNumerator, parSet.kDDenominator);
    m_pidCtrl.setSampleTime(PID_PROCESS_PERIOD);
    m_pidCtrl.setLimits(-maxSpeed, maxSpeed);
    m_pidCtrl.setDerivativeOnMeasurement(false);

    display.clear();
    display.print("DRV");
}

void DrivingState::process(StateMachine& sm)
{
    ILineSensors&      lineSensors             = Board::getInstance().getLineSensors();
    DifferentialDrive& diffDrive               = DifferentialDrive::getInstance();
    TrackStatus        nextTrackStatus         = TRACK_STATUS_ON_TRACK;
    bool               startStopLineDetected   = false;
    bool               allowNegativeMotorSpeed = false;

    /* Get the position of the line and each sensor value. */
    int16_t         position         = lineSensors.readLine();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues();
    uint8_t         numLineSensors   = lineSensors.getNumLineSensors();

    /* ========================================================================
     * Evaluate the situation based on the sensor values.
     * ========================================================================
     */
    if (true == isStartStopLineDetected(lineSensorValues, numLineSensors))
    {
        /* Update the line status on change to differ between detection of
         * the start line and the end line.
         */
        if (false == m_isStartStopLineDetected)
        {
            /* Start line detected? */
            if (LINE_STATUS_NO_START_STOP_LINE_DETECTED == m_lineStatus)
            {
                m_lineStatus = LINE_STATUS_START_LINE_DETECTED;

                /* Measure the lap time and use as start point the detected start line. */
                m_lapTime.start(0);
            }
            /* Stop line detected. */
            else
            {
                m_lineStatus = LINE_STATUS_STOP_LINE_DETECTED;

                /* Calculate lap time and show it. */
                ReadyState::getInstance().setLapTime(m_lapTime.getCurrentDuration());
                nextTrackStatus = TRACK_STATUS_FINISHED;
            }

            Sound::playBeep();
        }

        startStopLineDetected = true;
    }
    /* Is it a 90° curve to the left? */
    else if (true == isRightAngleCurveToLeft(lineSensorValues, numLineSensors))
    {
        /* Enfore max. error in PID to turn hard left at place. */
        position                = 0;
        allowNegativeMotorSpeed = true;
    }
    /* Is it a 90° curve to the right? */
    else if (true == isRightAngleCurveToRight(lineSensorValues, numLineSensors))
    {
        /* Enfore max. error in PID to turn hard right at place. */
        position                = static_cast<int16_t>(numLineSensors - 1U) * 1000;
        allowNegativeMotorSpeed = true;
    }
    /* Is it a gap in the track? */
    else if (true == isGapDetected(lineSensorValues, numLineSensors))
    {
        const int16_t SENSOR_VALUE_MAX = Board::getInstance().getLineSensors().getSensorValueMax();
        const int16_t POS_MIN          = POSITION_SET_POINT - SENSOR_VALUE_MAX;
        const int16_t POS_MAX          = POSITION_SET_POINT + SENSOR_VALUE_MAX;

        /* If its a gap in the track, last position will be well. */
        if ((POS_MIN <= m_lastPosition) && (POS_MAX > m_lastPosition))
        {
            /* Drive straight forward in hope to see the line again. */
            position = POSITION_SET_POINT;
        }
        else
        {
            /* In any other case, route the calculated position through and
             * hope. It will be the position of the most left sensor or the
             * most right sensor.
             */
            ;
        }

        nextTrackStatus = TRACK_STATUS_LOST;
    }
    /* Nothing special. */
    else
    {
        /* Just follow the line by calculated position. */
        ;
    }

    m_isStartStopLineDetected = startStopLineDetected;
    m_lastPosition            = position;

    /* ========================================================================
     * Handle track lost or back on track actions.
     * ========================================================================
     */

    /* Track lost just in this process cycle? */
    if ((TRACK_STATUS_LOST != m_trackStatus) && (TRACK_STATUS_LOST == nextTrackStatus))
    {
        /* Notify user by yellow LED. */
        Board::getInstance().getYellowLed().enable(true);

        /* Set mileage to 0, to be able to measure the max. distance, till
         * the track must be found again.
         */
        Odometry::getInstance().clearMileage();
    }
    /* Track found again just in this process cycle? */
    else if ((TRACK_STATUS_LOST == m_trackStatus) && (TRACK_STATUS_LOST != nextTrackStatus))
    {
        Board::getInstance().getYellowLed().enable(false);
    }
    else
    {
        ;
    }

    /* ========================================================================
     * Handle the abort conditions which will cause a alarm stop.
     * ========================================================================
     */

    /* Check whether the abort conditions are true. */
    if (true == isAbortRequired())
    {
        /* Stop motors immediately. Don't move this to a later position,
         * as this would extend the driven length.
         */
        diffDrive.setLinearSpeed(0, 0);

        Sound::playAlarm(); /* Blocking! */

        /* Clear lap time. */
        ReadyState::getInstance().setLapTime(0);

        m_trackStatus = TRACK_STATUS_FINISHED;
    }

    /* ========================================================================
     * Handle driving based on position or normal stop condition.
     * ========================================================================
     */

    /* Periodically adapt driving and check the abort conditions, except
     * the round is finished.
     */
    if (TRACK_STATUS_FINISHED != m_trackStatus)
    {
        /* Process the line follower PID controller periodically to adapt driving. */
        if (true == m_pidProcessTime.isTimeout())
        {
            adaptDriving(position, allowNegativeMotorSpeed);

            m_pidProcessTime.start(PID_PROCESS_PERIOD);
        }
    }
    /* Finished. */
    else
    {
        /* Stop motors immediately. Don't move this to a later position,
         * as this would extend the driven length.
         */
        diffDrive.setLinearSpeed(0, 0);

        /* Change to ready state. */
        sm.setState(&ReadyState::getInstance());
    }

    m_trackStatus = nextTrackStatus;
}

void DrivingState::exit()
{
    m_observationTimer.stop();
    Board::getInstance().getYellowLed().enable(false);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

DrivingState::DrivingState() :
    m_observationTimer(),
    m_lapTime(),
    m_pidProcessTime(),
    m_pidCtrl(),
    m_topSpeed(0),
    m_lineStatus(LINE_STATUS_NO_START_STOP_LINE_DETECTED),
    m_trackStatus(TRACK_STATUS_ON_TRACK),
    m_isStartStopLineDetected(false),
    m_lastSensorIdSawTrack(SENSOR_ID_MIDDLE),
    m_lastPosition(0)
{
}

bool DrivingState::isStartStopLineDetected(const uint16_t* lineSensorValues, uint8_t length)
{
    bool isDetected = false;

    /*
     * ===     =     ===
     *   +   + + +   +
     *   L     M     R
     */
    if ((LINE_SENSOR_ON_TRACK_VALUE <= lineSensorValues[SENSOR_ID_MOST_LEFT]) &&
        (LINE_SENSOR_ON_TRACK_VALUE <= lineSensorValues[SENSOR_ID_MIDDLE]) &&
        (LINE_SENSOR_ON_TRACK_VALUE <= lineSensorValues[SENSOR_ID_MOST_RIGHT]))
    {
        isDetected = true;
    }

    return isDetected;
}

bool DrivingState::isRightAngleCurveToLeft(const uint16_t* lineSensorValues, uint8_t length)
{
    bool    isDetected = true;
    uint8_t idx        = SENSOR_ID_MOST_LEFT;

    /*
     * =========
     *   +   + + +   +
     *   L     M     R
     */
    for (idx = SENSOR_ID_MOST_LEFT; idx <= SENSOR_ID_MIDDLE; ++idx)
    {
        if (LINE_SENSOR_ON_TRACK_VALUE > lineSensorValues[idx])
        {
            isDetected = false;
            break;
        }
    }

    return isDetected;
}

bool DrivingState::isRightAngleCurveToRight(const uint16_t* lineSensorValues, uint8_t length)
{
    bool    isDetected = true;
    uint8_t idx        = SENSOR_ID_MOST_RIGHT;

    /*
     *         =========
     *   +   + + +   +
     *   L     M     R
     */
    for (idx = SENSOR_ID_MIDDLE; idx <= SENSOR_ID_MOST_RIGHT; ++idx)
    {
        if (LINE_SENSOR_ON_TRACK_VALUE > lineSensorValues[idx])
        {
            isDetected = false;
            break;
        }
    }

    return isDetected;
}

bool DrivingState::isGapDetected(const uint16_t* lineSensorValues, uint8_t length)
{
    bool    isDetected = true;
    uint8_t idx        = SENSOR_ID_MOST_RIGHT;

    /*
     *
     *   +   + + +   +
     *   L     M     R
     */
    for (idx = SENSOR_ID_MOST_LEFT; idx <= SENSOR_ID_MOST_RIGHT; ++idx)
    {
        if (LINE_SENSOR_ON_TRACK_VALUE <= lineSensorValues[idx])
        {
            isDetected = false;
            break;
        }
    }

    return isDetected;
}

void DrivingState::adaptDriving(int16_t position, bool allowNegativeMotorSpeed)
{
    DifferentialDrive& diffDrive       = DifferentialDrive::getInstance();
    const int16_t      MAX_MOTOR_SPEED = diffDrive.getMaxMotorSpeed();
    const int16_t      MIN_MOTOR_SPEED = (false == allowNegativeMotorSpeed) ? 0 : (-MAX_MOTOR_SPEED);
    int16_t            speedDifference = 0; /* [steps/s] */
    int16_t            leftSpeed       = 0; /* [steps/s] */
    int16_t            rightSpeed      = 0; /* [steps/s] */

    /* Our "error" is how far we are away from the center of the
     * line, which corresponds to position (max. line sensor value multiplied
     * with sensor index).
     *
     * Get motor speed difference using PID terms.
     */
    speedDifference = m_pidCtrl.calculate(POSITION_SET_POINT, position);

    /* Get individual motor speeds.  The sign of speedDifference
     * determines if the robot turns left or right.
     */
    leftSpeed  = m_topSpeed - speedDifference;
    rightSpeed = m_topSpeed + speedDifference;

    /* Constrain our motor speeds to be between 0 and maxSpeed.
     * One motor will always be turning at maxSpeed, and the other
     * will be at maxSpeed-|speedDifference| if that is positive,
     * else it will be stationary. For some applications, you
     * might want to allow the motor speed to go negative so that
     * it can spin in reverse.
     */
    leftSpeed  = constrain(leftSpeed, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);
    rightSpeed = constrain(rightSpeed, MIN_MOTOR_SPEED, MAX_MOTOR_SPEED);

    diffDrive.setLinearSpeed(leftSpeed, rightSpeed);
}

bool DrivingState::isAbortRequired()
{
    bool isAbort = false;

    /* If track is lost over a certain distance, abort driving. */
    if (TRACK_STATUS_LOST == m_trackStatus)
    {
        /* Max. distance driven, but track still not found? */
        if (MAX_DISTANCE < Odometry::getInstance().getMileageCenter())
        {
            isAbort = true;
        }
    }

    /* If track is not finished over a certain time, abort driving. */
    if (TRACK_STATUS_FINISHED != m_trackStatus)
    {
        if (true == m_observationTimer.isTimeout())
        {
            isAbort = true;
        }
    }

    return isAbort;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
