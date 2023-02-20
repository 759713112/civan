#ifndef __CIVAN_HTTP_SERVER_H__
#define __CIVAN_HTTP_SERVER_H__

#include "civan/tcp_server.h"
#include "servlet.h"



namespace civan {
namespace http {

class HttpServer : public TcpServer {
public:
    HttpServer(bool keepalive = false
                ,civan::IOManager* worker = civan::IOManager::GetThis()
                //,civan::IOManager* io_worker = civan::IOManager::GetThis()
                , civan::IOManager* accept_worker = civan::IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}
    virtual void setName(const std::string& v);
protected:
    virtual void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;

};





} //namespace http
} // namespace civan





#endif