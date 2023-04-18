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
#include <queue>

#ifdef _WIN32
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>  /* definition of inet_ntoa */
#include <netdb.h>      /* definition of gethostbyname */
#include <netinet/in.h> /* definition of struct sockaddr_in */
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h> /* definition of close */
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/

#ifndef _WIN32
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR   (-1)
#endif

/******************************************************************************
 * Types and classes
 *****************************************************************************/

#ifndef _WIN32
typedef unsigned int UINT_PTR;
typedef UINT_PTR     SOCKET;
#endif

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** SocketServer Members. PIMPL Idiom. */
struct SocketServer::SocketServerImpl
{
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

    /**
     * Construct an SocketServerImpl instance.
     */
    SocketServerImpl() : m_clientSocket(INVALID_SOCKET), m_listenSocket(INVALID_SOCKET), m_rcvQueue()
    {
    }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

SocketServer::SocketServer() : Stream(), m_members(new SocketServerImpl)
{
}

SocketServer::~SocketServer()
{
    /* Sockets are closed before deleting m_members. */
    close();

    if (nullptr != m_members)
    {
        delete m_members;
    }
}

bool SocketServer::init(uint16_t port, uint8_t maxConnections)
{
    int              result;
    struct addrinfo  hints;
    struct addrinfo* addrInfo = nullptr;

    if (nullptr == m_members)
    {
        return false;
    }

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

#ifdef _WIN32
    WSADATA wsaData;

    /* Initialize Winsock */
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (0 != result)
    {
        printf("WSAStartup failed with error: %d\n", result);
        return false;
    }

#endif

    /* Resolve the server address and port */
    result = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addrInfo);
    if (0 != result)
    {
        printf("getaddrinfo failed with error: %d\n", result);
        close();
        return false;
    }

    /* Create a SOCKET for the server to listen for client connections. */
    m_members->m_listenSocket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (INVALID_SOCKET == m_members->m_listenSocket)
    {
        printf("socket failed\n");
        freeaddrinfo(addrInfo);
        close();
        return false;
    }

    /* Setup the TCP listening socket */
    result = bind(m_members->m_listenSocket, addrInfo->ai_addr, static_cast<int>(addrInfo->ai_addrlen));
    if (SOCKET_ERROR == result)
    {
        printf("bind failed\n");
        freeaddrinfo(addrInfo);
        close();
        return false;
    }

    freeaddrinfo(addrInfo);

    result = listen(m_members->m_listenSocket, maxConnections);
    if (SOCKET_ERROR == result)
    {
        printf("listen failed\n");
        close();
        return false;
    }

    return true;
}

void SocketServer::print(const char str[])
{
    /* Not implemented*/
    (void)str;
}

void SocketServer::print(uint8_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::print(uint16_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::print(uint32_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::print(int8_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::print(int16_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::print(int32_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::println(const char str[])
{
    /* Not implemented*/
    (void)str;
}

void SocketServer::println(uint8_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::println(uint16_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::println(uint32_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::println(int8_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::println(int16_t value)
{
    /* Not implemented*/
    (void)value;
}

void SocketServer::println(int32_t value)
{
    /* Not implemented*/
    (void)value;
}

size_t SocketServer::write(const uint8_t* buffer, size_t length)
{
    size_t bytesSent = 0;

    if (nullptr != m_members)
    {
        /* Echo the buffer back to the sender */
        if (INVALID_SOCKET != m_members->m_clientSocket)
        {
            int result = send(m_members->m_clientSocket, reinterpret_cast<const char*>(buffer), length, 0);
            if (SOCKET_ERROR == result)
            {
                printf("send failed\n");
                /* Error on the socket. Client is now invalid. */
                m_members->m_clientSocket = INVALID_SOCKET;
            }
            else
            {
                bytesSent = result;
            }
        }
    }
    return bytesSent;
}

int SocketServer::available() const
{
    return (nullptr != m_members) ? m_members->m_rcvQueue.size() : 0;
}

size_t SocketServer::readBytes(uint8_t* buffer, size_t length)
{
    size_t count = 0;

    for (count = 0; count < length; count++)
    {
        uint8_t byte;
        if (false == getByte(byte))
        {
            break;
        }
        else
        {
            buffer[count] = byte;
        }
    }

    return count;
}

void SocketServer::process()
{
    if (nullptr != m_members)
    {
        if (INVALID_SOCKET != m_members->m_listenSocket)
        {
            fd_set         readFDS, writeFDS, exceptFDS;
            int            result;
            struct timeval timeout;

            timeout.tv_sec  = 0;
            timeout.tv_usec = 10;

            FD_ZERO(&readFDS);
            FD_ZERO(&writeFDS);
            FD_ZERO(&exceptFDS);

            FD_SET(m_members->m_listenSocket, &readFDS);
            FD_SET(m_members->m_listenSocket, &exceptFDS);

            /* If there is a client connected */
            if (INVALID_SOCKET != m_members->m_clientSocket)
            {
                FD_SET(m_members->m_clientSocket, &readFDS);
                FD_SET(m_members->m_clientSocket, &exceptFDS);
            }

            result = select(m_members->m_listenSocket + 1, &readFDS, &writeFDS, &exceptFDS, &timeout);

            if (0 < result)
            {
                /* New Client Connection available */
                if (FD_ISSET(m_members->m_listenSocket, &readFDS))
                {
                    /* Accept a client socket */
                    m_members->m_clientSocket = accept(m_members->m_listenSocket, nullptr, nullptr);
                    if (INVALID_SOCKET == m_members->m_clientSocket)
                    {
                        printf("accept failed\n");
                    }
                }

                /* Client Ready to read */
                if (FD_ISSET(m_members->m_clientSocket, &readFDS))
                {
                    const size_t bufferLength = 300U;
                    char         recvbuf[bufferLength];
                    int          result = recv(m_members->m_clientSocket, recvbuf, bufferLength, 0);

                    if (0 < result)
                    {
                        for (uint8_t idx = 0; idx < result; idx++)
                        {
                            m_members->m_rcvQueue.push(static_cast<uint8_t>(recvbuf[idx]));
                        }
                    }
                    else
                    {
                        /* Client disconnected or error on the socket. */
                        m_members->m_clientSocket = INVALID_SOCKET;
                    }
                }
            }
        }
    }
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SocketServer::close()
{
    if (nullptr != m_members)
    {
        /* Close the listening socket. */
        if (INVALID_SOCKET != m_members->m_listenSocket)
        {
#ifdef _WIN32
            closesocket(m_members->m_listenSocket);
#else
            close(m_members->m_listenSocket);
#endif
        }
    }

#ifdef _WIN32
    /* Terminate the use of the Winsock 2 DLL. */
    WSACleanup();
#endif
}

bool SocketServer::getByte(uint8_t& byte)
{
    if (nullptr != m_members)
    {
        if (false == m_members->m_rcvQueue.empty())
        {
            byte = m_members->m_rcvQueue.front();
            m_members->m_rcvQueue.pop();
            return true;
        }
    }
    return false;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
