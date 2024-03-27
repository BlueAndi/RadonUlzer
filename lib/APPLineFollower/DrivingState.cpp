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

#ifdef DEBUG_ALGORITHM
static void logCsvDataTitles(uint8_t length);
static void logCsvDataTimestamp();
static void logCsvData(const uint16_t* lineSensorValues, uint8_t length, int16_t pos, int16_t pos3, bool isPos3Valid);
static void logCsvDataTrackStatus(DrivingState::TrackStatus trackStatus);
static void logCsvDataSpeed(int16_t leftSpeed, int16_t rightSpeed);
#endif /* DEBUG_ALGORITHM */

/******************************************************************************
 * Local Variables
 *****************************************************************************/

#ifdef DEBUG_ALGORITHM
static int16_t gSpeedLeft  = 0;
static int16_t gSpeedRight = 0;
#endif /* DEBUG_ALGORITHM */

/* Calculate the position set point to be generic. */
const int16_t DrivingState::POSITION_SET_POINT = (Board::getInstance().getLineSensors().getSensorValueMax() *
                                                  (Board::getInstance().getLineSensors().getNumLineSensors() - 1)) /
                                                 2;

/* Initialize the required sensor IDs to be generic. */
const uint8_t DrivingState::SENSOR_ID_MOST_LEFT  = 0U;
const uint8_t DrivingState::SENSOR_ID_MIDDLE     = Board::getInstance().getLineSensors().getNumLineSensors() / 2U;
const uint8_t DrivingState::SENSOR_ID_MOST_RIGHT = Board::getInstance().getLineSensors().getNumLineSensors() - 1U;

/* Initialize the position values used by the algorithmic. */
const int16_t DrivingState::POSITION_MIN = 0;
const int16_t DrivingState::POSITION_MAX =
    static_cast<int16_t>(Board::getInstance().getLineSensors().getNumLineSensors() - 1U) * 1000;
const int16_t DrivingState::POSITION_MIDDLE_MIN =
    POSITION_SET_POINT - (Board::getInstance().getLineSensors().getSensorValueMax() / 2);
const int16_t DrivingState::POSITION_MIDDLE_MAX =
    POSITION_SET_POINT + (Board::getInstance().getLineSensors().getSensorValueMax() / 2);
const int16_t DrivingState::POS_DIFF_SHARP_CURVE_THRESHOLD = 834;

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
    m_lineStatus              = LINE_STATUS_NO_START_LINE_DETECTED;
    m_trackStatus             = TRACK_STATUS_NORMAL; /* Assume that the robot is placed on track. */
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

#ifdef DEBUG_ALGORITHM
    logCsvDataTitles(Board::getInstance().getLineSensors().getNumLineSensors());
#endif /* DEBUG_ALGORITHM */
}

