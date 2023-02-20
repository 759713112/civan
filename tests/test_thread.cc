#include "../civan/civan.h"
#include  <unistd.h>

civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

void func1() {
    CIVAN_LOG_INFO(g_logger) << " thread name: " << civan::Thread::GetName()
                                    << " thread name: " << civan::Thread::GetThis()->getName()
                                    << " thread id: " << civan::GetThreadId()
                                    << " thread id: " << civan::Thread::GetThis()->getId();
    sleep(20);
    
}

void func2() {
    while (true)
    CIVAN_LOG_INFO(g_logger) << "++++++++++++++++++++++++++++++++++++++++";
}

void func3() {
    while (true)
    CIVAN_LOG_INFO(g_logger) << "0000000000000000000000000000000000000000";
}
int main(int argc, char const *argv[])
{
    std::vector<civan::Thread::ptr> thrs;
    YAML::Node root = YAML::LoadFile("/home/dell/jqchen/cpp_project/civan/bin/conf/test.yml");
    civan::Config::LoadFromYaml(root);
    for (int i = 0; i < 5; i++) {
        civan::Thread::ptr tmp(new civan::Thread(func2, "thread"+std::to_string(i)));
        civan::Thread::ptr tmp2(new civan::Thread(func3, "thread"+std::to_string(i)));
        thrs.push_back(tmp);
        thrs.push_back(tmp2); 
    }
    
    for (auto t : thrs) {
        t->join();
    }
    return 0;
}
