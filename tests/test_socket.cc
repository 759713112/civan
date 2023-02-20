#include "civan/socket.h"
#include "civan/log.h"
#include "civan/iomanager.h"
#include "civan/address.h"

civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

void testsocket() {
    civan::IPAddress::ptr addr = civan::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr) {
        CIVAN_LOG_INFO(g_logger) << "find address";
    } else {
        CIVAN_LOG_ERROR(g_logger) << "address failed";
        return;
    }
    civan::Socket::ptr sock = civan::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (sock->connect(addr)) {
        CIVAN_LOG_INFO(g_logger) << "connected";
    } else {
        CIVAN_LOG_ERROR(g_logger) << "connect failed";
        return;
    }

    const char buf[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buf, sizeof(buf));
    if (rt < 0) {
        CIVAN_LOG_ERROR(g_logger) << "send error";
        return;
    } 

    std::string buff;
    buff.resize(4096);

    rt = sock->recv(&buff[0], buff.size());
    if (rt < 0) {
        CIVAN_LOG_ERROR(g_logger) << "recv error";
        return;
    } 
    CIVAN_LOG_INFO(g_logger) << buff;

}

int main(int argc, char const *argv[])
{
    civan::IOManager iom;
    iom.schedule(testsocket);
    /* code */
    return 0;
}
