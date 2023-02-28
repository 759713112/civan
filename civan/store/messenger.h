#ifndef __CIVAN_STOTE_MESSENGER_H__
#define __CIVAN_STOTE_MESSENGER_H__

#include <memory.h>
#include "civan/tcp_server.h"
#include "Dispatcher.h"
#include "server_context.h"
namespace civan {
namespace store {
class Messenger : public TcpServer {
public:
    typedef std::shared_ptr<Messenger> ptr;
    typedef RWMutex RWMutexType;
public:
    Messenger(ServerContext::ptr sct, bool keepalive = false
                , civan::IOManager* accept_worker = civan::IOManager::GetThis()
                , int worker_num = 0
                , int thread_per_worker = 1);

    void add_dispatcher_head(Dispatcher::ptr dispatcher);
    virtual void setName(const std::string& v);
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepalive;

    std::deque<Dispatcher::ptr> dispatchers;
    Dispatcher::ptr m_default;

    ServerContext::ptr sct;
    int crcflags;
    bool require_authorizer = true;

};

}
}