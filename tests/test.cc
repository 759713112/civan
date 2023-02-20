#include <iostream>
#include "../civan/log.h"
#include "../civan/util.h"
#include <thread>

int main(int argc, char const* argv[])
{
    civan::Logger::ptr logger(new civan::Logger);
    logger->addAppender(civan::LogAppender::ptr(new civan::StdoutLogAppender));
    logger->addAppender(civan::LogAppender::ptr(new civan::FileLogAppender("try.txt")));
    

    //logger->log(civan::LogLevel::DEBUG, event);
    std::cout<<"hello" << std::endl;

    CIVAN_LOG_INFO(logger) << "test fff";

    // auto l = civan::LoggerMgr::GetInstance()->getLogger("xx");
    // civan::LogEvent::ptr event(new civan::LogEvent (l, civan::LogLevel::DEBUG, 
    //                                                 __FILE__, __LINE__, 0,
    //                                                 civan::GetThreadId(), civan::GetFiberId(), time(0)));
    // l->log(civan::LogLevel::DEBUG, event);
    //
    return 0;

}