#include "log.h"
#include <map>
#include <iostream>
#include <functional>
#include <time.h>
#include <string.h>
#include "macro.h"
#include "config.h"


namespace civan {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch(level) {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;
    
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOWN";
    }
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v) \
    if(str == #v) { \
        return LogLevel::level; \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y:%m:%d %H:%M:%S")
        : m_format(format) {
            if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
    
        os << buf;
    }
private:
    std::string m_format;
};


class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem (const std::string& str) 
    : m_string(str) {

    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem (const std::string& str = "") 
    {

    }
    void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
private:

};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
                const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            , const std::string& thread_name)
    :m_file(file)
    ,m_line(line)
    ,m_elapse(elapse)
    ,m_threadId(thread_id)
    ,m_fiberId(fiber_id)
    ,m_time(time)
    ,m_logger(logger)
    ,m_level(level)
    ,m_threadName(thread_name) {
}
void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf); 
    }
}
LogEvent::~LogEvent() {

}

LogEventWrap::LogEventWrap(LogEvent::ptr e)
                            : m_event(e) {

}
LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event); 
}
std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}


Logger::Logger(const std::string& name, uint32_t buffer_node_size
        , int flushInterval) 
    : m_name(name)
    , m_level(LogLevel::DEBUG)
    , currentBuffer(new ByteArray(buffer_node_size))
    , bufferToWrite(new ByteArray(buffer_node_size))
    , m_cond(m_mutex)
    , flushInterval_(flushInterval)
    , running_(true)
    , m_thread(std::bind(&Logger::asyncLogFunc, this), "log" + m_name) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%T%n"));
}


void Logger::setFormatter(LogFormatter::ptr val) {
    Mutex::Lock l(m_mutex);
    m_formatter = val;
}
void Logger::setFormatter(const std::string& val) {
    LogFormatter::ptr new_val(new LogFormatter(val));
    if (new_val->isError()) {
        std::cout << "Log formatter name = " << m_name << " value= " << val << "invalid" << std::endl; 
        return;
    }
    setFormatter(new_val);
}



LogFormatter::ptr Logger::getFormatter() {
    return m_formatter;
}

void Logger::addAppender(LogAppender::ptr appender) {
    Mutex::Lock l(m_mutex);
    // if (!appender->getFormatter()) {
    //     appender->setFormatter(m_formatter);
    // }
    m_appenders.push_back(appender);
}

void Logger::setStdoutAppender() {
    Mutex::Lock l(m_mutex);
    if (!m_stdoutAppender) {
        m_stdoutAppender.reset(new StdoutLogAppender());
    }
}


void Logger::deleteAppender(LogAppender::ptr appender) {
    Mutex::Lock l(m_mutex);
    for (auto it = m_appenders.begin();
            it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() {
    Mutex::Lock l(m_mutex);
    m_appenders.clear();
    m_stdoutAppender.reset();

}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        auto self = shared_from_this();

        std::string log_string = m_formatter->format(self, level, event);
        if (m_stdoutAppender) {
            m_stdoutAppender->log(level, log_string);
        }
        Mutex::Lock l(m_mutex);
        currentBuffer->writeStringWithoutLength(log_string);

    }
    
}
void Logger::start() {
    running_ = false;
    // thread_.start();
}
void Logger::stop() {
    running_ = false;
    // cond_.notify();
    // thread_.join();
}

