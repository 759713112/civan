#include "../civan/civan.h"
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


int sock;
void test_fiber() {
    CIVAN_LOG_INFO(g_logger) << "test_fiber";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton (AF_INET, "192.168.6.2", (void*)&addr.sin_addr.s_addr);
    addr.sin_port = htons(2223);

    
    int rt = connect(sock,  (struct sockaddr*)&addr, sizeof(addr));    
    if (rt != 0) {
        if (errno == EINPROGRESS) {
            civan::IOManager::GetThis()->addEvent(sock, civan::IOManager::WRITE, []() {
                CIVAN_LOG_INFO(g_logger) << "write callback";
                civan::IOManager::GetThis()->cancelEvent(sock, civan::IOManager::READ);
            });
            civan::IOManager::GetThis()->addEvent(sock, civan::IOManager::READ, []() {
                CIVAN_LOG_INFO(g_logger) << "read callback";
            });
        } else {
            CIVAN_LOG_INFO(g_logger) << "connected failed";
        }
        
    }

}

void test1() {
    civan::IOManager iom(2, false);
    
    iom.schedule(&test_fiber);
}

civan::Timer::ptr s_timer;
void testTimer() {
    civan::IOManager iom(2);
    // iom.addTimer(500, [](){
    //     CIVAN_LOG_INFO(g_logger) << "hello timer";

    // }, true);
    s_timer = iom.addTimer(1000, [](){
        CIVAN_LOG_INFO(g_logger) << "hello timer";
        static int i = 5;
        if (i == 0) {
            s_timer->reset(5000, true);
        }
        i--;
    }, true);
}

void test_epoll_wait() {

    civan::IPv4Address::ptr addr = civan::IPv4Address::Create("192.168.6.2", 2223);
    civan::Socket::ptr sock(new civan::Socket(AF_INET, SOCK_STREAM));
    sock->bind(addr);
    sock->listen();
    CIVAN_LOG_INFO(g_logger) << "listend";
    while (1) {
        civan::Socket::ptr connected = sock->accept();
        CIVAN_LOG_INFO(g_logger) << "sleep";
        sleep(20);
        CIVAN_LOG_INFO(g_logger) << "sleep wake";
    }
}
civan::IOManager::ptr iom;
int main(int argc, char const *argv[])
{    
    iom.reset(new civan::IOManager(15, true, "try"));
    iom->schedule(&test_epoll_wait);
    return 0;

}
