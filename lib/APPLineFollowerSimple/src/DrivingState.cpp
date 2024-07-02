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
#include <StateMachine.h>
#include "ReadyState.h"
#include "ParameterSets.h"
#include <Util.h>

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

const uint16_t DrivingState::SENSOR_VALUE_MAX = Board::getInstance().getLineSensors().getSensorValueMax();

/* Calculate the position set point to be generic. */
const int16_t DrivingState::POSITION_SET_POINT =
    (SENSOR_VALUE_MAX * (Board::getInstance().getLineSensors().getNumLineSensors() - 1)) / 2;

/* Initialize the required sensor IDs to be generic. */
const uint8_t DrivingState::SENSOR_ID_MOST_LEFT = 0U;
const uint8_t DrivingState::SENSOR_ID_MIDDLE    = (Board::getInstance().getLineSensors().getNumLineSensors() - 1U) / 2U;
const uint8_t DrivingState::SENSOR_ID_MOST_RIGHT = Board::getInstance().getLineSensors().getNumLineSensors() - 1U;

/* Initialize the position values used by the algorithmic. */
const int16_t DrivingState::POSITION_MIDDLE_MIN = POSITION_SET_POINT - (SENSOR_VALUE_MAX / 2);
const int16_t DrivingState::POSITION_MIDDLE_MAX = POSITION_SET_POINT + (SENSOR_VALUE_MAX / 2);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void DrivingState::entry()
{
    IBoard&                            board    = Board::getInstance();
    IDisplay&                          display  = board.getDisplay();
    const ParameterSets::ParameterSet& parSet   = ParameterSets::getInstance().getParameterSet();
    IMotors&                           motors   = board.getMotors();
    const int16_t                      maxSpeed = motors.getMaxSpeed(); /* [digits] */

    m_observationTimer.start(OBSERVATION_DURATION);
    m_pidProcessTime.start(0); /* Immediate */
    m_lineStatus              = LINE_STATUS_NO_START_LINE_DETECTED;
    m_trackStatus             = TRACK_STATUS_NORMAL; /* Assume that the robot is placed on track. */
    m_isTrackLost             = false;               /* Assume that the robot is placed on track. */
    m_isStartStopLineDetected = false;

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
    IBoard&       board                   = Board::getInstance();
    ILineSensors& lineSensors             = Board::getInstance().getLineSensors();
    IMotors&      motors                  = board.getMotors();
    TrackStatus   nextTrackStatus         = m_trackStatus;
    bool          allowNegativeMotorSpeed = true;

    /* Get the position of the line and each sensor value. */
    int16_t         position         = lineSensors.readLine();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues();
    uint8_t         numLineSensors   = lineSensors.getNumLineSensors();
    int16_t         position3        = 0;
    bool            isTrackLost      = isNoLineDetected(lineSensorValues, numLineSensors);

    /* ========================================================================
     * Evaluate the situation based on the sensor values.
     * ========================================================================
     */
    nextTrackStatus = evaluateSituation(lineSensorValues, numLineSensors, position, isTrackLost);

    /* ========================================================================
     * Initiate measures depended on the situation.
     * ========================================================================
     */
    processSituation(position, allowNegativeMotorSpeed, nextTrackStatus, position3);

    /* If the tracks status changes, the PID integral and derivative part
     * will be reset to provide a smooth reaction.
     */
    if (m_trackStatus != nextTrackStatus)
    {
        m_pidCtrl.clear();
    }

    /* ========================================================================
     * Handle start-/stop-line actions.
     * ========================================================================
     */
    if ((TRACK_STATUS_START_STOP_LINE != m_trackStatus) && (TRACK_STATUS_START_STOP_LINE == nextTrackStatus))
    {
        /* Start line detected? */
        if (LINE_STATUS_NO_START_LINE_DETECTED == m_lineStatus)
        {
            /* Measure the lap time and use as start point the detected start line. */
            m_lapTime.start(0);

            m_lineStatus = LINE_STATUS_START_LINE_DETECTED;
        }
        /* Stop line detected. */
        else
        {
            /* Calculate lap time and show it. */
            ReadyState::getInstance().setLapTime(m_lapTime.getCurrentDuration());

            m_lineStatus = LINE_STATUS_STOP_LINE_DETECTED;

            /* Overwrite track status. */
            nextTrackStatus = TRACK_STATUS_FINISHED;
        }

        /* Notify user about start-/stop-line detection. */
        Sound::playBeep();
    }

    /* ========================================================================
     * Handle track lost or back on track actions.
     * ========================================================================
     */

    /* Track lost just in this process cycle? */
    if ((false == m_isTrackLost) && (true == isTrackLost))
    {
        /* Notify user by yellow LED. */
        Board::getInstance().getYellowLed().enable(true);
    }
    /* Track found again just in this process cycle? */
    else if ((true == m_isTrackLost) && (false == isTrackLost))
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
        motors.setSpeeds(0, 0);

        Sound::playAlarm(); /* Blocking! */

        /* Clear lap time. */
        ReadyState::getInstance().setLapTime(0);

        /* Overwrite track status. */
        nextTrackStatus = TRACK_STATUS_FINISHED;
    }

    /* ========================================================================
     * Handle driving based on position or normal stop condition.
     * ========================================================================
     */

    /* Periodically adapt driving and check the abort conditions, except
     * the round is finished.
     */
    if (TRACK_STATUS_FINISHED != nextTrackStatus)
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
        motors.setSpeeds(0, 0);

        /* Change to ready state. */
        sm.setState(&ReadyState::getInstance());
    }

    /* Take over values for next cycle. */
    m_trackStatus  = nextTrackStatus;
    m_isTrackLost  = isTrackLost;
    m_lastPosition = position;
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
    m_lineStatus(LINE_STATUS_NO_START_LINE_DETECTED),
    m_trackStatus(TRACK_STATUS_NORMAL),
    m_isStartStopLineDetected(false),
    m_lastSensorIdSawTrack(SENSOR_ID_MIDDLE),
    m_lastPosition(0),
    m_isTrackLost(false)
{
}

