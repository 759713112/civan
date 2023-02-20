#ifndef __CIVAN_NONCOPYABLE_H__
#define __CIVAN_NONCOPYABLE_H__

namespace civan {

class Noncopyable {
public:
    Noncopyable() = default;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable) = delete;
    ~Noncopyable() = default;
};


}



#endif