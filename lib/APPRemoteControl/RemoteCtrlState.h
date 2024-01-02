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
 * @brief  Remote control state
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef REMOTE_CTRL_H
#define REMOTE_CTRL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IState.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The system remote control state. */
class RemoteCtrlState : public IState
{
public:
    /** Remote control commands. */
    typedef enum : uint8_t
    {
        CMD_ID_IDLE = 0,                /**< Nothing to do. */
        CMD_ID_START_LINE_SENSOR_CALIB, /**< Start line sensor calibration. */
        CMD_ID_START_MOTOR_SPEED_CALIB, /**< Start motor speed calibration. */
        CMD_ID_REINIT_BOARD             /**< Re-initialize the board. Required for webots simulation. */

    } CmdId;

    /** Remote control command responses. */
    typedef enum : uint8_t
    {
        RSP_ID_OK = 0,  /**< Command successful executed. */
        RSP_ID_PENDING, /**< Command is pending. */
        RSP_ID_ERROR    /**< Command failed. */

    } RspId;

    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static RemoteCtrlState& getInstance()
    {
        static RemoteCtrlState instance;

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
     * Execute command.
     * If a command is pending, it won't be executed.
     *
     * @param[in] cmdId The id of the command which to execute.
     */
    void execute(CmdId cmdId)
    {
        if (CMD_ID_IDLE == m_cmdId)
        {
            m_cmdId = cmdId;
            m_rspId = RSP_ID_PENDING;
        }
    }

    /**
     * Get command response of current command.
     *
     * @return Command response
     */
    RspId getCmdRsp() const
    {
        return m_rspId;
    }

protected:
private:
    CmdId m_cmdId; /**< Current pending command. */
    RspId m_rspId; /**< Current command response. */

    /**
     * Default constructor.
     */
    RemoteCtrlState() : m_cmdId(CMD_ID_IDLE), m_rspId(RSP_ID_OK)
    {
    }

    /**
     * Default destructor.
     */
    ~RemoteCtrlState()
    {
    }

    RemoteCtrlState(const RemoteCtrlState& state);            /**< Copy construction of an instance. */
    RemoteCtrlState& operator=(const RemoteCtrlState& state); /**< Assignment of an instance. */

    /**
     * Set command response and finish the command execution.
     *
     * @param[in] rsp   Command response
     */
    void finishCommand(RspId rsp)
    {
        m_rspId = rsp;
        m_cmdId = CMD_ID_IDLE;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* REMOTE_CTRL_H */
/** @} */
