#include <stdint.h>

class SocketServer
{
public:
    SocketServer();

    ~SocketServer();

    bool init();

    void sendMessage(const uint8_t* buf, int length);

    uint32_t available();

    int8_t getByte();

private:
    void processRx();

};
