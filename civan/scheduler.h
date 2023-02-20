#ifndef __CIVAN_SCHEDULER_H__
#define __CIVAN_SCHEDULER_H__

#include "fiber.h"
#include "thread.h"
#include <vector>
#include <list>
#include "log.h"
namespace civan {
static civan::Logger::ptr g_logger2 = CIVAN_LOG_NAME("system");
class  Scheduler{
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    static Scheduler* GetThis();
    static Fiber* GetMainFiber();

    void start();
    void stop();

    template <typename FiberOrCb>
    void schedule(FiberOrCb fb, int thread = -1) {
        bool need_tickle = m_fibers.empty();
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fb, thread);
        }
        
        if (need_tickle) {
            tickle();
        }

    }

    template <typename InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle |= scheduleNoLock(&*begin, -1);
                begin++;
            }
        }
        if (need_tickle) {
            tickle();
        }
    }

protected:
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();

    void setThis();
private:
    template <typename FiberOrCb>
    bool scheduleNoLock(FiberOrCb fb, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fb, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f, int thr)
            :fiber(f), thread(thr) {}
        
        FiberAndThread(Fiber::ptr* f, int thr): thread(thr) {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f, int thr) 
            :cb(f), thread(thr) {

        }

        FiberAndThread(std::function<void()>* f, int thr) 
            :thread(thr) {
            cb.swap(*f);
        }

        //给stl用
        FiberAndThread()
            :thread(-1) {

        }

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
        

    };
private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;
    //std::map<int, std::list<FiberAndThread>>
    Fiber::ptr m_rootFiber;
    std::string m_name;


protected:
    std::vector<int> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};
    bool m_stopping = true;
    bool m_autoStops = false;
    int m_rootThread = 0;


};

}

#endif