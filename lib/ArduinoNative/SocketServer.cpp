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

SocketServer::SocketServer() : m_clientSocket(INVALID_SOCKET),
                               m_listenSocket(INVALID_SOCKET)
{
}

SocketServer::~SocketServer()
{
}

bool SocketServer::init(uint16_t port, uint8_t maxConnections)
{
    bool             success = true;
    WSADATA          wsaData;
    int              iResult;
    struct addrinfo  hints;
    struct addrinfo* result = NULL;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        success = false;
    }

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        success = false;
    }

    // Create a SOCKET for the server to listen for client connections.
    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        success = false;
    }

    // Setup the TCP listening socket
    iResult = bind(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(m_listenSocket);
        WSACleanup();
        success = false;
    }

    freeaddrinfo(result);

    iResult = listen(m_listenSocket, maxConnections);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        success = false;
    }

    return success;
}

void SocketServer::sendMessage(const uint8_t* buf, uint16_t length)
{
    // Echo the buffer back to the sender
    if (m_clientSocket != INVALID_SOCKET)
    {
        int iSendResult = send(m_clientSocket, reinterpret_cast<const char*>(buf), length, 0);
        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(m_clientSocket);
        }
    }
}

uint32_t SocketServer::available()
{
    this->processRx();
    return m_rcvQueue.size();
}

int8_t SocketServer::getByte()
{
    int8_t byte = -1;

    if (!m_rcvQueue.empty())
    {
        byte = m_rcvQueue.front();
        m_rcvQueue.pop();
    }

    return byte;
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SocketServer::processRx()
{
    if (m_listenSocket != INVALID_SOCKET)
    {
        fd_set         fr, fw, fe;
        int            nRet;
        struct timeval tv;

        tv.tv_sec  = 0;
        tv.tv_usec = 10;

        FD_ZERO(&fr);
        FD_ZERO(&fw);
        FD_ZERO(&fe);

        FD_SET(m_listenSocket, &fr);
        FD_SET(m_listenSocket, &fe);

        // If there is a client connected
        if (m_clientSocket != INVALID_SOCKET)
        {
            FD_SET(m_clientSocket, &fr);
            FD_SET(m_clientSocket, &fe);
        }

        nRet = select(m_listenSocket + 1, &fr, &fw, &fe, &tv);

        if (0 < nRet)
        {
            // New Client Connection available
            if (FD_ISSET(m_listenSocket, &fr))
            {
                // Accept a client socket
                m_clientSocket = accept(m_listenSocket, NULL, NULL);
                if (m_clientSocket == INVALID_SOCKET)
                {
                    printf("accept failed with error: %d\n", WSAGetLastError());
                }
            }

            // Client Ready to read
            if (FD_ISSET(m_clientSocket, &fr))
            {
                uint16_t bufferLength = 300U;
                char recvbuf[bufferLength];
                int  iResult = recv(m_clientSocket, recvbuf, bufferLength, 0);

                if (iResult > 0)
                {
                    for (int i = 0; i < iResult; i++)
                    {
                        m_rcvQueue.push(recvbuf[i]);
                    }
                }
                else if (iResult == 0)
                {
                    closesocket(m_clientSocket);
                    m_clientSocket = INVALID_SOCKET;
                }
                else
                {
                    printf("recv failed with error: %d\n", WSAGetLastError());
                    closesocket(m_clientSocket);
                    m_clientSocket = INVALID_SOCKET;
                }
            }
        }
    }
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