void Logger::asyncLogFunc() {
    CIVAN_ASSERT(running_);
    while (running_) {
        {
            Mutex::Lock l(m_mutex);
            if (!currentBuffer->getSize()) {
                m_cond.waitForSeconds(flushInterval_);
            }
            currentBuffer.swap(bufferToWrite);
        }
        bufferToWrite->setPosition(0);
        //std::cout << m_name << " go to write " << bufferToWrite->getReadSize() << std::endl;
        //CIVAN_ASSERT(bufferToWrite->getSize());
        if (bufferToWrite->getReadSize()) {
            for (auto& appender : m_appenders) {
                appender->log(bufferToWrite);
            }
            bufferToWrite->clear();
        }
        

        
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}  

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

std::string Logger::toYamlString() {
    Mutex::Lock l(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    node["level"] = LogLevel::ToString(m_level);
    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    for (auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

// void LogAppender::setFormatter(LogFormatter::ptr val) { 
//     Mutex::Lock l(m_mutex);
//     m_formatter = val; 
// }
// LogFormatter::ptr LogAppender::getFormatter() {
//     Mutex::Lock l(m_mutex);
//     return m_formatter;
// }
// void LogAppender::setFormatter (const std::string& val) {
//     LogFormatter::ptr new_val(new LogFormatter(val));
//     if (new_val->isError()) {
//         std::cout << "Log formatter name = "  << " value= " << val << "invalid" << std::endl; 
//         return;
//     }
//     Mutex::Lock l(m_mutex);
//     m_formatter = new_val;
// }



FileLogAppender::FileLogAppender(const std::string& name)
    : LogAppender(), m_filename(name) {
    m_filestream.open(m_filename, std::ios::app);
}

void StdoutLogAppender::log(LogLevel::Level level, const std::string& log_string) {
    if (level >= m_level) {
        Mutex::Lock l(m_mutex);
        std::cout << log_string;
    }
}

std::string StdoutLogAppender::toYamlString() {
    Mutex::Lock l(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    // if (m_formatter) {
    //     node["formatter"] = m_formatter->getPattern();
    // }
    node["level"] = LogLevel::ToString(m_level);
    std::stringstream ss;
    ss << node;
    return ss.str();
}

std::string FileLogAppender::toYamlString() {
    Mutex::Lock l(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["level"] = LogLevel::ToString(m_level);
    // if (m_formatter) {
    //     node["formatter"] = m_formatter->getPattern();
    // }
    node["file"] = m_filename;
    std::stringstream ss;
    ss << node;
    return ss.str();
}

bool FileLogAppender::reopen() {
    Mutex::Lock l(m_mutex);
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !m_filestream;

}

void FileLogAppender::log(ByteArray::ptr bufferToWrite) {
    bufferToWrite->writeToFile(m_filestream);
    m_filestream.flush();
}

LogFormatter::LogFormatter(const std::string& pattern)
    :m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();


}

//%xxx %xxx{xxx} %%
void LogFormatter::init() {
    //str format type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    //std::cout<< m_pattern << std::endl;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i+1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }
        int fmt_status = 0;
        std::string fmt;
        std::string str;
        size_t fmt_begin = 0;
        size_t n = i + 1;
        while(n < m_pattern.size()) {
            if (!fmt_status && !isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}') {
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;  //解析format
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 2;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }

            str = m_pattern.substr(i + 1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout<< "patten parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 1));
            m_error = true;
        } else if (fmt_status == 2) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }
    }
    //尾巴也加进来
    if (!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, " ", 0));
    }

    //%m消息体 %p level %r 启动后的时间 %c 日志名称 %t 线程id %n换行  %d 时间 %f 文件名 %l行号

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
        //{"m", [](const std::string& fmt) { return FormatItem::ptr(new MessageFormatItem(fmt)); }}
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); } }
        
        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem),
        XX(N, ThreadNameFormatItem),

#undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back( FormatItem::ptr(new StringFormatItem(std::get<0>(i))) );
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back( FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")) );
                m_error = true; 
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
        
        //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }


}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger("root"));
    m_root->setStdoutAppender();

    m_loggers[m_root->m_name] = m_root;
    init();
}

void LoggerManager::init() {

}


