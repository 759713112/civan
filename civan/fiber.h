#ifndef __CIVAN_FIBER__
#define __CIVAN_FIBER__

#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"


namespace civan {
class Scheduler;
class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    
    typedef std::shared_ptr<Fiber> ptr;
    enum State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEPT
    };
private: 
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    void reset(std::function<void()> cb);
    //切换到当前协程
    void swapIn();
    //切换到后台
    void swapOut();

    void call();
    void back();

    uint64_t getId() const { return m_id; }
    State getState() const { return m_state; }
public:
    //设置当前协程
    static void SetThis(Fiber* f);
    //返回当前协程
    static Fiber::ptr GetThis();
    
    //协程切换到后台并且切换到Ready状态
    static void YieldToReady();
    //协程切换到后台并且切换到Hold状态
    static void YieldToHold();
    static uint64_t TotalFibers();
    static uint64_t GetFiberId();

    static void MainFunc();
    static void CallerMainFunc();



private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;

    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;
};




} //namespace civan 



#endif