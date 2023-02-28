#include "civan/http/http_server.h"
#include "civan/iomanager.h"
#include "civan/civan.h"
#include "civan/log.h"

static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("system");

void run() {
    g_logger->setLevel(civan::LogLevel::Level::ERROR);
    civan::http::HttpServer::ptr my_server(new civan::http::HttpServer(true, civan::IOManager::GetThis(), 5));

    civan::Address::ptr addr = civan::IPv4Address::Create("0.0.0.0", 8120);
    if (!my_server->bind(addr)) {
        CIVAN_LOG_ERROR(g_logger) << "bind address error: " << addr->toString();
        return;
    }
    my_server->start();
}


int main(int argc, char const *argv[])
{
    civan::IOManager iom(1, true, "main");
    iom.schedule(run);
    return 0;
}
