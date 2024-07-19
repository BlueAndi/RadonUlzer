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
     * This type defines different kind of information, which will be shown
     * to the user in the same order as defined.
     */
    enum UserInfo
    {
        USER_INFO_DRIVE_FORWARD = 0, /**< Show button to use to drive forward. */
        USER_INFO_TURN_LEFT,         /**< Show button to use to turn left 90°. */
        USER_INFO_TURN_RIGHT,        /**< Show button to use to turn right 90°. */
        USER_INFO_COUNT              /**< Number of user infos. */
    };

    /** Release timer duration in ms. */
    static const uint32_t RELEASE_DURATION = 2000;

    /**
     * Duration in ms how long a info on the display shall be shown, until
     * the next info appears.
     */
    static const uint32_t INFO_DURATION = 2000;

    SimpleTimer m_timer;         /**< Used to show information for a certain time before changing to the next info. */
    UserInfo    m_userInfoState; /**< Current user info state. */
    SimpleTimer m_releaseTimer;  /**< Release timer */
    bool        m_isButtonAPressed; /**< Is the button A pressed (last time)? */
    bool        m_isButtonBPressed; /**< Is the button B pressed (last time)? */
    bool        m_isButtonCPressed; /**< Is the button C pressed (last time)? */

    /**
     * Default constructor.
     */
    ReadyState() :
        m_timer(),
        m_userInfoState(USER_INFO_DRIVE_FORWARD),
        m_releaseTimer(),
        m_isButtonAPressed(false),
        m_isButtonBPressed(false),
        m_isButtonCPressed(false)
    {
    }

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
     * @returns Reference to ReadyState instance.
     */
    ReadyState& operator=(const ReadyState& state);

    /**
     * Show next user info.
     *
     * @param[in] next  Next user info which to show.
     */
    void showUserInfo(UserInfo next);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* READY_STATE_H */
/** @} */
