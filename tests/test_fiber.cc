#include "../civan/civan.h"
#include <vector>
civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

void run_in_fiber() {
    CIVAN_LOG_INFO(g_logger) << "run in fiber begin";
    civan::Fiber::GetThis()->YieldToHold();
    CIVAN_LOG_INFO(g_logger) << "run in fiber end";
    civan::Fiber::GetThis()->YieldToHold();
}

void test_fiber() {
    civan::Fiber::GetThis();
    
    CIVAN_LOG_INFO(g_logger) << "main begin";
    civan::Fiber::ptr fiber(new civan::Fiber(run_in_fiber));
    fiber->swapIn();
    CIVAN_LOG_INFO(g_logger) << "main after swapIn";
    fiber->swapIn();
    CIVAN_LOG_INFO(g_logger) << "main end1";
    fiber->swapIn();
    CIVAN_LOG_INFO(g_logger) << "main end2";
}

int main(int argc, char const *argv[])
{
    
    
    std::vector<civan::Thread::ptr> thrds;
    for (int i = 0; i < 3; i++) {
        thrds.push_back(civan::Thread::ptr(
                            new civan::Thread(test_fiber, "thread" + std::to_string(i))));
    }
    for (auto& t: thrds) {
        t->join();
    }
    return 0;
}
