#ifndef __CIVAN_STORE_BUFFER_H__
#define __CIVAN_STORE_BUFFER_H__


#include <stdint.h>
#include <atomic>
#include "civan/mutex.h"
namespace civan {
namespace store {
namespace buffer {

class raw {
public:
    typedef RWMutex RWMutexType;
    typedef shared_ptr<raw> ptr;
    char* data;
    uint64_t len;
    std::atomic<int> nref;
    //mutable RWMutexType m_mutex;

};

class ptr {
public:
    raw::ptr m_raw;
    uint64_t off, len;
};

class list {
public:
    std::list<ptr> _buffers;
    uint64_t _len;
    ptr append_buffer;

};

}
}
}



#endif