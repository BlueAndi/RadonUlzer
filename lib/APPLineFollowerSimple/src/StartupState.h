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
 * @brief  Startup state
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef STARTUP_STATE_H
#define STARTUP_STATE_H

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

/** The system startup state. */
class StartupState : public IState
{
public:
    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static StartupState& getInstance()
    {
        static StartupState instance;

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
        USER_INFO_TEAM_NAME = 0,   /**< Show the team name. */
        USER_INFO_UI,              /**< Show the user interface. */
        USER_INFO_COUNT            /**< Number of user infos. */
    };

    /**
     * Duration in ms how long a info on the display shall be shown, until
     * the next info appears.
     */
    static const uint32_t INFO_DURATION = 2000;

    bool        m_isMaxMotorSpeedCalibAvailable; /**< Is max. motor speed calibration value available? */
    SimpleTimer m_timer;         /**< Used to show information for a certain time before changing to the next info. */
    UserInfo    m_userInfoState; /**< Current user info state. */

    /**
     * Default constructor.
     */
    StartupState() : m_isMaxMotorSpeedCalibAvailable(false), m_timer(), m_userInfoState(USER_INFO_TEAM_NAME)
    {
    }

    /**
     * Default destructor.
     */
    ~StartupState()
    {
    }

    /**
     * Copy construction of an instance.
     * Not allowed.
     *
     * @param[in] state Source instance.
     */
    StartupState(const StartupState& state);

    /**
     * Assignment of an instance.
     * Not allowed.
     *
     * @param[in] state Source instance.
     *
     * @returns Reference to StartupState instance.
     */
    StartupState& operator=(const StartupState& state);

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

#endif /* STARTUP_STATE_H */
/** @} */
