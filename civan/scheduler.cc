#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
namespace civan{

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;


static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name) {
    CIVAN_ASSERT(threads > 0);

    if (use_caller) {
        //以当前主线程的主协程为Fiber主协程
        civan::Fiber::GetThis();
        --threads;

        t_scheduler = this;

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        civan::Thread::SetName(m_name);
        t_scheduler_fiber = m_rootFiber.get();
        
        m_rootThread = civan::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1; 
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    CIVAN_ASSERT(m_stopping);
    if (GetThis() == this) {
        t_scheduler = nullptr;
    }
}



Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if (!m_stopping) {
        return;
    }
    
    m_stopping = false;

    CIVAN_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    for (size_t i=0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                            , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }

    lock.unlock();

}

void Scheduler::stop() {
    m_autoStops = true;
    if (m_rootFiber 
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        CIVAN_LOG_INFO(g_logger) << this << " stopped ";
        m_stopping = true;

        if (stopping()) {
            return;
        }
    }

    //bool exit_on_this_fiber = false;
    if (m_rootThread != -1) {
        CIVAN_ASSERT(GetThis() == this);
    } else {
        CIVAN_ASSERT(GetThis() != this);
    }
    
    m_stopping = true;
    for (size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if (m_rootFiber) {
        tickle();
    }
    if (m_rootFiber) {
        // while (!stopping()) {
        //     if (m_rootFiber->getState() == Fiber::TERM
        //             || m_rootFiber->getState() == Fiber::EXCEPT) {
        //         m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
        //         CIVAN_LOG_INFO(g_logger) << "root fiber is term, reset";
        //         t_scheduler_fiber = m_rootFiber.get();
        //     } 
        //     m_rootFiber->call();
        // }
        if (!stopping()) {
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }
    for (auto& i : thrs) {
        i->join();
    }
    // if (exit_on_this_fiber) {

    // }
}

void Scheduler::tickle() {
    CIVAN_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStops && m_stopping 
            && m_fibers.empty() && m_activeThreadCount == 0; 
}
void Scheduler::idle() {
    CIVAN_LOG_INFO(g_logger) << "idle";
    while (!stopping()) {
        Fiber::GetThis()->YieldToHold();
    }
}



void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    CIVAN_LOG_INFO(g_logger) << m_name << "run";
    //Fiber::GetThis();
    setThis();
    set_hook_enable(true);
    if (civan::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    } 

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    CIVAN_LOG_INFO(g_logger) << "idle_fiber id: " << idle_fiber->getId();


    FiberAndThread ft;
    while (!stopping()) {
        //idle_fiber.reset(new Fiber(std::bind(&Scheduler::idle, this)));
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            //取出队列中待执行fiber
            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                if (it->thread != -1 && it->thread != civan::GetThreadId()) {
                    ++it;
                    CIVAN_LOG_INFO(g_logger) << "idle_fiber id: " << idle_fiber->getId();
                    tickle_me = true;
                    continue;
                }
                CIVAN_ASSERT(it->fiber || it->cb);
                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it);
                ++m_activeThreadCount;
                is_active = true;
                break;

            }
        }
        if (tickle_me) {
            tickle();
        }

        if (ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            CIVAN_LOG_INFO(g_logger) << "swap out1";
            --m_activeThreadCount;
            if (ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if (ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                CIVAN_LOG_INFO(g_logger) << "be hold ";
                ft.fiber->m_state = Fiber::HOLD;
            }

            ft.reset();
        } else if (ft.cb) {
            if (cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
                ft.cb = nullptr;
            }
            ft.reset();
            
            cb_fiber->swapIn();
            CIVAN_LOG_INFO(g_logger) << "swap out2";
            --m_activeThreadCount;
            if (cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if (cb_fiber->getState() == Fiber::TERM
                    || cb_fiber->getState() == Fiber::EXCEPT) {
                cb_fiber.reset();
            } else {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            if (is_active) {
                --m_activeThreadCount;
                continue;
            }
            if (idle_fiber->getState() == Fiber::TERM) {
                CIVAN_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if (idle_fiber->getState() != Fiber::TERM 
                    && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
            
        }

    }

}


} // namespace civan