void DrivingState::process(StateMachine& sm)
{
    ILineSensors&      lineSensors             = Board::getInstance().getLineSensors();
    DifferentialDrive& diffDrive               = DifferentialDrive::getInstance();
    TrackStatus        nextTrackStatus         = m_trackStatus;
    bool               allowNegativeMotorSpeed = true;

    /* Get the position of the line and each sensor value. */
    int16_t         position         = lineSensors.readLine();
    const uint16_t* lineSensorValues = lineSensors.getSensorValues();
    uint8_t         numLineSensors   = lineSensors.getNumLineSensors();

    int16_t position3        = 0;
    bool    isPosition3Valid = calcPosition3(position3, lineSensorValues, numLineSensors);
    int16_t posDiff          = 0;

    if (true == isPosition3Valid)
    {
        posDiff = position - position3;
    }

#ifdef DEBUG_ALGORITHM
    logCsvDataTimestamp();
    logCsvData(lineSensorValues, numLineSensors, position, position3, isPosition3Valid);
#endif /* DEBUG_ALGORITHM */

    /* ========================================================================
     * Evaluate the situation based on the sensor values.
     * ========================================================================
     */

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

        allowNegativeMotorSpeed = false; /* Avoid that the robot turns in place. */
    }
    /* Is the start-/stop-line detected? */
    else if (true == isStartStopLineDetected(lineSensorValues, numLineSensors))
    {
        /* Start line detected? */
        if (LINE_STATUS_NO_START_LINE_DETECTED == m_lineStatus)
        {
            /* Measure the lap time and use as start point the detected start line. */
            m_lapTime.start(0);

            m_lineStatus    = LINE_STATUS_START_LINE_DETECTED;
            nextTrackStatus = TRACK_STATUS_START_STOP_LINE;
        }
        /* Stop line detected. */
        else
        {
            /* Calculate lap time and show it. */
            ReadyState::getInstance().setLapTime(m_lapTime.getCurrentDuration());

            m_lineStatus    = LINE_STATUS_STOP_LINE_DETECTED;
            nextTrackStatus = TRACK_STATUS_FINISHED;
        }

        Sound::playBeep();

        allowNegativeMotorSpeed = false; /* Avoid that the robot turns in place. */
    }
    /* Sharp curve to the left expected? */
    else if (TRACK_STATUS_SHARP_CURVE_LEFT == m_trackStatus)
    {
        /* Turn just before the robot leaves the line. */
        if (POSITION_MIDDLE_MAX < position3)
        {
            nextTrackStatus = TRACK_STATUS_SHARP_CURVE_LEFT_TURN;
            position        = POSITION_MIN; /* Enfore max. error in PID to turn sharp left at place. */
        }
        /* The calculated position can not be used anymore, because the left sensors
         * may overlap the line.
         */
        else
        {
            position = position3;
        }
    }
    /* Sharp curve to the right expected? */
    else if (TRACK_STATUS_SHARP_CURVE_RIGHT == m_trackStatus)
    {
        /* Turn just before the robot leaves the line. */
        if (POSITION_MIDDLE_MIN > position3)
        {
            nextTrackStatus = TRACK_STATUS_SHARP_CURVE_RIGHT_TURN;
            position        = POSITION_MAX; /* Enfore max. error in PID to turn sharp right at place. */
        }
        /* The calculated position can not be used anymore, because the left sensors
         * may overlap the line.
         */
        else
        {
            position = position3;
        }
    }
    /* Sharp curve to the left turning now! */
    else if (TRACK_STATUS_SHARP_CURVE_LEFT_TURN == m_trackStatus)
    {
        if ((POSITION_MIDDLE_MIN <= position3) && (POSITION_MIDDLE_MAX >= position3))
        {
            nextTrackStatus = TRACK_STATUS_NORMAL;
        }
        else
        {
            position = POSITION_MIN; /* Enfore max. error in PID to turn sharp left at place. */
        }
    }
    /* Sharp curve to the right turning now! */
    else if (TRACK_STATUS_SHARP_CURVE_RIGHT_TURN == m_trackStatus)
    {
        if ((POSITION_MIDDLE_MIN <= position3) && (POSITION_MIDDLE_MAX >= position3))
        {
            nextTrackStatus = TRACK_STATUS_NORMAL;
        }
        else
        {
            position = POSITION_MAX; /* Enfore max. error in PID to turn sharp right at place. */
        }
    }
    /* Sharp curve to left detected? */
    else if ((-POS_DIFF_SHARP_CURVE_THRESHOLD >= posDiff) && (POSITION_MIDDLE_MIN <= position3) &&
             (POSITION_MIDDLE_MAX >= position3))
    {
        nextTrackStatus = TRACK_STATUS_SHARP_CURVE_LEFT;
        position        = position3; /* Use only the inner sensors for position calculation. */
    }
    /* Sharp curve to right detected? */
    else if ((POS_DIFF_SHARP_CURVE_THRESHOLD <= posDiff) && (POSITION_MIDDLE_MIN <= position3) &&
             (POSITION_MIDDLE_MAX >= position3))
    {
        nextTrackStatus = TRACK_STATUS_SHARP_CURVE_RIGHT;
        position        = position3; /* Use only the inner sensors for position calculation. */
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
            position                = POSITION_SET_POINT;
            allowNegativeMotorSpeed = false; /* Avoid that the robots turns. */
            nextTrackStatus         = TRACK_STATUS_TRACK_LOST_BY_GAP;
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

    m_lastPosition = position;

    /* ========================================================================
     * Handle track lost or back on track actions.
     * ========================================================================
     */

    /* Track lost just in this process cycle? */
    if ((TRACK_STATUS_TRACK_LOST_BY_GAP != m_trackStatus) && (TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE != m_trackStatus) &&
        ((TRACK_STATUS_TRACK_LOST_BY_GAP == nextTrackStatus) ||
         (TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE == nextTrackStatus)))
    {
        /* Notify user by yellow LED. */
        Board::getInstance().getYellowLed().enable(true);

        /* Set mileage to 0, to be able to measure the max. distance, till
         * the track must be found again.
         */
        Odometry::getInstance().clearMileage();
    }
    /* Track found again just in this process cycle? */
    else if (((TRACK_STATUS_TRACK_LOST_BY_GAP == m_trackStatus) ||
              (TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE == m_trackStatus)) &&
             ((TRACK_STATUS_TRACK_LOST_BY_GAP != nextTrackStatus) &&
              (TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE != nextTrackStatus)))
    {
        Board::getInstance().getYellowLed().enable(false);
    }
    else
    {
        ;
    }

    /* Take over next track status. */
    m_trackStatus = nextTrackStatus;

#ifdef DEBUG_ALGORITHM
    logCsvDataTrackStatus(m_trackStatus);
#endif /* DEBUG_ALGORITHM */

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

#ifdef DEBUG_ALGORITHM
        logCsvDataSpeed(gSpeedLeft, gSpeedRight);
#endif /* DEBUG_ALGORITHM */
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

#ifdef DEBUG_ALGORITHM
    /* Next CSV data. */
    printf("\n");
#endif /* DEBUG_ALGORITHM */
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
    m_lastPosition(0)
{
}

bool DrivingState::calcPosition3(int16_t& position, const uint16_t* lineSensorValues, uint8_t length) const
{
    const int32_t WEIGHT      = Board::getInstance().getLineSensors().getSensorValueMax();
    bool          isValid     = true;
    int32_t       numerator   = 0U;
    int32_t       denominator = 0U;
    int32_t       idxBegin    = 1;
    int32_t       idxEnd      = length - 1;

    for (int32_t idx = idxBegin; idx < idxEnd; ++idx)
    {
        int32_t sensorValue = static_cast<int32_t>(lineSensorValues[idx]);

        numerator += idx * WEIGHT * sensorValue;
        denominator += sensorValue;
    }

    if (0 == denominator)
    {
        isValid = false;
    }
    else
    {
        position = numerator / denominator;
    }

    return isValid;
}

bool DrivingState::isStartStopLineDetected(const uint16_t* lineSensorValues, uint8_t length)
{
    bool           isDetected = false;
    const uint32_t LINE_MAX_30 =
        (Board::getInstance().getLineSensors().getSensorValueMax() * 30U) / 100U; /* 30 % of max. value */
    const uint32_t LINE_MAX_70 =
        (Board::getInstance().getLineSensors().getSensorValueMax() * 70U) / 100U; /* 70 % of max. value */

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
        if (LINE_SENSOR_ON_TRACK_MIN_VALUE <= lineSensorValues[idx])
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

#ifdef DEBUG_ALGORITHM
    gSpeedLeft  = leftSpeed;
    gSpeedRight = rightSpeed;
#endif /* DEBUG_ALGORITHM */

    diffDrive.setLinearSpeed(leftSpeed, rightSpeed);
}

bool DrivingState::isAbortRequired()
{
    bool isAbort = false;

    /* If track is lost over a certain distance, abort driving. */
    if ((TRACK_STATUS_TRACK_LOST_BY_GAP == m_trackStatus) || (TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE == m_trackStatus))
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

#ifdef DEBUG_ALGORITHM

/**
 * Log the titles of each column as CSV data.
 *
 * @param[in] length Number of line sensors.
 */
static void logCsvDataTitles(uint8_t length)
{
    uint8_t idx = 0;

    printf("Timestamp");

    while (length > idx)
    {
        printf(";Sensor %u", idx);

        ++idx;
    }

    printf(";Position;Position3;PosDiff;Scenario;Speed Left; Speed Right\n");
}

/**
 * Log the timestamp as CSV data.
 */
static void logCsvDataTimestamp()
{
    printf("%u;", millis());
}

/**
 * Log the sensor values, the calculated positions and the position difference
 * as CSV data.
 *
 * @param[in] lineSensorValues  The array of line sensor values.
 * @param[in] length            The number of line sensors.
 * @param[in] pos               The calculated position by all line sensors.
 * @param[in] pos3              The calculated position by the inner line sensors.
 * @param[in] isPos3Valid       Flag to see whether the inner line sensors position is valid or not.
 */
static void logCsvData(const uint16_t* lineSensorValues, uint8_t length, int16_t pos, int16_t pos3, bool isPos3Valid)
{
    uint8_t idx = 0;

    while (length > idx)
    {
        if (0 < idx)
        {
            printf(";");
        }

        printf("%u", lineSensorValues[idx]);

        ++idx;
    }

    printf(";%d;", pos);

    if (false == isPos3Valid)
    {
        printf(";;");
    }
    else
    {
        printf("%d;%d", pos3, pos - pos3);
    }
}

/**
 * Log the track status as CSV data.
 *
 * @param[in] trackStatus   The track status.
 */
static void logCsvDataTrackStatus(DrivingState::TrackStatus trackStatus)
{
    switch (trackStatus)
    {
    case DrivingState::TRACK_STATUS_NORMAL:
        printf(";\"Normal\"");
        break;

    case DrivingState::TRACK_STATUS_START_STOP_LINE:
        printf(";\"Start-/Stop-line\"");
        break;

    case DrivingState::TRACK_STATUS_SHARP_CURVE_LEFT:
        printf(";\"Sharp curve left\"");
        break;

    case DrivingState::TRACK_STATUS_SHARP_CURVE_RIGHT:
        printf(";\"Sharp curve right\"");
        break;

    case DrivingState::TRACK_STATUS_SHARP_CURVE_LEFT_TURN:
        printf(";\"Sharp curve left turn\"");
        break;

    case DrivingState::TRACK_STATUS_SHARP_CURVE_RIGHT_TURN:
        printf(";\"Sharp curve right turn\"");
        break;

    case DrivingState::TRACK_STATUS_TRACK_LOST_BY_GAP:
        printf(";\"Track lost by gap\"");
        break;

    case DrivingState::TRACK_STATUS_TRACK_LOST_BY_MANOEUVRE:
        printf(";\"Track lost by manoeuvre\"");
        break;

    case DrivingState::TRACK_STATUS_FINISHED:
        printf(";\"Track finished\"");
        break;

    default:
        printf(";\"?\"");
        break;
    }
}

/**
 * Log the motor speed set points as CSV data.
 *
 * @param[in] leftSpeed     Right motor speed set point in steps/s.
 * @param[in] rightSpeed    Left motor speed set point in step/s.
 */
static void logCsvDataSpeed(int16_t leftSpeed, int16_t rightSpeed)
{
    printf(";%d;%d", leftSpeed, rightSpeed);
}

#endif /* DEBUG_ALGORITHM */