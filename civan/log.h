#ifndef _CIVAN_LOG_H_
#define _CIVAN_LOG_H_

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdarg.h>
#include <map>
#include "singleton.h"
#include "util.h"
#include "thread.h"
#include "mutex.h"

#define CIVAN_LOG_LEVEL(logger, level) \
    if (logger->getLevel() <= level) \
        civan::LogEventWrap(civan::LogEvent::ptr(new civan::LogEvent(logger, level, \
                            __FILE__, __LINE__, 0, civan::GetThreadId(), \
                            civan::GetFiberId(), time(0), civan::Thread::GetName())) ).getSS()

#define CIVAN_LOG_DEBUG(logger) CIVAN_LOG_LEVEL(logger, civan::LogLevel::DEBUG)
#define CIVAN_LOG_INFO(logger) CIVAN_LOG_LEVEL(logger, civan::LogLevel::INFO)
#define CIVAN_LOG_WARN(logger) CIVAN_LOG_LEVEL(logger, civan::LogLevel::WARN)
#define CIVAN_LOG_ERROR(logger) CIVAN_LOG_LEVEL(logger, civan::LogLevel::ERROR)
#define CIVAN_LOG_FATAL(logger) CIVAN_LOG_LEVEL(logger, civan::LogLevel::FATAL)


#define CIVAN_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if (logger->getLevel() <= level) \
        civan::LogEventWrap( civan::LogEvent::ptr( new civan::LogEvent(logger, level, \
                __FILE__, __LINE__, 0, civan::GetThreadId(), \
                            civan::GetFiberId(), time(0), civan::Thread::GetName()) ) ).getEvent()->format(fmt, __VA_ARGS__)


#define CIVAN_LOG_ROOT() civan::LoggerMgr::GetInstance()->getRoot()
#define CIVAN_LOG_NAME(name) civan::LoggerMgr::GetInstance()->getLogger(name)
namespace civan {
class Logger;
class LoggerManager;
class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4, 
        FATAL = 5
    };
    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& );
};
//日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, 
            const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            , const std::string& thread_name);
    ~LogEvent();
    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; } 
    std::string getContent() const { return m_ss.str(); }

    std::stringstream& getSS() { return m_ss; }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }
    const std::string& getThreadName() const { return m_threadName; }
    void format(const char* fmt, ...);

    void format(const char* fmt, va_list al);
private:
    const char* m_file = nullptr;       //file name
    int32_t m_line = 0;                 //line number
    uint32_t m_elapse = 0;              //程序启动到现在的事件
    uint32_t m_threadId = 0; 
         
    uint32_t m_fiberId = 0;            //协程id
    uint64_t m_time;                    //事件错
    std::string m_content;
    std::stringstream m_ss;

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    
    std::string m_threadName;      
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    std::stringstream& getSS();

private:
    LogEvent::ptr m_event;
};



class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string& pattern);

    //%t   
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level, LogEvent::ptr event);
public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        //FormatItem(const std::string& fmt) { };
        virtual ~FormatItem() {};
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };
    void init();

    bool isError() const { return m_error; }
    std::string getPattern() const { return m_pattern; }
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;

};

//日志输出地
class LogAppender {
public:
    typedef SpinLock MutexType;
    typedef std::shared_ptr<LogAppender> ptr;
    LogAppender() : m_level(LogLevel::DEBUG) { }
    virtual ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    
    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter();
    void setFormatter(const std::string& val);

    LogLevel::Level getLevel() { return m_level; }
    void setLevel(LogLevel::Level var) { m_level = var; }
    virtual std::string toYamlString() = 0;
protected:
    LogLevel::Level m_level;
    MutexType m_mutex;
    LogFormatter::ptr m_formatter; 
};



//日志器
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef SpinLock MutexType;
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);

    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void deleteAppender(LogAppender::ptr appender);
    void clearAppenders();

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level val) { m_level = val; }
    const std::string& getName() { return m_name; }
    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);

    LogFormatter::ptr getFormatter();

    std::string toYamlString();
private:
    std::string m_name;
    LogLevel::Level m_level;
    std::list<LogAppender::ptr> m_appenders;
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
    MutexType m_mutex;
};

//输出到控制台
class StdoutLogAppender : public LogAppender {
public:
    StdoutLogAppender() : LogAppender() {}
    typedef  std::shared_ptr<StdoutLogAppender> ptr;
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    virtual std::string toYamlString() override;
};

class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    ~FileLogAppender() { m_filestream.close(); }
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;
    bool reopen();

    virtual std::string toYamlString() override;
private:
    std::string m_filename;
    std::ofstream m_filestream;

};

class LoggerManager {
public:
    LoggerManager();
    typedef SpinLock MutexType;
    Logger::ptr getLogger(const std::string& name);
    
    void init();

    Logger::ptr getRoot() const { return m_root; }

    std::string toYamlString();
private:
    MutexType m_mutex;
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

typedef civan::Singleton<LoggerManager> LoggerMgr;


} //namespace civan




#endif