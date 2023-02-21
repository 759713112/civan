#include "fiber.h"
#include "config.h"
#include "macro.h"
#include <atomic>
#include "scheduler.h"
namespace civan {

static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr;  //当前协程
static thread_local Fiber::ptr t_threadFiber = nullptr;  //当前线程的主协程
 
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 1024*1024, "fiber stack size");


class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if (getcontext(&m_ctx)) {
        CIVAN_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
    //CIVAN_LOG_DEBUG(g_logger) << "construct fiber id = " << m_id;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    : m_id(++s_fiber_id)
     ,m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        CIVAN_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    if (!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
     
    //CIVAN_LOG_DEBUG(g_logger) << "construct fiber id = " << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_stack) {
        //CIVAN_LOG_DEBUG(g_logger) << "aaa:destroy fiber id" << m_id << " state " <<m_state << " name" << Thread::GetName();
        CIVAN_ASSERT(m_state == TERM || m_state == INIT
                        || m_state == EXCEPT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        CIVAN_ASSERT(!m_cb);
        CIVAN_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }
    CIVAN_LOG_DEBUG(g_logger) << "destroy fiber id = " << m_id;
}

//重置协程函数 并重置状态
void Fiber::reset(std::function<void()> cb) {
    CIVAN_ASSERT(m_stack);
    CIVAN_ASSERT(m_state == TERM
                || m_state == INIT
                || m_state == EXCEPT);
    m_cb = cb;
    if (getcontext(&m_ctx)) {
        CIVAN_ASSERT2(false, "getcontex");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}


//GetMainFiber() 和 线程主协程切换
void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        CIVAN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::call() {
    CIVAN_ASSERT(GetThis() == t_threadFiber);
    SetThis(this);
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        CIVAN_ASSERT2(false, "swapcontex");
    }
}


//非GetMainFiber()切换
//切换到当前协程
void Fiber::swapIn() {
    SetThis(this);
    CIVAN_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if ( swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx) ) {
        CIVAN_ASSERT2(false, "swapcontex");
    }

}

//切换到后台
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());
        //m_state = EXEC;
    if ( swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx) ) {
        CIVAN_ASSERT2(false, "swapcontex");
    }
    
}


//设置当前协程
void Fiber::SetThis(Fiber* f) {
    t_fiber = f;
}

//返回当前协程
Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    //私有构造函数 将t_fiber = main_fiber.get()
    Fiber::ptr main_fiber(new Fiber);
    CIVAN_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

//协程切换到后台并且切换到Ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}

//协程切换到后台并且切换到Hold状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
    
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    CIVAN_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state =  TERM;
    } catch(std::exception& ex) {
        cur->m_state = EXCEPT;
        CIVAN_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << civan::BacktraceToString();
    } catch(...) {
        cur->m_state = EXCEPT;
        CIVAN_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << civan::BacktraceToString();
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    CIVAN_ASSERT2(false, "never get here");
}

void Fiber::CallerMainFunc() {
    CIVAN_LOG_INFO(g_logger) << "CallerMainFunc";
    Fiber::ptr cur = GetThis();
    CIVAN_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state =  TERM;
    } catch(std::exception& ex) {
        cur->m_state = EXCEPT;
        CIVAN_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
            << " fiber_id=" << cur->getId()
            << std::endl
            << civan::BacktraceToString();
    } catch(...) {
        cur->m_state = EXCEPT;
        CIVAN_LOG_ERROR(g_logger) << "Fiber Except"
            << " fiber_id=" << cur->getId()
            << std::endl
            << civan::BacktraceToString();
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();

    CIVAN_ASSERT2(false, "never get here");
}

} //namespace civan