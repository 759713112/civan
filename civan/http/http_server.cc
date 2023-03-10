#include "http_server.h"
#include "civan/log.h"
//#include "civan/http/servlets/config_servlet.h"
//#include "civan/http/servlets/status_servlet.h"

namespace civan {
namespace http {

static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");

HttpServer::HttpServer(bool keepalive
               //,civan::IOManager* worker
               //,civan::IOManager* io_worker
               , civan::IOManager* accept_worker
               , int worker_num
               , int thread_per_worker)
    : TcpServer(accept_worker, worker_num, thread_per_worker)
    , m_isKeepalive(keepalive) {
    m_dispatch.reset(new ServletDispatch);

    //m_type = "http";
    // m_dispatch->addServlet("/_/status", Servlet::ptr(new StatusServlet));
    // m_dispatch->addServlet("/_/config", Servlet::ptr(new ConfigServlet));
}

void HttpServer::setName(const std::string& v) {
    TcpServer::setName(v);
    //m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
}

void HttpServer::handleClient(Socket::ptr client) {
    CIVAN_LOG_DEBUG(g_logger) << "handleClient " << *client;
    HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if(!req) {
            CIVAN_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                << errno << " errstr=" << strerror(errno)
                << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
            break;
        }
        
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion()
                            , req->isClose() || !m_isKeepalive));
        // rsp->setBody("hello civan");
        // session->sendResponse(rsp);
        //rsp->setHeader("Server", getName());
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);
        
        if(!m_isKeepalive || req->isClose()) {
            break;
        }
    } while(true);
    session->close();
}

}
}
