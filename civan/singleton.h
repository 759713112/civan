#ifndef __CIVAN_SINGLETON_H__
#define __CIVAN_SINGLETON_H__

namespace civan {

template <typename T>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template<class T>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};



}  //namespace civan




#endif