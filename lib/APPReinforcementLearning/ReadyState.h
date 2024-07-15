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
 * @author Andreas Merkle <web@blue-andi.de>
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
     *  Returns the mode selection
     *
     * @return It returns 1 if DrivingMode is selected or 2 if TrainingMode is selected or 0 if nothing is selected and there is no timeout..
     */

    uint8_t selectedMode( );
    /* Re-initialize the board. This is required for the webots simulation in
     * case the world is reset by a supervisor without restarting the RadonUlzer
     * controller executable.
    */
    void reinit_Board();

protected:
private:
    /**
    * the mode that can be selected.
    */
    /**< Timer used for cyclic debug output. */
    SimpleTimer m_timer; 

    /** Timeout timer for the selected mode. */
    SimpleTimer m_ModeTimeoutTimer; 

    /** The system state machine. */
    StateMachine m_systemStateMachine;  

    /**< Is set (true), if a lap time is available. */
    bool        m_isLapTimeAvailable; 

    /**< Lap time in ms of the last successful driven round. */
    uint32_t    m_lapTime;       
    
    enum Mode_: uint8_t
    {
        DRIVING_MODE = 0, /**< Driving Mode Selected. */
        TRAINING_MODE     /**< Training Mode Selected. */
    };
    
    Mode_ myMode;
    

    ReadyState() : 
               m_timer(), 
               m_isLapTimeAvailable(false), 
               m_lapTime(0),
               myMode(TRAINING_MODE)   
    {
    }

    /**
     * Default destructor.
     */
    ~ReadyState()
    {
    }

    /* Not allowed. */
    ReadyState(const ReadyState& state);            /**< Copy construction of an instance. */
    ReadyState& operator=(const ReadyState& state); /**< Assignment of an instance. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* READY_STATE_H */
/** @} */
