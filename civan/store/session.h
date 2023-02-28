#ifndef __CIVAN_STORE_SESSION_H__
#define __CIVAN_STORE_SESSION_H__

#include <memory>
#include <string.h>
#include "civan/socket_stream.h"
#include "message_handler.h"
#include "civan/iomanager.h"
namespace civan {
namespace store {

class Session : public SocketStream, public std::enable_shared_from_this<Session> {
public:
    typedef std::shared_ptr<Session> ptr;

    Session(Socket::ptr sock, IOManager::ptr iom, bool owner = true);

    MessageHandler::ptr recvMessage();
    void sendMessage(MessageHandler::ptr);
    
private:
    void writeFixSize_(const void* data, size_t size);
    IOManager::ptr m_iom;
};


} //namespace store
} //namespace civan


#endif