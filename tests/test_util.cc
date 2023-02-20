#include "../civan/civan.h"
#include <assert.h>

civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

void test_assert() {
    CIVAN_LOG_INFO(g_logger) << civan::BacktraceToString(10, 2, "    ");
}

int main(int argc, char const *argv[])
{
    test_assert();
    CIVAN_ASSERT(false);
    return 0;
}
