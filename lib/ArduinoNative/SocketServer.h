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
 *  @brief  Socket Server for Inter-Process Communication
 *  @author Gabryel Reyes <gabryelrdiaz@gmail.com>
 */

#ifndef SOCKET_SERVER_H_
#define SOCKET_SERVER_H_

/******************************************************************************
 * Includes
 *****************************************************************************/

#include <stdint.h>

#ifdef _WIN32
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <queue>
#pragma comment(lib, "Ws2_32.lib")
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Socket Server for Inter-Process Communication.
 */
class SocketServer
{
public:
    /**
     * Construct a SocketServer.
     */
    SocketServer();

    /**
     * Destruct the SocketServer.
     */
    ~SocketServer();

    /**
     * Initialize the SocketServer.
     * @param[in] port Port number to set the Socket to.
     * @returns true if server has been succesfully set-up.
     */
    bool init(uint16_t port);

    /**
     * Send a message to the socket.
     * @param[in] buf Byte buffer to send
     * @param[in] length Number of bytes to send
     */
    void sendMessage(const uint8_t* buf, uint16_t length);

    /**
     * Check if any data has been received.
     * @returns number of available bytes.
     */
    uint32_t available();

    /**
     * Get a Byte.
     * @returns Received byte.
     * @returns -1 if no bytes available
     */
    int8_t getByte();

private:
    /**
     * Process the receiving of messages and client connections.
     */
    void processRx();

    /**
     * File Descriptor of the Client Socket.
     */
    SOCKET m_clientSocket;

    /**
     * File Descriptor of the Listening Socket.
     */
    SOCKET m_listenSocket;

    /**
     * Queue for the received bytes.
     */
    std::queue<uint8_t> m_rcvQueue;
};

#endif /* SOCKET_SERVER_H_ */