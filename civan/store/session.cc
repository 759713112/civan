#include "session.h"
#include "civan/config.h"
#include "civan/log.h"
#include <functional>

namespace civan {
namespace store {
static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");

static civan::ConfigVar<uint32_t>::ptr g_store_request_buffer_size =
    civan::Config::Lookup("store.request.buffer_size"
                ,(uint32_t)(4 * 1024), "store request buffer size");

static civan::ConfigVar<uint32_t>::ptr g_store_request_max_body_size =
    civan::Config::Lookup("store.request.max_body_size"
                ,(uint32_t)(64 * 1024 * 1024), "store request max body size");

static civan::ConfigVar<uint32_t>::ptr g_store_response_buffer_size =
    civan::Config::Lookup("store.response.buffer_size"
                ,(uint32_t)(4 * 1024), "store response buffer size");

static civan::ConfigVar<uint32_t>::ptr g_store_response_max_body_size =
    civan::Config::Lookup("store.response.max_body_size"
                ,(uint32_t)(64 * 1024 * 1024), "store response max body size");

static uint32_t s_store_request_buffer_size = 0;
static uint32_t s_store_request_max_body_size = 0;
static uint32_t s_store_response_buffer_size = 0;
static uint32_t s_store_response_max_body_size = 0;

namespace {
struct _RequestSizeIniter {
    _RequestSizeIniter() {
        s_store_request_buffer_size = g_store_request_buffer_size->getValue();
        s_store_request_max_body_size = g_store_request_max_body_size->getValue();
        s_store_response_buffer_size = g_store_response_buffer_size->getValue();
        s_store_response_max_body_size = g_store_response_max_body_size->getValue();

        g_store_request_buffer_size->addListener(
                [](const uint32_t& ov, const uint32_t& nv){
                s_store_request_buffer_size = nv;
        });

        g_store_request_max_body_size->addListener(
                [](const uint32_t& ov, const uint32_t& nv){
                s_store_request_max_body_size = nv;
        });

        g_store_response_buffer_size->addListener(
                [](const uint32_t& ov, const uint32_t& nv){
                s_store_response_buffer_size = nv;
        });

        g_store_response_max_body_size->addListener(
                [](const uint32_t& ov, const uint32_t& nv){
                s_store_response_max_body_size = nv;
        });
    }
};
static _RequestSizeIniter _init;
}


Session::Session(Socket::ptr sock, IOManager::ptr iom, bool owner) 
    : SocketStream(sock, owner) 
    , m_iom(iom) { 
}

MessageHandler::ptr Session::recvMessage() {
    ProbobufTransportFormat pb_format;
    int len = readFixSize(&pb_format, pb_format.getSize());
    if (len <= 0) {
        close();
        return nullptr;
    }
    if (pb_format.len > s_store_request_buffer_size) {
        close();
        return nullptr;
    }
    pb_format.message_name.resize(pb_format.nameLen);
    pb_format.protobuf_data.resize(pb_format.len);

    len = readFixSize(&pb_format.message_name[0], pb_format.nameLen);
    if (len <= 0) {
        close();
        return nullptr;
    }

    len = readFixSize(&pb_format.protobuf_data[0], pb_format.len); 
    if (len <= 0) {
        close();
        return nullptr;
    }

    MessageHandler::ptr msg_handler(new MessageHandler());
    if (!msg_handler->decode(pb_format.message_name
            , pb_format.protobuf_data)) {
        close();
        return nullptr;
    }
    msg_handler->set_session(shared_from_this());
    return msg_handler;

}
void Session::writeFixSize_(const void* data, size_t size) {
    writeFixSize(data, size);
}
void Session::sendMessage(MessageHandler::ptr message) {
    std::stringstream ss;
    ss << *message;
    std::string data = ss.str();
    auto x = std::bind(&Session::writeFixSize_
            , shared_from_this(), (const void*)data.c_str(), (size_t)data.size());
    m_iom->schedule(x);

}


} //namespace store
} //namespace civan