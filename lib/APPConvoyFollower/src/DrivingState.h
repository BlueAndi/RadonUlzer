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
     * Set target motor speeds.
     *
     * @param[in] leftMotor  Left motor speed. [steps/s]
     * @param[in] rightMotor Right motor speed. [steps/s]
     */
    void setTargetSpeeds(int16_t leftMotor, int16_t rightMotor);

protected:
private:
    /** Flag: State is active. */
    bool m_isActive;

    /**
     * Default constructor.
     */
    DrivingState() : IState(), m_isActive(false)
    {
    }

    /**
     * Default destructor.
     */
    ~DrivingState()
    {
    }

    /**
     * Copy construction of an instance.
     * Not allowed.
     *
     * @param[in] state Source instance.
     */
    DrivingState(const DrivingState& state);

    /**
     * Assignment of an instance.
     * Not allowed.
     *
     * @param[in] state Source instance.
     *
     * @returns Reference to DrivingState instance.
     */
    DrivingState& operator=(const DrivingState& state);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DRIVING_STATE_H */
/** @} */
