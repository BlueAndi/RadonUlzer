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

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "SocketServer.h"
#include <stdio.h>
#include <string>
#include <cstring>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

SocketServer::SocketServer() : m_clientSocket(INVALID_SOCKET), m_listenSocket(INVALID_SOCKET)
{
}

SocketServer::~SocketServer()
{
}

bool SocketServer::init(uint16_t port, uint8_t maxConnections)
{
    bool             success = true;
    int              result;
    struct addrinfo  hints;
    struct addrinfo* addrInfo = NULL;

#ifdef _WIN32
    ZeroMemory(&hints, sizeof(hints));
#else
    memset(&hints, 0, sizeof(struct addrinfo));
#endif

    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

#ifdef _WIN32
    WSADATA wsaData;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return false;
    }

#endif

    // Resolve the server address and port
    result = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &addrInfo);
    if (result != 0)
    {
        printf("getaddrinfo failed with error: %d\n", result);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    // Create a SOCKET for the server to listen for client connections.
    m_listenSocket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET)
    {
        printf("socket failed\n");
        freeaddrinfo(addrInfo);
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    // Setup the TCP listening socket
    result = bind(m_listenSocket, addrInfo->ai_addr, static_cast<int>(addrInfo->ai_addrlen));
    if (result == SOCKET_ERROR)
    {
        printf("bind failed\n");
        freeaddrinfo(addrInfo);
#ifdef _WIN32
        closesocket(m_listenSocket);
        WSACleanup();
#else
        close(m_listenSocket);
#endif
        return false;
    }

    freeaddrinfo(addrInfo);

    result = listen(m_listenSocket, maxConnections);
    if (result == SOCKET_ERROR)
    {
        printf("listen failed\n");
#ifdef _WIN32
        closesocket(m_listenSocket);
        WSACleanup();
#else
        close(m_listenSocket);
#endif
        return false;
    }

    return true;
}

void SocketServer::sendMessage(const uint8_t* buf, uint16_t length)
{
    // Echo the buffer back to the sender
    if (m_clientSocket != INVALID_SOCKET)
    {
        int iSendResult = send(m_clientSocket, reinterpret_cast<const char*>(buf), length, 0);
        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed\n");
#ifdef _WIN32
            closesocket(m_listenSocket);
#else
            close(m_listenSocket);
#endif
        }
    }
}

uint32_t SocketServer::available()
{
    this->processRx();
    return m_rcvQueue.size();
}

bool SocketServer::getByte(uint8_t& byte)
{
    if (!m_rcvQueue.empty())
    {
        byte = m_rcvQueue.front();
        m_rcvQueue.pop();
        return true;
    }
    return false;
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SocketServer::processRx()
{
    if (m_listenSocket != INVALID_SOCKET)
    {
        fd_set         readFDS, writeFDS, exceptFDS;
        int            ret;
        struct timeval timeout;

        timeout.tv_sec  = 0;
        timeout.tv_usec = 10;

        FD_ZERO(&readFDS);
        FD_ZERO(&writeFDS);
        FD_ZERO(&exceptFDS);

        FD_SET(m_listenSocket, &readFDS);
        FD_SET(m_listenSocket, &exceptFDS);

        // If there is a client connected
        if (m_clientSocket != INVALID_SOCKET)
        {
            FD_SET(m_clientSocket, &readFDS);
            FD_SET(m_clientSocket, &exceptFDS);
        }

        ret = select(m_listenSocket + 1, &readFDS, &writeFDS, &exceptFDS, &timeout);

        if (0 < ret)
        {
            // New Client Connection available
            if (FD_ISSET(m_listenSocket, &readFDS))
            {
                // Accept a client socket
                m_clientSocket = accept(m_listenSocket, NULL, NULL);
                if (m_clientSocket == INVALID_SOCKET)
                {
                    printf("accept failed\n");
                }
            }

            // Client Ready to read
            if (FD_ISSET(m_clientSocket, &readFDS))
            {
                const size_t bufferLength = 300U;
                char     recvbuf[bufferLength];
                int      result = recv(m_clientSocket, recvbuf, bufferLength, 0);

                if (result > 0)
                {
                    for (uint8_t idx = 0; idx < result; idx++)
                    {
                        m_rcvQueue.push(static_cast<uint8_t>(recvbuf[idx]));
                    }
                }
                else
                {
#ifdef _WIN32
                    closesocket(m_listenSocket);
#else
                    close(m_listenSocket);
#endif
                    m_clientSocket = INVALID_SOCKET;
                }
            }
        }
    }
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
