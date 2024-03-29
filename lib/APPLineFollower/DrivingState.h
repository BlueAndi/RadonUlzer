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
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef DRIVING_STATE_H
#define DRIVING_STATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IState.h>
#include <SimpleTimer.h>
#include <PIDController.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The system driving state. */
class DrivingState : public IState
{
public:
    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static DrivingState& getInstance()
    {
        static DrivingState instance;

        /* Singleton idiom to force initialization during first usage. */

        return instance;
    }

    /**
     * If the state is entered, this method will called once.
     */
    void entry() final;

    /**
     * Processing the state.
     *
     * @param[in] sm State machine, which is calling this state.
     */
    void process(StateMachine& sm) final;

    /**
     * If the state is left, this method will be called once.
     */
    void exit() final;

protected:
private:
    /**
     * The line detection status.
     */
    enum LineStatus
    {
        LINE_STATUS_NO_START_STOP_LINE_DETECTED = 0, /**< No start/stop line detected. */
        LINE_STATUS_START_LINE_DETECTED,             /**< Start line detected. */
        LINE_STATUS_STOP_LINE_DETECTED               /**< Stop line detected. */
    };

    /**
     * The track status.
     */
    enum TrackStatus
    {
        TRACK_STATUS_ON_TRACK = 0, /**< Robot is on track */
        TRACK_STATUS_LOST,         /**< Robot lost track */
        TRACK_STATUS_FINISHED      /**< Robot found the end line or a error happened */
    };

    /** Observation duration in ms. This is the max. time within the robot must be finished its drive. */
    static const uint32_t OBSERVATION_DURATION = 3000000;

    /** Max. distance in mm after a lost track must be found again. */
    static const uint32_t MAX_DISTANCE = 200;

    /** Period in ms for PID processing. */
    static const uint32_t PID_PROCESS_PERIOD = 10;

    /**
     * The line sensor threshold (normalized) used to detect the track.
     * The track is detected in case the received value is greater or equal than
     * the threshold.
     */
    static const uint16_t LINE_SENSOR_ON_TRACK_VALUE = 200U;

    /**
     * Position set point which is the perfect on track position.
     * This is the goal to achieve.
     */
    static const int16_t POSITION_SET_POINT;

    /**
     * ID of most left sensor.
     */
    static const uint8_t SENSOR_ID_MOST_LEFT;

    /**
     * ID of most right sensor.
     */
    static const uint8_t SENSOR_ID_MIDDLE;

    /**
     * ID of middle sensor.
     */
    static const uint8_t SENSOR_ID_MOST_RIGHT;

    SimpleTimer            m_observationTimer; /**< Observation timer to observe the max. time per challenge. */
    SimpleTimer            m_lapTime;          /**< Timer used to calculate the lap time. */
    SimpleTimer            m_pidProcessTime;   /**< Timer used for periodically PID processing. */
    PIDController<int16_t> m_pidCtrl;          /**< PID controller, used for driving. */
    int16_t                m_topSpeed;    /**< Top speed in [steps/s]. It might be lower or equal to the max. speed! */
    LineStatus             m_lineStatus;  /**< Status of start-/end line detection */
    TrackStatus            m_trackStatus; /**< Status of track which means on track or track lost, etc. */
    bool                   m_isStartStopLineDetected; /**< Is the start/stop line detected? */
    uint8_t                m_lastSensorIdSawTrack;    /**< The sensor id of the sensor which saw the track as last. */
    int16_t                m_lastPosition; /**< Last position, used to decide strategy in case of a track gap. */

    /**
     * Default constructor.
     */
    DrivingState();

    /**
     * Default destructor.
     */
    ~DrivingState()
    {
    }

    /* Not allowed. */
    DrivingState(const DrivingState& state);            /**< Copy construction of an instance. */
    DrivingState& operator=(const DrivingState& state); /**< Assignment of an instance. */

    /**
     * Is the start/stop line detected?
     *
     * @param[in] lineSensorValues  The line sensor values as array.
     * @param[in] length            The number of line sensor values.
     *
     * @return If start/stop line detected, it will return true otherwise false.
     */
    bool isStartStopLineDetected(const uint16_t* lineSensorValues, uint8_t length);

    /**
     * Is right angle curve to the left detected?
     *
     * @param[in] lineSensorValues  The line sensor values as array.
     * @param[in] length            The number of line sensor values.
     *
     * @return If right angle curve to the left is detected, it will return true otherwise false.
     */
    bool isRightAngleCurveToLeft(const uint16_t* lineSensorValues, uint8_t length);

    /**
     * Is right angle curve to the right detected?
     *
     * @param[in] lineSensorValues  The line sensor values as array.
     * @param[in] length            The number of line sensor values.
     *
     * @return If right angle curve to the right is detected, it will return true otherwise false.
     */
    bool isRightAngleCurveToRight(const uint16_t* lineSensorValues, uint8_t length);

    /**
     * Is track gap detected?
     *
     * @param[in] lineSensorValues  The line sensor values as array.
     * @param[in] length            The number of line sensor values.
     *
     * @return If track gap is detected, it will return true otherwise false.
     */
    bool isGapDetected(const uint16_t* lineSensorValues, uint8_t length);

    /**
     * Adapt driving by using a PID algorithm, depended on the position
     * input.
     *
     * @param[in] position                  Position in digits
     * @param[in] allowNegativeMotorSpeed   Allow negative motor speed.
     */
    void adaptDriving(int16_t position, bool allowNegativeMotorSpeed);

    /**
     * Check the abort conditions while driving the challenge.
     *
     * @return If abort is required, it will return true otherwise false.
     */
    bool isAbortRequired();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DRIVING_STATE_H */
/** @} */
