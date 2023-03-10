#include "thread.h"
#include "util.h"
#include <errno.h>
#include "log.h"
namespace civan {
static auto g_logger = CIVAN_LOG_NAME("root");

Thread::Thread(std::function<void()> cb, const std::string& name) : m_cb(cb) {
    if (name.empty()) {
        m_name = "UNKNOW";
    } else {
        m_name = name;
    }
    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if (rt) {
        CIVAN_LOG_ERROR(g_logger) << "pthread create failed, rt=" 
                                    << rt << " name= " << name;
        throw std::logic_error("pthread create error");
    }
    //需要跑起来的run去设置该线程对象成员
    //防止线程还没跑起来 其他线程获取线程信息时错误
    m_semaphore.wait();

}

Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            CIVAN_LOG_ERROR(g_logger) << "pthread join failed, rt=" 
                                    << rt << " name= " << m_name;
            throw std::logic_error("pthread joinerror");
        }
        m_thread = 0; //join了 析构时候就不要detach了
    }
}



static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";
Thread* Thread::GetThis() {
    return t_thread;
}

const std::string& Thread::GetName() {
    return t_thread_name;
}
    

void Thread::SetName(const std::string& name) {
    if (t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

void* Thread::run(void *arg) {
    Thread* thread = (Thread*)arg;
    t_thread = thread;
    t_thread_name = thread->getName();
    thread->m_id = civan::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    thread->m_semaphore.notify();
    cb();
    return 0;
}

} //namespace civan