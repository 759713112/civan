#ifndef __CIVAN_HTTP_SESSION_H__
#define __CIVAN_HTTP_SESSION_H__

#include <memory>

#include "civan/socket_stream.h"
#include "http.h"
#include "http_parser.h"



namespace civan {
namespace http{

class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);

    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr);

private:

};


} //namespace http
} //namespace civan


#endif