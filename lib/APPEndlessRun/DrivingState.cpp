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
 * @brief  Calibration state
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

#include "StartupState.h"
#include "ParameterSets.h"
#include "ColorState.h"

// #include <stdio.h>
// #include <stdlib.h>

// using namespace std;
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

static bool SETUP_FLAG;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void DrivingState::entry()
{
    /* Setup PID and top speed once. */
    if (false == SETUP_FLAG)
    {
        const ParameterSets::ParameterSet& parSet    = ParameterSets::getInstance().getParameterSet();
        DifferentialDrive&                 diffDrive = DifferentialDrive::getInstance();
        const int16_t                      maxSpeed  = diffDrive.getMaxMotorSpeed(); /* [steps/s] */

        /* Choose parameter set 0 by default. */
        ParameterSets::getInstance().choose(0);

        m_observationTimer.start(OBSERVATION_DURATION);
        m_pidProcessTime.start(0); /* Immediate */
        m_lineStatus  = LINE_STATUS_FIND_START_LINE;
        m_trackStatus = TRACK_STATUS_ON_TRACK; /* Assume that the robot is placed on track. */

        diffDrive.enable();

        /* Configure PID controller with selected parameter set. */
        m_topSpeed = parSet.topSpeed;
        m_pidCtrl.clear();
        m_pidCtrl.setPFactor(parSet.kPNumerator, parSet.kPDenominator);
        m_pidCtrl.setIFactor(parSet.kINumerator, parSet.kIDenominator);
        m_pidCtrl.setDFactor(parSet.kDNumerator, parSet.kDDenominator);
        m_pidCtrl.setSampleTime(PID_PROCESS_PERIOD);
        m_pidCtrl.setLimits(-maxSpeed, maxSpeed);
        m_pidCtrl.setDerivativeOnMeasurement(true);

        /* Save top speed to be used freely even after being modified. */
        KEEP_TOP_SPEED = m_topSpeed;

        SETUP_FLAG = true;
    }

    /* Periodic callback timer to check for new colors and refresh current coordinates. */
    m_callbackTimer.start(150U);

    /* DrivingState is marked with Red LED. */
    Board::getInstance().getRedLed().enable(true);
}

void DrivingState::process(StateMachine& sm)
{
    ILineSensors&      lineSensors = Board::getInstance().getLineSensors();
    DifferentialDrive& diffDrive   = DifferentialDrive::getInstance();
    int16_t            position    = 0;

    /* Get line position. */
    position = lineSensors.readLine();

    switch (m_trackStatus)
    {
    case TRACK_STATUS_ON_TRACK:
        processOnTrack(position, lineSensors.getSensorValues());

        /** If callback timer is done, refresh odometry coordinates. */
        if (true == m_callbackTimer.isTimeout())
        {
            Odometry::getInstance().getPosition(CURRENT_X_COORDINATE, CURRENT_Y_COORDINATE);

            // cout << "x:" << CURRENT_X_COORDINATE << " "
            //      << "y:" << CURRENT_Y_COORDINATE << endl;

            // m_callbackTimer.restart();
            sm.setState(&ColorState::getInstance());
        }
        break;

    case TRACK_STATUS_LOST:
        processTrackLost(position, lineSensors.getSensorValues());
        break;

    case TRACK_STATUS_FINISHED:
        /* Change to ready state. */
        sm.setState(&StartupState::getInstance());
        break;

    default:
        /* Fatal error */
        diffDrive.setLinearSpeed(0, 0);
        Sound::playAlarm();
        sm.setState(&StartupState::getInstance());
        break;
    }

    /* Max. time over? */
    if ((true == m_observationTimer.isTimeout()))
    {
        m_trackStatus = TRACK_STATUS_FINISHED;

        /* Stop motors immediately. Don't move this to a later position,
         * as this would extend the driven length.
         */
        diffDrive.setLinearSpeed(0, 0);

        Sound::playAlarm();
    }
}

