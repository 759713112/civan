#include "../civan/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include "../civan/timer.h"
#include "civan/address.h"
#include "civan/socket.h"

civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");
void testConect() {
    civan::IPv4Address::ptr addr = civan::IPv4Address::Create("192.168.6.2", 2223);
    civan::Socket::ptr sock(new civan::Socket(AF_INET, SOCK_STREAM));
    if (sock->connect(addr)) {
        CIVAN_LOG_INFO(g_logger) << "connected";
        sleep(5);
    } else {
        CIVAN_LOG_INFO(g_logger) << "connect fail";
    }
    sock->close();


}

int main(int argc, char const *argv[])
{
    civan::IOManager iom(3);
    for (int i = 0; i < 1; i++) {
        iom.schedule(&testConect);
    }    
    return 0;
}
