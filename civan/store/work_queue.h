#ifndef __CIVAN_STORE_WORKQUEUE_H__
#define __CIVAN_STORE_WORKQUEUE_H__

#include <utime.h>
#include <vector>
#include <string>
#include "civan/mutex.h"
#include "civan/thread.h"

namespace civan {
namespace store {
class ThreadPool {
protected:
    std::string name;
    std::string thread_name;
    std::string lockname;
    Mutex _lock;
    Semaphore _cond;
    bool _stop;
    int _pause;
    int _draining;
    Semaphore _wait_cond;
    unsigned _num_threads;
    std::string _thread_num_option;
    std::vector<WorkQueue_*> work_queues;
    int next_work_queue = 0;
public:
    class TPHandle {
        friend class ThreadPool;
    };

protected:
/// Basic interface to a work queue used by the worker threads.
    struct WorkQueue_ {
        std::string name;
        struct timeval timeout_interval;
        struct timeval suicide_interval;
        WorkQueue_(std::string n, struct timeval ti, struct timeval sti)
            : name(std::move(n)), timeout_interval(ti), suicide_interval(sti)
        { }
        virtual ~WorkQueue_() {}
        /// Remove all work items from the queue.
        virtual void _clear() = 0;
        /// Check whether there is anything to do.
        virtual bool _empty() = 0;
        /// Get the next work item to process.
        virtual void *_void_dequeue() = 0;
        /** @brief Process the work item.
         * This function will be called several times in parallel
         * and must therefore be thread-safe. */
        virtual void _void_process(void *item, TPHandle &handle) = 0;
        /** @brief Synchronously finish processing a work item.
         * This function is called after _void_process with the global thread pool lock held,
         * so at most one copy will execute simultaneously for a given thread pool.
         * It can be used for non-thread-safe finalization. */
        virtual void _void_process_finish(void *) = 0;
        void set_timeout(time_t ti){
            timeout_interval = ceph::make_timespan(ti);
        }
        void set_suicide_timeout(time_t sti){
            suicide_interval = ceph::make_timespan(sti);
        }
        };



public:
  /** @brief Templated by-value work queue.
   * Skeleton implementation of a queue that processes items submitted by value.
   * This is useful if the items are single primitive values or very small objects
   * (a few bytes). The queue will automatically add itself to the thread pool on
   * construction and remove itself on destruction. */
  template<typename T, typename U = T>
  class WorkQueueVal : public WorkQueue_ {
    ceph::mutex _lock = ceph::make_mutex("WorkQueueVal::_lock");
    ThreadPool *pool;
    std::list<U> to_process;
    std::list<U> to_finish;
    virtual void _enqueue(T) = 0;
    virtual void _enqueue_front(T) = 0;
    bool _empty() override = 0;
    virtual U _dequeue() = 0;
    virtual void _process_finish(U) {}

    void *_void_dequeue() override {
      {
	std::lock_guard l(_lock);
	if (_empty())
	  return 0;
	U u = _dequeue();
	to_process.push_back(u);
      }
      return ((void*)1); // Not used
    }
    void _void_process(void *, TPHandle &handle) override {
      _lock.lock();
      ceph_assert(!to_process.empty());
      U u = to_process.front();
      to_process.pop_front();
      _lock.unlock();

      _process(u, handle);

      _lock.lock();
      to_finish.push_back(u);
      _lock.unlock();
    }

    void _void_process_finish(void *) override {
      _lock.lock();
      ceph_assert(!to_finish.empty());
      U u = to_finish.front();
      to_finish.pop_front();
      _lock.unlock();

      _process_finish(u);
    }

    void _clear() override {}

  public:
    WorkQueueVal(std::string n,
		 struct timeval ti,
		 struct timeval sti,
		 ThreadPool *p)
      : WorkQueue_(std::move(n), ti, sti), pool(p) {
      pool->add_work_queue(this);
    }
    ~WorkQueueVal() override {
      pool->remove_work_queue(this);
    }
    void queue(T item) {
      std::lock_guard l(pool->_lock);
      _enqueue(item);
      pool->_cond.notify_one();
    }
    void queue_front(T item) {
      std::lock_guard l(pool->_lock);
      _enqueue_front(item);
      pool->_cond.notify_one();
    }
    void drain() {
      pool->drain(this);
    }
  protected:
    void lock() {
      pool->lock();
    }
    void unlock() {
      pool->unlock();
    }
    virtual void _process(U u, TPHandle &) = 0;
  };

    /** @brief Template by-pointer work queue.
     * Skeleton implementation of a queue that processes items of a given type submitted as pointers.
     * This is useful when the work item are large or include dynamically allocated memory. The queue
     * will automatically add itself to the thread pool on construction and remove itself on
     * destruction. */
    template<class T>
    class WorkQueue : public WorkQueue_ {
        ThreadPool *pool;

        /// Add a work item to the queue.
        virtual bool _enqueue(T *) = 0;
        /// Dequeue a previously submitted work item.
        virtual void _dequeue(T *) = 0;
        /// Dequeue a work item and return the original submitted pointer.
        virtual T *_dequeue() = 0;
        virtual void _process_finish(T *) {}

        // implementation of virtual methods from WorkQueue_
        void *_void_dequeue() override {
            return (void *)_dequeue();
        }
        void _void_process(void *p, TPHandle &handle) override {
            _process(static_cast<T *>(p), handle);
        }
        void _void_process_finish(void *p) override {
            _process_finish(static_cast<T *>(p));
        }

    protected:
        /// Process a work item. Called from the worker threads.
        virtual void _process(T *t, TPHandle &) = 0;

    public:
        WorkQueue(std::string n,
                struct timeval ti, struct timeval sti,
                ThreadPool* p)
            : WorkQueue_(std::move(n), ti, sti), pool(p) {
            pool->add_work_queue(this);
        }
        ~WorkQueue() override {
            pool->remove_work_queue(this);
        }

        bool queue(T *item) {
            pool->_lock.lock();
            bool r = _enqueue(item);
            pool->_cond.notify_one();
            pool->_lock.unlock();
            return r;
        }
        void dequeue(T *item) {
            pool->_lock.lock();
            _dequeue(item);
            pool->_lock.unlock();
        }
        void clear() {
            pool->_lock.lock();
            _clear();
            pool->_lock.unlock();
        }

        void lock() {
            pool->lock();
        }
        void unlock() {
            pool->unlock();
        }
        /// wake up the thread pool (without lock held)
        void wake() {
            pool->wake();
        }
        /// wake up the thread pool (with lock already held)
        void _wake() {
            pool->_wake();
        }
        void _wait() {
            pool->_wait();
        }
        void drain() {
            pool->drain(this);
        }

    };

    // threads
    struct WorkThread : public Thread {
        ThreadPool *pool;
        // cppcheck-suppress noExplicitConstructor
        WorkThread(std::function<void()> cb, const std::string& name, ThreadPool* pool) 
                : Thread(cb, name)
                , pool(p) {}
    };
      int processing;

    void start_threads();
    void join_old_threads();
    virtual void worker(WorkThread *wt);
};


}
}



#endif