Logger::ptr LoggerManager::getLogger(const std::string& name) {
    Mutex::Lock l(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    Logger::ptr logger(new Logger(name));
    //logger->m_root = m_root;
    m_loggers.insert(std::make_pair(name, logger));
    return logger;
}

std::string LoggerManager::toYamlString() {
    Mutex::Lock l(m_mutex);
    YAML::Node node;
    for (auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}  

struct LogAppenderDefine {
    int type;  //1 File 2 Stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    //std::string formatter;
    std::string file;
    bool operator==(const LogAppenderDefine& rhs) const {
        return type == rhs.type
            && level == rhs.level
            && file == rhs.file;
    }
};

//LogAppenderDefine
template<>
class LexicalCast<std::string, LogAppenderDefine> {
public:
    LogAppenderDefine operator() (const std::string& v) {
        YAML::Node node = YAML::Load(v);
        LogAppenderDefine res;
        std::string type = node["type"].as<std::string>();
        if (type == "FileLogAppender") {
            res.type = 1;
            if (!node["file"].IsDefined()) {
                std::cout << "log config error: " << " not file: " << std::endl;
            }
            res.file =  node["file"].as<std::string>(); 
        } else if (type == "StdoutLogAppender") {
            res.type = 2;
        } else {
            std::cout << "log config error: " << " appender type is invalid: " << type << std::endl;
         }
        res.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
        // res.formatter = node["formatter"].IsDefined() ? 
        //                         node["formatter"].as<std::string>() : "";
        
        return res;
    }
};

template<>
class LexicalCast<LogAppenderDefine, std::string> {
public:
    std::string operator()(const LogAppenderDefine& v) {
        YAML::Node node;
        if (v.type == 1) {
            node["type"] = "FileLogAppender";
            node["file"] = v.file;
            
        } else if (v.type == 2) {
            node["type"] = "StdoutLogAppender";
        }
        node["level"] = LogLevel::ToString(v.level);
        //node["formatter"] = v.formatter;
        

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

class LogDefine {
public:
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;
    uint32_t buffer_node_size;
    bool operator==(const LogDefine& rhs) const {
        return name == rhs.name
            && level == rhs.level
            && formatter == rhs.formatter;
            //&& appenders == rhs.appenders;
    }
    
    bool operator<(const LogDefine& rhs) const {
        return name < rhs.name;
    }

};


//LogDefine
template<>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator() (const std::string& v) {
        std::cout<< v << std::endl;
        YAML::Node node = YAML::Load(v);
        LogDefine res;
        if (!node["name"].IsDefined()) {
            std::cout << "log config error: name is null"  << std::endl;
            return res;
        }
        res.name = node["name"].as<std::string>();
        res.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
        res.formatter = node["formatter"].IsDefined() ? 
                                node["formatter"].as<std::string>() : "";
        res.buffer_node_size = node["buffer_node_size"].IsDefined() ? 
                                node["buffer_node_size"].as<uint32_t>() : 4096;
        
        if (node["appenders"].IsDefined()) {
            std::stringstream ss;
            ss.str("");
            ss << node["appenders"];
            res.appenders = LexicalCast<std::string, std::vector<LogAppenderDefine> >()(ss.str());
        }
        return res;
    }
};

template<>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& v) {
        YAML::Node node;
        node["name"] = v.name;
        node["level"] = LogLevel::ToString(v.level);
        node["formatter"] = v.formatter;
        node["appenders"] = YAML::Load(
                    LexicalCast<std::vector<LogAppenderDefine>, std::string>()(v.appenders));
        node["buffer_node_size"] = std::to_string(v.buffer_node_size);
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// std::set<LogDefine> logDefines;
// LogDefine tmp;
// tmp.name = "123456";
// tmp.level = LogLevel::DEBUG;
// logDefines.insert()
civan::ConfigVar<std::set<LogDefine> >::ptr g_log_defines =
    civan::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
    LogIniter() {
        g_log_defines->addListener([](const std::set<LogDefine>& old_value
                                            , const std::set<LogDefine>& new_value) {
            CIVAN_LOG_DEBUG(CIVAN_LOG_ROOT()) << "on log change"; 
            for (auto& i : new_value) {
                std::cout << i.formatter << std::endl;
                auto it = old_value.find(i); 
                if (it == old_value.end()) {
                    //新增
                    auto logger = CIVAN_LOG_NAME(i.name);
                    logger->setLevel(i.level);
                    if (!i.formatter.empty()) {
                        logger->setFormatter(i.formatter);   
                    }
                    logger->clearAppenders();
                    for (auto& a : i.appenders) {
                        if (a.type == 1) {
                            LogAppender::ptr ap;
                            ap.reset(new FileLogAppender(a.file));
                            ap->setLevel(a.level);
                            logger->addAppender(ap);
                        } else if (a.type == 2) {
                            logger->setStdoutAppender();
                        }
                        // if (!a.formatter.empty()) {
                        //     ap->setFormatter(a.formatter);
                        // }   
                    }
                    
                } else {
                    if (!(i == *it)) {
                        //修改的logger
                        auto logger = CIVAN_LOG_NAME(i.name);
                        logger->setLevel(i.level);

                        if (!i.formatter.empty() && i.formatter != it->formatter) {
                            logger->setFormatter(i.formatter);   
                        }

                        if (i.appenders != it->appenders) {
                            logger->clearAppenders();
                            for (auto& a : i.appenders) {
                                if (a.type == 1) {
                                    LogAppender::ptr ap;
                                    ap.reset(new FileLogAppender(a.file));
                                    ap->setLevel(a.level);
                                    logger->addAppender(ap);
                                } else if (a.type == 2) {
                                    logger->setStdoutAppender();
                                }
                            }
                        }

                    }
                }

            }
            
            for (auto& i : old_value) {
                auto it = new_value.find(i);
                if (it == new_value.end()){
                    //删除logger
                    auto logger = CIVAN_LOG_NAME(i.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }
        });
    }
};


static LogIniter __log_init;
}; //namespace civan



    