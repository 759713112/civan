#ifndef __CIVAN_MACRO_H__
#define __CIVAN_MACRO_H__

#include <string.h>
#include <assert.h>
#include "util.h"


//告诉编译器上面成功几率大 下面成功几率小
#if defined __GNUC__ || defined __llvm__
#define CIVAN_LICKLY(x)        __builtin_expect(!!(x), 1)
#define CIVAN_UNLICKLY(x)      __builtin_expect(!!(x), 0)
#else 
# define CIVAN_LICKLY(x) (x)
# define CIVAN_UNLICKLY(x) (x)
#endif

#define CIVAN_ASSERT(x) \
    if (CIVAN_UNLICKLY(!(x))) { \
        CIVAN_LOG_ERROR(CIVAN_LOG_ROOT()) << "Assertion" << #x \
            << "\nbacktrace:\n" \
            << civan::BacktraceToString(100, 2, "    "); \
        assert(x); \
    } 

#define CIVAN_ASSERT2(x, w) \
    if (CIVAN_UNLICKLY(!(x)))  { \
        CIVAN_LOG_ERROR(CIVAN_LOG_ROOT()) << "Assertion" << #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << civan::BacktraceToString(100, 2, "    "); \
        assert(x); \
    } 



#endif