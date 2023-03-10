#ifndef __CIVAN_MUTEX_H__
#define __CIVAN_MUTEX_H__

#include "noncopyable.h"
#include <functional>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>

namespace civan {


class Semaphore : Noncopyable  {
public:
    Semaphore(uint32_t count = 0) {
        if (sem_init(&m_semaphore, 0, count)) {
            throw std::logic_error("sem_init error");
        }
    }
    ~Semaphore() { sem_destroy(&m_semaphore); }

    void wait() {
        while (true) {
            if (!sem_wait(&m_semaphore)) {
                return;
            } else if (errno != EINTR) {
                throw std::logic_error("sem_wait error");
            }
        }
    }
    void notify() {
        if (sem_post(&m_semaphore)) {
            throw std::logic_error("sem_post error");
        }
    }
// private:
//     Semaphore(const Semaphore&) = delete;
//     Semaphore(const Semaphore&&) = delete;
//     Semaphore& operator=(const Semaphore&) = delete;
private:
    sem_t m_semaphore;
};



template<typename T>
class ScopedLockImpl : Noncopyable {
public:
    ScopedLockImpl(T& mutex) 
        :m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }
    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
        
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class Mutex : Noncopyable {
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex() {
        pthread_mutex_init(&m_mutex, nullptr);
    }
    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock() {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t* getPthreadMutex() {
        return &m_mutex;
    }
private:
    pthread_mutex_t m_mutex;
};


//轻量级锁 不会陷入挂起
class SpinLock : Noncopyable {
public:
    typedef ScopedLockImpl<SpinLock> Lock;
    SpinLock() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~SpinLock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

class CASLock : Noncopyable {
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock() {
        m_mutex.clear();
    }

    ~CASLock() {

    }

    void lock() {
        while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;
};

class NullMutex : Noncopyable {
    typedef ScopedLockImpl<NullMutex> Lock;
public:
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

template<typename T>
class ReadScopedLockImpl : Noncopyable {
public:
    ReadScopedLockImpl(T& mutex) 
        :m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
        
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};


template<typename T>
class WriteScopedLockImpl : Noncopyable {
public:
    WriteScopedLockImpl(T& mutex) 
        :m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }
    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
        
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};


class RWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }
private:
    pthread_rwlock_t m_lock;
};

class NullRWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<NullRWMutex> NullReadLock;
    typedef WriteScopedLockImpl<NullRWMutex> NullWriteLock;
    NullRWMutex() {}
    ~NullRWMutex() {}

    void rdlock() {}

    void wrlock() {}

    void unlock() {}
};

class Condition : Noncopyable
{
public:
    explicit Condition(Mutex& mutex)
    : mutex_(mutex)
    {
        if (pthread_cond_init(&pcond_, NULL)) {
            throw std::logic_error("condition error");
        }
    }

    ~Condition()
    {
        pthread_cond_destroy(&pcond_);
    }

    void wait()
    {
        while (true) {
            if (!pthread_cond_wait(&pcond_, mutex_.getPthreadMutex())) {
                return;
            } else if (errno != EINTR) {
                throw std::logic_error("cond_wait error");
            }
        }
    }

    // returns true if time out, false otherwise.
    bool waitForSeconds(double seconds) {
        struct timespec abstime;
        // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
        clock_gettime(CLOCK_REALTIME, &abstime);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
    }

    void notify()
    {
        if (pthread_cond_signal(&pcond_)) {
            throw std::logic_error("condition notify error");
        }
    }

    void notifyAll()
    {
        if (pthread_cond_broadcast(&pcond_)) {
            throw std::logic_error("condition broadcast error");
        }
    }

private:
    Mutex& mutex_;
    pthread_cond_t pcond_;
};

} //namespace civan

#endif