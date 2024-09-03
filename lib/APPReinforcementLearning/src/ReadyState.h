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
 * @brief  Ready state
 * @author Akram Bziouech
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef READY_STATE_H
#define READY_STATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IState.h>
#include <SimpleTimer.h>
#include <StateMachine.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The system ready state. */
class ReadyState : public IState
{
public:
    /**
     * The mode that can be selected.
     */
    enum Mode: uint8_t
    {
        IDLE = 0,     /**< No mode has been selected*/
        DRIVING_MODE, /**< Driving mode Selected. */
        TRAINING_MODE /**< Training mode Selected. */
    };

    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static ReadyState& getInstance()
    {
        static ReadyState instance;

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
     * Set lap time, which to show on the display.
     *
     * @param[in] lapTime   Lap time in ms
     */
    void setLapTime(uint32_t lapTime);

    /**
     *  Set the selected mode.
     *
     * @return It returns 1 if DrivingMode is selected or 2 if TrainingMode is selected, otherwise 0.
     */
    Mode getSelectedMode();

protected:
private:

    /** Duration of the selected mode in ms. This is the maximum time to select a mode. */
    static const uint32_t MODE_SELECTED_PERIOD = 2000U;

    /** Duration to handle State changes in ms. */
    static const uint32_t STATE_TRANSITION_PERIOD = 2100U;

    /**
     * The line sensor threshold (normalized) used to detect the track.
     * The track is detected in case the received value is greater or equal than
     * the threshold.
     */
    static const uint16_t LINE_SENSOR_ON_TRACK_MIN_VALUE = 200U;

    /**
     * ID of most left sensor.
     */
    static const uint8_t SENSOR_ID_MOST_LEFT;

    /**
     * ID of middle sensor.
     */
    static const uint8_t SENSOR_ID_MIDDLE;

    /**
     * ID of most right sensor.
     */
    static const uint8_t SENSOR_ID_MOST_RIGHT;

    /**
     * The max. normalized value of a sensor in digits.
     */
    static const uint16_t SENSOR_VALUE_MAX;

    bool        m_isLastStartStopLineDetected; /**< Is the Last start/stop line detected? */
    SimpleTimer m_modeTimeoutTimer;            /**< Timeout timer for the selected mode. */
    Mode        m_mode;                        /**< Mode that can select Driving Mode or Training Mode. */
    bool        m_isLapTimeAvailable;          /**< Is set (true), if a lap time is available. */
    uint32_t    m_lapTime;                     /**< Lap time in ms of the last successful driven round. */
    bool        m_isButtonAPressed;            /**< Is the button A pressed (last time)? */
    bool        m_isButtonBPressed;            /**< Is the button B pressed (last time)? */
    SimpleTimer m_stateTransitionTimer;        /**< Timer to handle State changes */
    /**
     * Default constructor.
     */
    ReadyState();

    /**
     * Default destructor.
     */
    ~ReadyState()
    {
    }

    /**
     * Copy construction of an instance.
     * Not allowed.
     *
     * @param[in] state Source instance.
     */
    ReadyState(const ReadyState& state);

    /**
     * Assignment of an instance.
     * Not allowed.
     *
     * @param[in] state Source instance.
     *
     * @returns Reference to ErrorState instance.
     */
    ReadyState& operator=(const ReadyState& state);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* READY_STATE_H */
/** @} */
