#ifndef __CIVAN_STORE_DISPATCHER_H__
#define __CIVAN_STORE_DISPATCHER_H__

#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>

#include "http.h"
#include "session.h"
#include "civan/mutex.h"
namespace civan {
namespace store {

class Dispatcher {
public:
    typedef std::shared_ptr<Dispatcher> ptr;
    Dispatcher(const std::string& name)
        :m_name(name) {}
    virtual ~Dispatcher() {}
    virtual int32_t ms_dispatch(Request::ptr request
                    , Response::ptr response
                    , Session::ptr session) = 0;

    const std::string& getName() const { return m_name; }
protected:
    std::string m_name;
};

class FunctionDispatcher : public Dispatcher {
public:
    typedef std::shared_ptr<FunctionDispatcher> ptr;
    typedef std::function<int32_t (Request::ptr request
                    , Response::ptr response
                    , Session::ptr session)> callback;
    FunctionDispatcher(callback cb);
    virtual int32_t ms_dispatch(Request::ptr request
                    , Response::ptr response
                    , Session::ptr session) override;
private:
    callback m_cb;
};

class NotFoundDispatcher : public Dispatcher {
public:
    typedef std::shared_ptr<NotFoundDispatcher> ptr;
    NotFoundDispatcher();
    virtual int32_t ms_dispatch(Request::ptr request
                    , Response::ptr response
                    , Session::ptr session) override;
private:
    std::string m_content;
};

}  //namespace store
} //namespace civan

#endif