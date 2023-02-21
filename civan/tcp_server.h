#ifndef __CIVAN_TCP_SERVER_H__
#define __CIVAN_TCP_SERVER_H__

#include <memory>
#include <functional>
#include "iomanager.h"
#include "noncopyable.h"
#include "address.h"
#include "socket.h"
namespace civan{

class TcpServer : public std::enable_shared_from_this<TcpServer>
                    , Noncopyable{
public:
    typedef std::shared_ptr<TcpServer> ptr;
    TcpServer(IOManager* accept_worker = IOManager::GetThis());
    virtual ~TcpServer();
    virtual bool bind(civan::Address::ptr addr);
    //bind and listen
    virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails);
    virtual bool start();
    virtual void stop();

    uint64_t getReadTimeout() const { return m_recvTimeout; }
    std::string getName() const { return m_name; }
    void setReadTimeout(uint64_t v) { m_recvTimeout = v; }
    void setName(const std::string& v) { m_name = v; }
    bool isStop() const { return m_isStop; }

    void addWorker(IOManager::ptr worker);
protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);
private:
    std::vector<Socket::ptr> m_socks;

    std::vector<IOManager::ptr> m_workers;
    IOManager* m_acceptWorker;
    uint64_t m_recvTimeout;
    std::string m_name;
    bool m_isStop;



};





}







#endif