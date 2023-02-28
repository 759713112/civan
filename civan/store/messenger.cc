#include "messenger.h"
#include "civan/log.h"

namespace civan {
namespace store {

static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("system");

Messenger(ServerContext::ptr s, bool keepalive
                , civan::IOManager* accept_worker = civan::IOManager::GetThis()
                , int worker_num
                , int thread_per_worker)
        : TcpServer(accept_worker, worker_num, thread_per_worker)
        , sct(s),
        , m_isKeepalive(keepalive) {
    m_default.reset(new NotFoundDispatcher());
}

void Messenger::setName(const std::string& v) {
    TcpServer::setName(v);
    //m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void Messenger::handleClient(Socket::ptr client) {
    CIVAN_LOG_DEBUG(g_logger) << "handleClient " << *client;
    Session::ptr session(new Session(client));
    do {
        if (!session->isConnect()) break;
        auto req = session->recvRequest();
        if(!req) {
            CIVAN_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
            break;
        }
        
        Response::ptr rsp(new Response(req->getVersion()
                            , req->isClose() || !m_isKeepalive));

        for (auto& dispatcher : dispatchers) {
            if (dispatcher->handle(req, rsp, session)) {
                break;
            }
        }
        m_dispatch->handle(req, rsp, session);
        
        // if(!m_isKeepalive || req->isClose()) {
        //     break;
        // }
    } while(true);
    //session->close();
}



}
}