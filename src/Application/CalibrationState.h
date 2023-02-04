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
 * 
 * @addtogroup Application
 *
 * @{
 */

#ifndef CALIBRATION_STATE_H
#define CALIBRATION_STATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IState.h"
#include <SimpleTimer.h>
#include <RelativeEncoders.h>
#include "Board.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The system calibration state. */
class CalibrationState : public IState
{
public:
    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static CalibrationState& getInstance()
    {
        static CalibrationState instance;

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
     * Duration in ms about to wait, until the calibration drive starts.
     */
    static const uint32_t WAIT_TIME = 1000;

    SimpleTimer m_timer; /**< Timer used to wait, until the calibration drive starts. */
    uint16_t    m_steps; /**< Calibration steps, used to determine the rotation direction during calibration drive. */
    RelativeEncoders m_encoder; /**< Encoders used to turn the robot over the track with the sensors. */

    /**
     * Default constructor.
     */
    CalibrationState() : m_timer(), m_steps(0), m_encoder(Board::getInstance().getEncoders())
    {
    }

    /**
     * Default destructor.
     */
    ~CalibrationState()
    {
    }

    CalibrationState(const CalibrationState& state);
    CalibrationState& operator=(const CalibrationState& state);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CALIBRATION_STATE_H */