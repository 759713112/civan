#ifndef __CIVAN_UTIL_H__
#define __CIVAN_UTIL_H__
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string>
#include <sys/types.h>
#include <pthread.h>
#include <vector>
#include <execinfo.h>
#include "log.h"
#include <string>
#include "fiber.h"
#include <sys/time.h>
namespace civan {



pid_t GetThreadId();


uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");


//时间ms
uint64_t GetCurrentMs();

uint64_t GetCurrentUs();

std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");

class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

} //namespace civan
#endif