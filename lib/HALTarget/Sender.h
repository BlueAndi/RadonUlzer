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
 * @brief  Sender realization
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HALTarget
 *
 * @{
 */

#ifndef SENDER_H
#define SENDER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ISender.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This class simulates a sender, because its only relevant for the simulation. */
class Sender : public ISender
{
public:
    /**
     * Constructs the sender adapter.
     */
    Sender() : ISender()
    {
    }

    /**
     * Destroys the sender adapter.
     */
    ~Sender()
    {
    }

    /**
     * Set channel which to send data to.
     *
     * @param[in] channel   The channel which to use.
     */
    void setChannel(int32_t channel) final
    {
        (void)channel;
    }

    /**
     * Sends data to the configured channel.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in bytes.
     */
    void send(const void* data, size_t size) const final
    {
        (void)data;
        (void)size;
    }

    /**
     * Sends string to the configured channel.
     *
     * @param[in] str   String which to send.
     */
    void send(const char* str) const final
    {
        (void)str;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SENDER_H */
/** @} */
