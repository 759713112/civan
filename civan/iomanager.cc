#include "iomanager.h"
#include "macro.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "string.h"
#include<algorithm>
namespace civan {

static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");

enum EpollCtlOp {
};


IOManager::FdContext::EventContext& IOManager::FdContext::getContext(Event event) {
    switch(event) {
        case READ:
            return read;
        case WRITE:
            return write;
        default:
            CIVAN_ASSERT2(false, "getcContext");
    }
}

void IOManager::FdContext::resetContext(EventContext& ctx) {
    ctx.scheduler = nullptr;
    ctx.fiber.reset();
    ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(Event event) {
    CIVAN_ASSERT(events & event);
    events = (Event)(events & ~event);
    EventContext& ctx = getContext(event);
    if (ctx.cb) {
        //采用指针  将ctx.cb swap成null
        ctx.scheduler->schedule(&ctx.cb);
    } else {
        ctx.scheduler->schedule(&ctx.fiber);
    }
    ctx.scheduler = nullptr;
    return;
}



IOManager::IOManager(size_t threads, bool use_caller, const std::string& name) 
    :Scheduler(threads, use_caller, name) {
    m_epfd = epoll_create(1);

    //int rt = pipe2(m_tickleFds, O_NONBLOCK);
    int rt = pipe(m_tickleFds);
    CIVAN_ASSERT(rt == 0);
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = m_tickleFds[0];

    rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
    CIVAN_ASSERT(rt == 0);

    rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
    CIVAN_ASSERT(rt == 0);

    contextResize(64);
    start();
}

IOManager::~IOManager() {
    stop();
    close(m_epfd);
    close(m_tickleFds[0]);
    close(m_tickleFds[0]);

    for (size_t i = 0; i < m_fdContexts.size(); i++) {
        if (m_fdContexts[i]) {
            delete m_fdContexts[i];
        }
    }
}

void IOManager::contextResize(size_t size) {
    m_fdContexts.resize(size);

    for (size_t i = 0; i < m_fdContexts.size(); ++i) {
        if (!m_fdContexts[i]) {
            m_fdContexts[i] = new FdContext;
            m_fdContexts[i]->fd = i;
        }
    }
}

//0 success, -1 error
int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
    FdContext* fd_ctx = nullptr;
    
    RWMutexType::ReadLock lock(m_mutex);
    
    if ((int)m_fdContexts.size() > fd) {
        fd_ctx = m_fdContexts[fd];
    } else {
        lock.unlock();
        RWMutexType::WriteLock lock2(m_mutex);
        contextResize(fd * 1.5);
        fd_ctx = m_fdContexts[fd];
    }

    FdContext::MutexType::Lock lock2(fd_ctx->mutex);
    if (fd_ctx->events & event) {
        CIVAN_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                    << " event=" << (EPOLL_EVENTS)event
                    << " fd_ctx.event=" << (EPOLL_EVENTS)fd_ctx->events;
        CIVAN_ASSERT(!(fd_ctx->events & event));
    }

    int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    epoll_event epevent;
    epevent.events = EPOLLET | fd_ctx->events | event;
    epevent.data.ptr = fd_ctx;
    //epevent.data.fd = fd;  //epevent.data为union

    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if (rt) {
        CIVAN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ","
            << op << "," << fd << "," << epevent.events << "):"
            << rt << "( " << errno << ") (" << strerror(errno) << ")";
        return -1;    
    }
    m_pendingEventCount++;
    fd_ctx->events = (Event)(fd_ctx->events | event);
    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    CIVAN_ASSERT(!event_ctx.scheduler 
                && !event_ctx.fiber 
                && !event_ctx.cb)
    event_ctx.scheduler = Scheduler::GetThis();
    if (cb) {
        event_ctx.cb.swap(cb);
    } else {
        event_ctx.fiber = Fiber::GetThis();
        CIVAN_ASSERT2(event_ctx.fiber->getState() == Fiber::EXEC
                      ,"state=" << event_ctx.fiber->getState());
    }
    return 0;
}

bool IOManager::delEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    if (!(event & fd_ctx->events)) {
        return false;
    }
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);


    Event new_event = (Event)(fd_ctx->events & ~event);
    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_event;
    epevent.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
        CIVAN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }

    --m_pendingEventCount;
    fd_ctx->events = new_event;

    FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
    fd_ctx->resetContext(event_ctx);
    return true;
}


//找到对应事件 取消其epoll 强制促发执行
bool IOManager::cancelEvent(int fd, Event event) {
    RWMutexType::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    if (!(event & fd_ctx->events)) {
        return false;
    }
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);


    Event new_event = (Event)(fd_ctx->events & ~event);
    int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = EPOLLET | new_event;
    epevent.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if(rt) {
        CIVAN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    fd_ctx->triggerEvent(event);
    --m_pendingEventCount;
    return true;
}

