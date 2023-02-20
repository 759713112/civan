#include "../civan/hook.h"
#include "../civan/civan.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();
void test_sleep() {
    civan::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        CIVAN_LOG_INFO(g_logger) << "sleep 2";
    });

    iom.schedule([](){
        sleep(3);
        CIVAN_LOG_INFO(g_logger) << "sleep 3";
    });

    CIVAN_LOG_INFO(g_logger) << "test sleep";
}
int sock;
void test_sock() {
    CIVAN_LOG_INFO(g_logger) << "test_fiber";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    //fcntl(sock, F_SETFL, O_NONBLOCK);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton (AF_INET, "14.215.177.38", (void*)&addr.sin_addr.s_addr);
    addr.sin_port = htons(80);

    
    int rt = connect(sock,  (struct sockaddr*)&addr, sizeof(addr)); 
    if (rt) {
        CIVAN_LOG_INFO(g_logger) << "connect error";
        return;
    } else {
        CIVAN_LOG_INFO(g_logger) << "connect success";
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    std::string buff;
    buff.resize(4096);
    rt = recv(sock, &buff[0], buff.size(), 0);
    CIVAN_LOG_INFO(g_logger) << "recv rt=" << "errno=" << errno;

    if (rt <= 0) {
        return;
    }

    buff.resize(rt);
    CIVAN_LOG_INFO(g_logger) << buff;
}


int main(int argc, char const *argv[])
{
    //test_sock();
    civan::IOManager iom;
    iom.schedule(test_sock);

    return 0;
}