void DrivingState::exit()
{
    Board::getInstance().getRedLed().enable(false);
    Board::getInstance().getYellowLed().enable(false);

    m_callbackTimer.stop();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DrivingState::processOnTrack(int16_t position, const uint16_t* lineSeonsorValues)
{
    if (nullptr == lineSeonsorValues)
    {
        return;
    }

    /* Track lost just now? */
    if (true == isTrackGapDetected(position))
    {
        m_trackStatus = TRACK_STATUS_LOST;

        /* Set mileage to 0, to be able to measure the max. distance, till
         * the track must be found again.
         */
        Odometry::getInstance().clearMileage();

        /* Show the operator that the track is lost visual. */
        Board::getInstance().getYellowLed().enable(true);
    }
    else
    {
        /* Detect start/endline */
        if (true == isStartEndLineDetected(lineSeonsorValues))
        {
            /* Start line detected? */
            if (LINE_STATUS_FIND_START_LINE == m_lineStatus)
            {
                m_lineStatus = LINE_STATUS_START_LINE_DETECTED;

                Sound::playBeep();
            }
            else
            {
                ;
            }
        }
        else if (LINE_STATUS_START_LINE_DETECTED == m_lineStatus)
        {
            m_lineStatus = LINE_STATUS_FIND_END_LINE;
        }
        else
        {
            ;
        }
        /* If track is not done, keep driving. */
        if (TRACK_STATUS_FINISHED != m_trackStatus)
        {
            if (true == m_pidProcessTime.isTimeout())
            {
                adaptDriving(position);

                m_pidProcessTime.start(PID_PROCESS_PERIOD);
            }
        }
    }
}

void DrivingState::processTrackLost(int16_t position, const uint16_t* lineSensorValues)
{
    DifferentialDrive& diffDrive = DifferentialDrive::getInstance();

    if (nullptr == lineSensorValues)
    {
        return;
    }

    /* Back on track? */
    if (false == isTrackGapDetected(position))
    {
        m_trackStatus = TRACK_STATUS_ON_TRACK;
        m_pidCtrl.resync();

        Board::getInstance().getYellowLed().enable(false);
    }
    /* Max. distance driven, but track still not found? */
    else if (MAX_DISTANCE < Odometry::getInstance().getMileageCenter())
    {
        /* Stop motors immediately. Don't move this to a later position,
         * as this would extend the driven length.
         */
        diffDrive.setLinearSpeed(0, 0);

        Sound::playAlarm();
        m_trackStatus = TRACK_STATUS_FINISHED;
    }
    else
    {
        /* Drive straight on. */
        diffDrive.setLinearSpeed(m_topSpeed, m_topSpeed);
    }
}

bool DrivingState::isStartEndLineDetected(const uint16_t* lineSensorValues)
{
    bool           isDetected                  = false;
    uint16_t       leftSensor                  = lineSensorValues[0];
    uint16_t       middleSensor                = (lineSensorValues[1] + lineSensorValues[2] + lineSensorValues[3]) / 3;
    uint16_t       rightSensor                 = lineSensorValues[4];
    const uint8_t  DEBOUNCE_CNT                = 3;
    const uint16_t LINE_SENSOR_OFF_TRACK_VALUE = 200;

    /* Note, the start-/end line detection must be debounced. Otherwise
     * especially in low speed use cases, the line may be in one cycle
     * detected, in the next not and then detected again. This would lead
     * to a start line detection and afterwards to a end line detection,
     * which would be wrong.
     *
     * Note the three sensors in the middle are handled as one sensor to
     * avoid detection problems with different kind of line widths.
     */
    if ((LINE_SENSOR_OFF_TRACK_VALUE <= leftSensor) && (LINE_SENSOR_OFF_TRACK_VALUE <= middleSensor) &&
        (LINE_SENSOR_OFF_TRACK_VALUE <= rightSensor))
    {
        if (DEBOUNCE_CNT > m_startEndLineDebounce)
        {
            ++m_startEndLineDebounce;
        }

        if (DEBOUNCE_CNT <= m_startEndLineDebounce)
        {
            /* Reset orientation, clear position and PID controller errors. */
            Odometry::getInstance().setOrientation(FP_PI() / 2);
            Odometry::getInstance().clearPosition();
            m_pidCtrl.clear();
        }
        else
        {
            isDetected = false;
        }
    }
    else
    {
        m_startEndLineDebounce = 0;
    }

    return isDetected;
}

bool DrivingState::isTrackGapDetected(int16_t position) const
{
    bool                isDetected  = false;
    const ILineSensors& lineSensors = Board::getInstance().getLineSensors();

    /* Position value after loosing the track and sensor 0 saw it as last.
     * It depends on the Zumo32U4LineSensors::readLine() implementation.
     */
    const int16_t POS_MIN = 0;

    /* Position value after loosing the track and sensor N saw it as last.
     * It depends on the Zumo32U4LineSensors::readLine() implementation.
     */
    const int16_t POS_MAX = (lineSensors.getNumLineSensors() - 1) * 1000;

    /* Note, no debouncing is done here. If necessary, it shall be done
     * outside this method.
     */
    if ((POS_MIN >= position) || (POS_MAX <= position))
    {
        isDetected = true;
    }

    return isDetected;
}

void DrivingState::adaptDriving(int16_t position)
{
    DifferentialDrive&  diffDrive       = DifferentialDrive::getInstance();
    const ILineSensors& lineSensors     = Board::getInstance().getLineSensors();
    const int16_t       MAX_MOTOR_SPEED = diffDrive.getMaxMotorSpeed();
    int16_t             speedDifference = 0; /* [steps/s] */
    int16_t             leftSpeed       = 0; /* [steps/s] */
    int16_t             rightSpeed      = 0; /* [steps/s] */

    /* Our "error" is how far we are away from the center of the
     * line, which corresponds to position (max. line sensor value multiplied
     * with sensor index).
     *
     * Get motor speed difference using PID terms.
     */
    speedDifference = m_pidCtrl.calculate(lineSensors.getSensorValueMax() * 2, position);

    /** If the traffic light is transitioning from green to red, then slow down. */
    if (true == IS_YELLOW_GR)
    {
        if (true == (m_topSpeed > 100))
        {
            leftSpeed  = (m_topSpeed - speedDifference);
            rightSpeed = (m_topSpeed + speedDifference);

            m_topSpeed -= 20;
        }
    }
    /** If the traffic light is transitioning from green to red, then speed up. */
    else if (true == IS_YELLOW_RG)
    {
        leftSpeed  = (m_topSpeed - speedDifference);
        rightSpeed = (m_topSpeed + speedDifference);

        m_topSpeed += 20;
    }
    /** Green light? Keep moving with original calibrated speed. */
    else
    {
        m_topSpeed = KEEP_TOP_SPEED;
        /* Get individual motor speeds.  The sign of speedDifference
         * determines if the robot turns left or right.
         */
        leftSpeed  = m_topSpeed - speedDifference;
        rightSpeed = m_topSpeed + speedDifference;
    }

    /* Constrain our motor speeds to be between 0 and maxSpeed.
     * One motor will always be turning at maxSpeed, and the other
     * will be at maxSpeed-|speedDifference| if that is positive,
     * else it will be stationary. For some applications, you
     * might want to allow the motor speed to go negative so that
     * it can spin in reverse.
     */
    leftSpeed  = constrain(leftSpeed, 0, MAX_MOTOR_SPEED);
    rightSpeed = constrain(rightSpeed, 0, MAX_MOTOR_SPEED);

    diffDrive.setLinearSpeed(leftSpeed, rightSpeed);
}

/** Slowdown toggle function. */
void DrivingState::enableSlowdown(bool enableIt)
{
    if (false == enableIt)
    {
        IS_YELLOW_GR = false;
    }
    else
    {
        IS_YELLOW_GR = true;
    }
}

/** Speedup toggle function. */
void DrivingState::enableSpeedup(bool enableIt)
{
    if (false == enableIt)
    {
        IS_YELLOW_RG = false;
    }
    else
    {
        IS_YELLOW_RG = true;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/