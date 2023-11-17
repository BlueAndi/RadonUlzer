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
 * @brief  Motor speed calibration state
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef MOTOR_SPEED_CALIBRATION_STATE_H
#define MOTOR_SPEED_CALIBRATION_STATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IState.h>
#include <SimpleTimer.h>
#include <Board.h>
#include <RelativeEncoders.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The motor speed calibration state. */
class MotorSpeedCalibrationState : public IState
{
public:
    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static MotorSpeedCalibrationState& getInstance()
    {
        static MotorSpeedCalibrationState instance;

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
    /** Calibration phases */
    enum Phase
    {
        PHASE_1_BACK,    /**< Drive with max. speed backwards. */
        PHASE_2_FORWARD, /**< Drive with max. speed forwards. */
        PHASE_3_FINISHED /**< Calibration is finished. */
    };

    /**
     * Duration in ms about to wait, until the calibration drive starts.
     */
    static const uint32_t WAIT_TIME = 1000;

    /**
     * Calibration drive duration in ms.
     * It means how long the robot is driven with max. speed forward/backward.
     */
    static const uint32_t CALIB_DURATION = 1000;

    SimpleTimer      m_timer; /**< Timer used to wait, until the calibration drive starts and for drive duration. */
    Phase            m_phase; /**< Current calibration phase */
    int16_t          m_maxSpeedLeft;  /**< Max. determined left motor speed [steps/s]. */
    int16_t          m_maxSpeedRight; /**< Max. determined right motor speed [steps/s]. */
    RelativeEncoders m_relEncoders;   /**< Relative encoders left/right. */

    /**
     * Default constructor.
     */
    MotorSpeedCalibrationState() :
        m_timer(),
        m_phase(PHASE_1_BACK),
        m_maxSpeedLeft(0),
        m_maxSpeedRight(0),
        m_relEncoders(Board::getInstance().getEncoders())
    {
    }

    /**
     * Default destructor.
     */
    ~MotorSpeedCalibrationState()
    {
    }

    /* Not allowed. */
    MotorSpeedCalibrationState(const MotorSpeedCalibrationState& state); /**< Copy construction of an instance. */
    MotorSpeedCalibrationState& operator=(const MotorSpeedCalibrationState& state); /**< Assignment of an instance. */

    /**
     * Determine the max. motor speed, considering both driving directions.
     * There are two steps necessary:
     * - Drive full backward and call this method to determine.
     * - Drive full forward and call this method to determine.
     */
    void determineMaxMotorSpeed();

    /**
     * Finish the calibration and determine next state.
     *
     * @param[in] sm    State machine
     */
    void finishCalibration(StateMachine& sm);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MOTOR_SPEED_CALIBRATION_STATE_H */
/** @} */