DrivingState::TrackStatus DrivingState::evaluateSituation(const uint16_t* lineSensorValues, uint8_t length,
                                                          int16_t position, bool isTrackLost) const
{
    TrackStatus nextTrackStatus = m_trackStatus;

    /* Driving over start-/stop-line? */
    if (TRACK_STATUS_START_STOP_LINE == m_trackStatus)
    {
        /* Left the start-/stop-line?
         * If the robot is not exact on the start-/stop-line, the calculated position
         * may misslead. Therefore additional the most left and right sensor values
         * are evaluated too.
         */
        if ((POSITION_MIDDLE_MIN <= position) && (POSITION_MIDDLE_MAX >= position) &&
            (LINE_SENSOR_ON_TRACK_MIN_VALUE > lineSensorValues[SENSOR_ID_MOST_LEFT]) &&
            (LINE_SENSOR_ON_TRACK_MIN_VALUE > lineSensorValues[SENSOR_ID_MOST_RIGHT]))
        {
            nextTrackStatus = TRACK_STATUS_NORMAL;
        }
    }
    /* Is the start-/stop-line detected? */
    else if (true == isStartStopLineDetected(lineSensorValues, length))
    {
        nextTrackStatus = TRACK_STATUS_START_STOP_LINE;
    }
    /* Is the track lost or just a gap in the track? */
    else if (true == isTrackLost)
    {
        const int16_t POS_MIN = POSITION_SET_POINT - SENSOR_VALUE_MAX;
        const int16_t POS_MAX = POSITION_SET_POINT + SENSOR_VALUE_MAX;

        /* If its a gap in the track, last position will be well. */
        if ((POS_MIN <= m_lastPosition) && (POS_MAX > m_lastPosition))
        {
            nextTrackStatus = TRACK_STATUS_TRACK_LOST_BY_GAP;
        }
        else
        {
            /* In any other case, route the calculated position through and
             * hope. It will be the position of the most left sensor or the
             * most right sensor.
             */
            nextTrackStatus = TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE;
        }
    }
    /* Nothing special. */
    else
    {
        /* Just follow the line by calculated position. */
        nextTrackStatus = TRACK_STATUS_NORMAL;
    }

    return nextTrackStatus;
}

