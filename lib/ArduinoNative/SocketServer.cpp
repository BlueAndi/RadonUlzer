#include "SocketServer.h"
#include <stdio.h>

#ifdef _WIN32
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <queue>
#pragma comment(lib, "Ws2_32.lib")
#endif

#define DEFAULT_BUFLEN  512
#define DEFAULT_PORT    "65432"
#define MAX_CONNECTIONS 1

static std::queue<uint8_t> m_rcvQueue;
static SOCKET              m_clientSocket = INVALID_SOCKET;
static SOCKET              m_listenSocket = INVALID_SOCKET;

SocketServer::SocketServer()
{
}

SocketServer::~SocketServer()
{
    WSACleanup();
}

bool SocketServer::init()
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
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
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

    iResult = listen(m_listenSocket, MAX_CONNECTIONS);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(m_listenSocket);
        WSACleanup();
        success = false;
    }

    return success;
}

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
                char recvbuf[DEFAULT_BUFLEN];
                int  iResult = recv(m_clientSocket, recvbuf, DEFAULT_BUFLEN, 0);

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

void SocketServer::sendMessage(const uint8_t* buf, int length)
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
