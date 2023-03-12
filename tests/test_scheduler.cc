#include "../civan/civan.h"

civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

void test_fiber() {
    static int count = 10;
    CIVAN_LOG_INFO(g_logger) << "test in fiber" << count;
    sleep(1);
    
    if (--count > 0) {
        civan::Scheduler::GetThis()->schedule(&test_fiber);
    }
}

int main(int argc, char const *argv[])
{
    civan::Scheduler sc(3, true, "test");
    sc.start();
    sc.schedule(test_fiber);
    CIVAN_LOG_INFO(g_logger) << "stop?";
    sc.stop();
    CIVAN_LOG_INFO(g_logger) << "over";
    return 0;
}
