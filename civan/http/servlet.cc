
#include "servlet.h"
#include <fnmatch.h>
namespace civan {
namespace http {

FunctionServlet::FunctionServlet(callback cb)
    :Servlet("FunctionServlet")
    ,m_cb(cb) {

}
int32_t FunctionServlet::handle(civan::http::HttpRequest::ptr request
                , civan::http::HttpResponse::ptr response
                , civan::http::HttpSession::ptr session) {
    return m_cb(request, response, session);
}




ServletDispatch::ServletDispatch() 
    :Servlet("ServletDispatch") {
    m_default.reset(new NotFoundServlet());
}
int32_t ServletDispatch::handle(civan::http::HttpRequest::ptr request
                , civan::http::HttpResponse::ptr response
                , civan::http::HttpSession::ptr session) {
    auto slt = getMatchedServlet(request->getPath());
    if (slt) {
        slt->handle(request, response, session);
    }
    return 0;
}
void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[uri] = slt;
}

void ServletDispatch::addServlet(const std::string& uri
                            , FunctionServlet::callback cb){
    RWMutexType::WriteLock lock(m_mutex);
    m_datas[uri].reset(new FunctionServlet(cb));
}

void ServletDispatch::addGlobServlet(const std::string& uri
                                    , Servlet::ptr slt) {
    RWMutexType::WriteLock lock(m_mutex);
    for (auto it = m_globs.begin();
            it != m_globs.end(); ++it) {
        if (it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, slt));

}
void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::callback cb) {
    return addGlobServlet(uri, FunctionServlet::ptr(new FunctionServlet(cb)));
}

void ServletDispatch::delServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    m_datas.erase(uri);
}
void ServletDispatch::delGlobServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    for (auto it = m_globs.begin();
            it != m_globs.end(); ++it) {
        if (it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
}


Servlet::ptr ServletDispatch::getServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string& uri) {
    for (auto it = m_globs.begin();
            it != m_globs.end(); ++it) {
        if (it->first == uri) {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& uri) {
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_datas.find(uri);
    if (it != m_datas.end()) return it->second;
    for (auto it = m_globs.begin();
            it != m_globs.end(); ++it) {
        //fnmatch: match filename or pathname(shell格式 return 0匹配)
        if (!fnmatch(it->first.c_str(), uri.c_str(), 0)) {
            return it->second;
        }
    }
    return m_default;
}

NotFoundServlet::NotFoundServlet()
    : Servlet("NotFoundServlet"){
    // m_content = "<html><head><title>404 Not Found"
    //     "</title></head><body><center><h1>404 Not Found</h1></center>"
    //     "<hr><center>" "</center></body></html>"; //name +
m_content = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Welcome to nginx!</title>"
"<style>"
    "body {"
        "width: 35em;"
        "margin: 0 auto;"
        "font-family: Tahoma, Verdddddddddddana, Arial, sans-serif;"
"    }"
"</style>"
"</head>"
"<body>"
"<h1>Welcomeddddddddddddddddddddddddddddddddddddddddd to nginx!</h1>"
"<p>If you see this page, the nginxdddddddddddd web server is successfully installed and"
"working. Further configuration is reqfdufffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffired.</p>"

"<p>For online documentation and support please refer to"
"<a href="
"Commercial support is available at"
"<a href="

"<p><em>Thank you for using nginx.</em></p>"
"</body>"
"</html>";
}
int32_t NotFoundServlet::handle(civan::http::HttpRequest::ptr request
                , civan::http::HttpResponse::ptr response
                , civan::http::HttpSession::ptr session) {
    response->setStatus(civan::http::HttpStatus::OK);
    response->setHeader("Server", "civan/1.0.0");
    response->setHeader("Content-Type", "text/html");
    response->setBody(m_content);
    return 0;
}


}

}