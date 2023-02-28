#include "tcp_server.h"
#include "config.h"

namespace civan {

static civan::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = 
    civan::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2),
                    "tcp server read timeout");

static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("system");

TcpServer::TcpServer(civan::IOManager* accept_worker
        , int worker_num, int thread_per_worker)
    :m_acceptWorker(accept_worker)
    ,m_recvTimeout(g_tcp_server_read_timeout->getValue())
    ,m_name("civan/1.0.0")
    ,m_isStop(true) {
    addWorkers(worker_num, thread_per_worker);
}

TcpServer::~TcpServer() {
    //sock析构会调用close 还需要嘛？
    for(auto& i : m_socks) {
        i->close();
    }
    m_socks.clear();
}


bool TcpServer::bind(civan::Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails);
}
bool TcpServer::bind(const std::vector<Address::ptr>& addrs
                , std::vector<Address::ptr>& fails) {
    for (auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if (!sock->bind(addr)) {
            CIVAN_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << "errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if (!sock->listen()) {
            CIVAN_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << "errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if (!fails.empty()) {
        m_socks.clear();
        return false;
    }
    for (auto& i : m_socks) {
        CIVAN_LOG_INFO(g_logger) << "server bind success : " << *i;
    }
    return true;
}


void TcpServer::addWorkers(int worker_num, int thread_per_worker) {
    for (int i = 0; i < worker_num; ++i) {
        std::string woker_name = m_name + "_" + std::to_string(m_workers.size() + 1);
        m_workers.push_back(IOManager::ptr(new IOManager(thread_per_worker, false, "woker_name")));
    }
}


void TcpServer::startAccept(Socket::ptr sock) {
    while (!m_isStop) {
        Socket::ptr client = sock->accept();
        if (client) {
            client->setRecvTimeout(m_recvTimeout);
            if (m_workers.size() == 0) {
                m_acceptWorker->schedule(std::bind(&TcpServer::handleClient, 
                                shared_from_this(), client));
            } else {
                int worker_id = client->getSocket() % m_workers.size();
                m_workers[worker_id]->schedule(std::bind(&TcpServer::handleClient, 
                                shared_from_this(), client));
            }
            
        } else {
            CIVAN_LOG_ERROR(g_logger) << "accept errno" << errno
                << "errstr=" << strerror(errno);
        }
    }
}

bool TcpServer::start() {
    if (!m_isStop) {
        return true;
    }
    m_isStop = false;
    for (auto& sock : m_socks) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept, 
                                shared_from_this(), sock));
    }
    return true;

}

void TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this();
    m_acceptWorker->schedule([this, self] () {
        for (auto& sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}



void TcpServer::handleClient(Socket::ptr client) {
    CIVAN_LOG_INFO(g_logger) << "handle client";
}
    

}


