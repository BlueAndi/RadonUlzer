#include <stdint.h>

class SocketServer
{
public:
    SocketServer();

    ~SocketServer();

    bool init();

    void sendMessage(const uint8_t* buf, int length);

    int available();

    int getByte();

private:
    void processRx();

};
