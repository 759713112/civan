#include "../civan/address.h"
#include "../civan/log.h"
#include <vector>
civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

void test_address() {
    std::vector<civan::Address::ptr> result;
    if (civan::Address::Lookup(result, "www.baidu.com:80")) {
        for (auto& i : result) {
            CIVAN_LOG_INFO(g_logger) << i->toString();
        }
    } else {
        CIVAN_LOG_ERROR(g_logger) << "Lookup failed";
    }

}

void test_iface() {
    std::multimap<std::string, std::pair<civan::Address::ptr, uint32_t>> results;
    bool v = civan::Address::GetInterfaceAddress(results);
    if (!v) {
        return;
    }
    for (auto& i : results) {
        CIVAN_LOG_INFO(g_logger) << i.first << "-" << i.second.first->toString();
    }
}


void test_ipv4() {
    auto addr = civan::IPAddress::Create("www.baidu.com");
    CIVAN_LOG_INFO(g_logger) << addr->toString();
}

int main(int argc, char const *argv[])
{
    test_address();
    test_iface();
    test_ipv4();
    return 0;
}
