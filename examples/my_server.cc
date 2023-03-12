#include "civan/http/http_server.h"
#include "civan/iomanager.h"
#include "civan/civan.h"
#include "civan/log.h"
#include "civan/config.h"
static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");
int iom_num = 0;
int thread_num_per_iom = 1;
void run() {
    //g_logger->setLevel(civan::LogLevel::Level::ERROR);
    civan::http::HttpServer::ptr my_server(new civan::http::HttpServer(true
                , civan::IOManager::GetThis(), iom_num, thread_num_per_iom));

    civan::Address::ptr addr = civan::IPv4Address::Create("0.0.0.0", 8120);
    if (!my_server->bind(addr)) {
        CIVAN_LOG_ERROR(g_logger) << "bind address error: " << addr->toString();
        return;
    }
    my_server->start();
}


int main(int argc, char const *argv[])
{
    if (argc > 2) {
        iom_num = atoi(argv[1]);
        thread_num_per_iom = atoi(argv[2]);
    }

    YAML::Node root = YAML::LoadFile("/home/dell/jqchen/cpp_project/civan/bin/conf/log.yml");
    civan::Config::LoadFromYaml(root);
    civan::IOManager iom(1, true, "main");
    iom.schedule(run);
    return 0;
}
