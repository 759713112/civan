#ifndef __CIVAN_STORE_MESSAGE_HANDLER_H__
#define __CIVAN_STORE_MESSAGE_HANDLER_H__

#include <stdint.h>
#include <utime.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

//#include "./buffer/buffer.h"
#include "message.pb.h"
#include "civan/socket_stream.h"

namespace civan {
namespace store {

class Session;
struct ProbobufTransportFormat {
    uint32_t len = 0;
    uint32_t nameLen = 0;
    std::string message_name;
    std::string protobuf_data;

    size_t getSize() const {
        return sizeof(len) + sizeof(nameLen);
    }
};


class MessageHandler {
public:
    typedef std::shared_ptr<MessageHandler> ptr;
    typedef std::shared_ptr<msg_header> messageRef;
    //friend class Messenger;
    friend std::ostream& operator<<(const std::ostream& os, MessageHandler msg_handler);
public:
    MessageHandler();
    virtual ~MessageHandler();

public:
    std::shared_ptr<Session> get_session() const { return m_session; }
    void set_session(std::shared_ptr<Session> s) {
        m_session = s;
    }
    const messageRef &get_header() const { return header; }
    messageRef get_header() { return header; }
    void set_header(messageRef e) { header = e; }

    
    void clear_payload() {
        payload.clear();
    }

    const std::string& get_payload() const { return payload; }
    void set_payload(const std::string& v) {
        payload =v;
    }

    uint32_t get_payload_len() const { return payload.length(); }

    void set_recv_stamp(struct timeval t) { recv_stamp = t; }
    const struct timeval& get_recv_stamp() const { return recv_stamp; }
    void set_dispatch_stamp(struct timeval t) { dispatch_stamp = t; }
    const struct timeval& get_dispatch_stamp() const { return dispatch_stamp; }
    void set_recv_complete_stamp(struct timeval t) { recv_complete_stamp = t; }
    const struct timeval& get_recv_complete_stamp() const { return recv_complete_stamp; }

    // type
    MSG_TYPE get_type() const { return header->type(); }
    void set_type(MSG_TYPE t) { header->set_type(t); }

    uint64_t get_tid() const { return header->tid(); }
    void set_tid(uint64_t t) { header->set_tid(t); }

    uint64_t get_seq() const { return header->seq(); }
    void set_seq(uint64_t s) { header->set_seq(s); }

    unsigned get_priority() const { return header->priority(); }
    void set_priority(uint32_t p) { header->set_priority(p); }

    // source/dest
    // entity_inst_t get_source_inst() const {
    //     return entity_inst_t(get_source(), get_source_addr());
    // }
    //void set_src(const entity_name_t& src) { header->set_src(src); }

    entity_name_t get_source() const {
        return header->src();
    }

public:
    bool createMessage(const std::string& typeName);
    bool encode(std::string& buf);
    bool decode(const std::string& typeName, const std::string& buf);
protected:
    messageRef header;

    //msg_footer footer;
    // buffer::list payload;  // "front" unaligned blob
    // buffer::list middle;   // "middle" unaligned blob
    // buffer::list data;     // data payload (page-alignment will be preserved where possible)
    std::string payload;
    struct timeval recv_stamp;
    struct timeval dispatch_stamp;
    struct timeval throttle_stamp;
    /* time at which message was fully read */
    struct timeval recv_complete_stamp;

    std::shared_ptr<Session> m_session;
};

std::ostream& operator<<(std::ostream& os, MessageHandler msg_handler);

} //namespace store
} //namespace civan

#endif