bool IOManager::cancelAll(int fd) {
    RWMutexType::ReadLock lock(m_mutex);
    if ((int)m_fdContexts.size() <= fd) {
        return false;
    }
    FdContext* fd_ctx = m_fdContexts[fd];
    lock.unlock();

    if (!fd_ctx->events) {
        return false;
    }
    FdContext::MutexType::Lock lock2(fd_ctx->mutex);


    int op = EPOLL_CTL_DEL;
    epoll_event epevent;
    epevent.events = 0;
    epevent.data.ptr = fd_ctx;
    int rt = epoll_ctl(m_epfd, op, fd, &epevent);
    if(rt) {
        CIVAN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
            << (EpollCtlOp)op << ", " << fd << ", " << (EPOLL_EVENTS)epevent.events << "):"
            << rt << " (" << errno << ") (" << strerror(errno) << ")";
        return false;
    }
    if (fd_ctx->events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }

    if (fd_ctx->events & WRITE) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
    }
    
    return true;
}

IOManager* IOManager::GetThis() {
    return dynamic_cast<IOManager*>(Scheduler::GetThis());
}

void IOManager::tickle() {
    if (!hasIdleThreads()) {
        return;
    }
    int rt = write(m_tickleFds[1], "T", 1);
    CIVAN_ASSERT(rt == 1);
}

bool IOManager::stopping() {
    uint64_t time_out;
    return stopping(time_out);
}

bool IOManager::stopping(uint64_t& time_out) {
    time_out = getNextTimer();

    return time_out == ~0ull
        && m_pendingEventCount == 0
        && Scheduler::stopping();
}

void IOManager::idle() {
    std::unique_ptr<epoll_event> unique_event(new epoll_event[64]());
    epoll_event* events = unique_event.get();
    while (true) {
        int rt = 0;
        uint64_t next_timeout = 0;
        if (stopping(next_timeout)) {
            CIVAN_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
            break;
        }

        do {
            static const int MAX_TIMEOUT = 10000;
            if (next_timeout != ~0ull) {
                
                next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
            } else {
                next_timeout = MAX_TIMEOUT;
            }
            rt = epoll_wait(m_epfd, events, 64, next_timeout);

            if (rt < 0 && errno == EINTR) {
                continue;
            } else {
                break;
            }
        } while(true);
        //CIVAN_LOG_FATAL(g_logger) << Thread::GetName() << " epoll wait wake!!!";
        std::vector<std::function<void()>> cbs;
        listExpiredCb(cbs);
        if (!cbs.empty()) {
            //CIVAN_LOG_INFO(g_logger) << "schedule cbs" ;
            schedule(cbs.begin(), cbs.end());
            
            cbs.clear();
        } 
        //CIVAN_LOG_INFO(g_logger) << "epwait rt=" << rt; 
        for (int i = 0; i < rt; i++) {
            epoll_event& event = events[i];
            
            if (event.data.fd == m_tickleFds[0]) {
                uint8_t dummy[256];
                while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0);
                CIVAN_LOG_INFO(g_logger) << "tickle" << rt; 
                continue;
            }
            FdContext* fd_ctx = (FdContext*)event.data.ptr;

            FdContext::MutexType::Lock lock(fd_ctx->mutex);
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }
            int real_events = NONE;
            if (event.events & EPOLLIN) {
                real_events |= READ;
            }
            if (event.events & EPOLLOUT) {
                real_events |= WRITE;
            }

            if ((fd_ctx->events & real_events) == NONE) {
                continue;
            }

            int left_events = (fd_ctx->events & ~real_events);
            int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            event.events = EPOLLET | left_events;

            int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
            if (rt2) {
                CIVAN_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                    << (EpollCtlOp)op << ", " << event.data.fd << ", " << (EPOLL_EVENTS)event.events << "):"
                    << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                continue;
            }

            if (real_events & READ) {
                fd_ctx->triggerEvent(READ);
                --m_pendingEventCount;
            }
            if (real_events & WRITE) {
                fd_ctx->triggerEvent(WRITE);
                --m_pendingEventCount;
            }
        }

        Fiber::ptr cur = Fiber::GetThis();
        auto raw_ptr = cur.get();
        cur.reset();
        //CIVAN_LOG_INFO(g_logger) << "idle swap out" ;
        raw_ptr->swapOut();





    }
}


void IOManager::onTimerInsertAtFront(){
    tickle();   
}

} //namespace civan

