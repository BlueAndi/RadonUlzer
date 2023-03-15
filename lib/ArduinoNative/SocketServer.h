#include <stdint.h>
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

class SocketServer
{
public:
    static SocketServer& getInstance();

    bool init();

    void process();

    void sendMessage(const uint8_t* buf, int length);

    int available();

    int getByte();

private:

    SocketServer()
    {
    }

    ~SocketServer()
    {
        WSACleanup();
    }
};
