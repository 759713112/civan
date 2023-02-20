#include "civan/http/http_server.h"
#include "civan/log.h"

static civan::Logger::ptr g_logger = CIVAN_LOG_ROOT();

#define XX(...) #__VA_ARGS__


civan::IOManager::ptr worker;
void run() {
    g_logger->setLevel(civan::LogLevel::INFO);
    //civan::http::HttpServer::ptr server(new civan::http::HttpServer(true, worker.get(), civan::IOManager::GetThis()));
    civan::http::HttpServer::ptr server(new civan::http::HttpServer(true));
    civan::Address::ptr addr = civan::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/civan/xx", [](civan::http::HttpRequest::ptr req
                ,civan::http::HttpResponse::ptr rsp
                ,civan::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/civan/*", [](civan::http::HttpRequest::ptr req
                ,civan::http::HttpResponse::ptr rsp
                ,civan::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

//     sd->addGlobServlet("/civanx/*", [](civan::http::HttpRequest::ptr req
//                 ,civan::http::HttpResponse::ptr rsp
//                 ,civan::http::HttpSession::ptr session) {
//             rsp->setBody(XX(<html>
// <head><title>404 Not Found</title></head>
// <body>
// <center><h1>404 Not Found</h1></center>
// <hr><center>nginx/1.16.0</center>
// </body>
// </html>
// <!-- a padding to disable MSIE and Chrome friendly error page -->
// <!-- a padding to disable MSIE and Chrome friendly error page -->
// <!-- a padding to disable MSIE and Chrome friendly error page -->
// <!-- a padding to disable MSIE and Chrome friendly error page -->
// <!-- a padding to disable MSIE and Chrome friendly error page -->
// <!-- a padding to disable MSIE and Chrome friendly error page -->
// ));
//             return 0;
//     });

    server->start();
}

int main(int argc, char** argv) {
    civan::IOManager iom(1, true, "main");
    worker.reset(new civan::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
