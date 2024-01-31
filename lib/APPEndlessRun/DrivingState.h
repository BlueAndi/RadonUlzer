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

    /**
     * Toggle slowdown of the motor.
     *
     * @param[in] enableIt enables/disables the slowing down of the robot.
     */
    void enableSlowdown(bool enableIt);

    /**
     * Toggle speedup of the motor.
     *
     * @param[in] enableIt enables/disables the speed up of the robot.
     */
    void enableSpeedup(bool enableIt);

    /**
     * Set driving speed of the robot during line following.
     *
     * @param[in] topSpeed the speed the robot shall drive with.
     */
    void setTopSpeed(int16_t topSpeed)
    {
        m_topSpeed = topSpeed;
    }

    /**
     * Get current speed of the robot during driving.
     *
     * @return speed in steps/s
     */
    uint16_t getTopSpeed()
    {
        return m_topSpeed;
    }

protected:
private:
    /* Flags used for slowdown and speedup methods. */
    bool IS_YELLOW_RG = false;
    bool IS_YELLOW_GR = false;

    /**
     * The line detection status.
     */
    enum LineStatus
    {
        LINE_STATUS_FIND_START_LINE = 0, /**< Find the start line. */
        LINE_STATUS_START_LINE_DETECTED, /**< Start line detected. */
        LINE_STATUS_FIND_END_LINE        /**< Find the end line. */
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

    SimpleTimer            m_observationTimer; /**< Observation timer to observe the max. time per challenge. */
    SimpleTimer            m_callbackTimer;
    SimpleTimer            m_lapTime;        /**< Timer used to calculate the lap time. */
    SimpleTimer            m_pidProcessTime; /**< Timer used for periodically PID processing. */
    PIDController<int16_t> m_pidCtrl;        /**< PID controller, used for driving. */
    int16_t                m_topSpeed;    /**< Top speed in [steps/s]. It might be lower or equal to the max. speed! */
    LineStatus             m_lineStatus;  /**< Status of start-/end line detection */
    TrackStatus            m_trackStatus; /**< Status of track which means on track or track lost, etc. */
    uint8_t m_startEndLineDebounce;       /**< Counter used for easys debouncing of the start-/end line detection. */

    int16_t KEEP_TOP_SPEED = m_topSpeed;

    /**
     * Default constructor.
     */
    DrivingState() :
        m_observationTimer(),
        m_callbackTimer(),
        m_lapTime(),
        m_pidProcessTime(),
        m_pidCtrl(),
        m_topSpeed(0),
        m_lineStatus(LINE_STATUS_FIND_START_LINE),
        m_trackStatus(TRACK_STATUS_ON_TRACK),
        m_startEndLineDebounce(0)
    {
    }

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
     * Control driving in case the robot is on track.
     *
     * @param[in] position           Current position on track
     * @param[in] lineSensorValues   Value of each line sensor
     */
    void processOnTrack(int16_t position, const uint16_t* lineSensorValues);

    /**
     * Control driving in case the robot lost the track.
     * It handles the track search algorithm.
     *
     * @param[in] position           Current position on track
     * @param[in] lineSensorValues   Value of each line sensor
     */
    void processTrackLost(int16_t position, const uint16_t* lineSensorValues);

    /**
     * Is start-/endline detected?
     *
     * @param[in] lineSensorValues  Line sensor values
     *
     * @return If a start-/endline is detected, it will return true otherwise false.
     */
    bool isStartEndLineDetected(const uint16_t* lineSensorValues);

    /**
     * Is a track gap detected?
     * Note, it contains no debouncing inside. If necessary, it shall be
     * done outside.
     *
     * @param[in] position Determined position in digits
     *
     * @return If a track gap is detected, it will return true otherwise false.
     */
    bool isTrackGapDetected(int16_t position) const;

    /**
     * Adapt driving by using a PID algorithm, depended on the position
     * input.
     *
     * @param[in] position  Position in digits
     */
    void adaptDriving(int16_t position);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DRIVING_STATE_H */
/** @} */
