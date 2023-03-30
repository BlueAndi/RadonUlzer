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
 *  @brief  Stream Module for uint testing
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

#ifndef TEST_STREAM_H_
#define TEST_STREAM_H_

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <queue>
#include <Stream.h>
#include <YAPCommon.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Test Stream Class for Unit Testing
 */
class TestStream_ : public Stream
{
public:
    /**
     * Stream Constructor.
     */
    TestStream_()
    {
    }

    /**
     * Stream Destructor.
     */
    ~TestStream_()
    {
    }

    /**
     * Write bytes to stream.
     * @param[in] buffer Byte Array to send.
     * @param[in] length Length of Buffer.
     */
    void write(const uint8_t* buffer, size_t length) final
    {
        for (size_t idx = 0; idx < length; idx++)
        {
            m_outputBuffer[idx] = buffer[idx];
        }
    }

    /**
     * Check if there are available bytes in the Stream.
     * @returns Number of available bytes.
     */
    int available() final
    {
        return m_rcvQueue.size();
    }

    /**
     * Read bytes into a buffer.
     * @param[in] buffer Array to write bytes to.
     * @param[in] length number of bytes to be read.
     * @returns Number of bytes read from Stream.
     */
    size_t readBytes(uint8_t* buffer, size_t length) final
    {
        size_t count = 0;

        while ((!m_rcvQueue.empty()) && (count <= length))
        {
            buffer[count] = m_rcvQueue.front();
            m_rcvQueue.pop();
            count++;
        }

        return count;
    }

    /**
     * Push a data Array to the receive Queue of the Stream.
     * Used to make data available for the "read" functions.
     */
    void pushToQueue(const uint8_t* payload, uint8_t payloadSize)
    {
        for (uint8_t idx = 0; idx < payloadSize; idx++)
        {
            m_rcvQueue.push(payload[idx]);
        }
    }

    /**
     * Flush output buffer, setting all values to 0x00
     */
    void flushOutputBuffer()
    {
        memset(m_outputBuffer, 0x00, MAX_FRAME_LEN);
    }

    /**
     * Flush input queue (buffer)
     */
    void flushInputBuffer()
    {
        while (!m_rcvQueue.empty())
        {
            m_rcvQueue.pop();
        }
    }

    /**
     * Buffer to write any output of the Stream
     */
    uint8_t m_outputBuffer[MAX_FRAME_LEN] = {0U};

    /**
     * Byte Queue working as an RX Buffer.
     */
    std::queue<uint8_t> m_rcvQueue;
};

#endif /* TEST_STREAM_H_ */