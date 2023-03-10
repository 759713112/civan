#include "message_handler.h"
#include "civan/log.h"

namespace civan {
namespace store{
    
static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("root");

MessageHandler::MessageHandler() {

}

MessageHandler::~MessageHandler() {

}

bool MessageHandler::createMessage(const std::string& typeName) {
    const google::protobuf::Descriptor* descriptor = 
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor) {
        const google::protobuf::Message* prototype =
        google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype) {
            header.reset(dynamic_cast<msg_header*>(prototype->New()));
            return true;
        }
    }
    return false;
}



bool MessageHandler::encode(std::string& buf) {
    //bool SerializeToString(string* output) const;
    //bool SerializeToOstream(ostream* output) const;
    return true;
}

bool MessageHandler::decode(const std::string& typeName
                        , const std::string& buf) {
    if (!createMessage(typeName)) {
        return false;
    }
	if (!header->ParseFromString(buf)) {
        return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, MessageHandler msg_handler) {
    return os;
}


} //namespace store
} //namespace civan