bool DrivingState::isStartStopLineDetected(const uint16_t* lineSensorValues, uint8_t length) const
{
    bool           isDetected  = false;
    const uint32_t LINE_MAX_30 = (SENSOR_VALUE_MAX * 3U) / 10U; /* 30 % of max. value */
    const uint32_t LINE_MAX_70 = (SENSOR_VALUE_MAX * 7U) / 10U; /* 70 % of max. value */

    /*
     * ===     =     ===
     *   +   + + +   +
     *   L     M     R
     */
    if ((LINE_MAX_30 <= lineSensorValues[SENSOR_ID_MOST_LEFT]) &&
        (LINE_MAX_70 > lineSensorValues[SENSOR_ID_MIDDLE - 1U]) &&
        (LINE_MAX_70 <= lineSensorValues[SENSOR_ID_MIDDLE]) &&
        (LINE_MAX_70 > lineSensorValues[SENSOR_ID_MIDDLE + 1U]) &&
        (LINE_MAX_30 <= lineSensorValues[SENSOR_ID_MOST_RIGHT]))
    {
        isDetected = true;
    }

    return isDetected;
}

bool DrivingState::isNoLineDetected(const uint16_t* lineSensorValues, uint8_t length) const
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
        if (LINE_SENSOR_ON_TRACK_MIN_VALUE <= lineSensorValues[idx])
        {
            isDetected = false;
            break;
        }
    }

    return isDetected;
}

void DrivingState::processSituation(int16_t& position, bool& allowNegativeMotorSpeed, TrackStatus trackStatus,
                                    int16_t position3)
{
    switch (trackStatus)
    {
    case TRACK_STATUS_NORMAL:
        /* The position is used by default to follow the line. */
        break;

    case TRACK_STATUS_START_STOP_LINE:
        /* Use the inner sensors only for driving to avoid jerky movements, caused
         * by the most left or right sensor.
         */
        position = position3;

        /* Avoid that the robot turns in place. */
        allowNegativeMotorSpeed = false;
        break;

    case TRACK_STATUS_TRACK_LOST_BY_GAP:
        /* Overwrite the positin to drive straight forward in hope to see the
         * line again.
         */
        position = POSITION_SET_POINT;

        /* Avoid that the robots turns. */
        allowNegativeMotorSpeed = false;
        break;

    case TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE:
        /* If the track is lost by a robot manoeuvre and not becase the track
         * has a gap, the last position given by most left or right sensor
         * will be automatically used to find the track again.
         *
         * See ILineSensors::readLine() description regarding the behaviour in
         * case the line is not detected anymore.
         */
        break;

    case TRACK_STATUS_FINISHED:
        /* Nothing to do. */
        break;

    default:
        /* Should never happen. */
        break;
    }
}

void DrivingState::adaptDriving(int16_t position, bool allowNegativeMotorSpeed)
{
    IBoard&       board           = Board::getInstance();
    IMotors&      motors          = board.getMotors();
    const int16_t MAX_MOTOR_SPEED = motors.getMaxSpeed(); /* [digits] */
    const int16_t MIN_MOTOR_SPEED = (false == allowNegativeMotorSpeed) ? 0 : (-MAX_MOTOR_SPEED);
    int16_t       speedDifference = 0; /* [digits] */
    int16_t       leftSpeed       = 0; /* [digits] */
    int16_t       rightSpeed      = 0; /* [digits] */

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

    motors.setSpeeds(leftSpeed, rightSpeed);
}

bool DrivingState::isAbortRequired()
{
    bool isAbort = false;

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
