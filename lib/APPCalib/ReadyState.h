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

protected:
private:
    /**
     * Display pages which are showing different kind of information.
     */
    enum Page
    {
        PAGE_DRIVE_FORWARD = 0, /**< Show button to use to drive forward. */
        PAGE_TURN_LEFT,         /**< Show button to use to turn left 90°. */
        PAGE_TURN_RIGHT         /**< Show button to use to turn right 90°. */
    };

    /** Release timer duration in ms. */
    static const uint32_t RELEASE_DURATION = 2000;

    /**
     * Period in ms how long a page is shown on the display,
     * before the next page.
     */
    static const uint32_t NEXT_PAGE_PERIOD = 1000;

    SimpleTimer m_pageTimer;    /**< Timer for display handling. */
    Page        m_page;         /**< Current shown display page. */
    SimpleTimer m_releaseTimer; /**< Release timer */

    /**
     * Default constructor.
     */
    ReadyState() : m_pageTimer(), m_page(PAGE_DRIVE_FORWARD), m_releaseTimer()
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

    /**
     * Show current page on the display.
     */
    void showCurrentPage();

    /**
     * Show button to drive forward on the display.
     */
    void showDriveForwardPage();

    /**
     * Show button to turn left 90° on the display.
     */
    void showTurnLeftPage();

    /**
     * Show button to turn right 90° on the display.
     */
    void showTurnRightPage();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* READY_STATE_H */
/** @} */
