#include "bytearray.h"
#include "log.h"
#include "iostream"
#include <cstring>
#include "endian.h"
#include <iostream>
#include <iomanip>
#include <math.h>


namespace civan {
static civan::Logger::ptr g_logger = CIVAN_LOG_NAME("system");
 
ByteArray::Node::Node(size_t s) 
    : ptr(new char[s])
    , size(s)
    , next(nullptr) {

}

ByteArray::Node::Node() 
    : ptr(nullptr)
    , size(0)
    , next(nullptr) {

}

ByteArray::Node::~Node() {
    if (ptr) {
        delete[] ptr;
    }
} 

ByteArray::ByteArray(size_t base_size) 
    : m_baseSize(base_size)
    , m_position(0)
    , m_capacity(base_size)
    , m_size(0)
    , m_endian(CIVAN_BIG_ENDIAN) 
    , m_root(new Node(base_size))
    , m_cur(m_root) {

}

ByteArray::~ByteArray() {
    Node* temp = m_root;
    while(temp) {
        m_cur = temp;
        temp = temp->next;
        delete m_cur;
    }
}


void ByteArray::setIsLittleEndian(bool val) {
    if (val) {
        m_endian = CIVAN_LITTLE_ENDIAN;
    } else {
        m_endian = CIVAN_BIG_ENDIAN;
    }
}

//write
void ByteArray::writeFint8  (int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8 (uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16 (int16_t value) {
    if(m_endian != CIVAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if(m_endian != CIVAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32 (int32_t value) {
    if(m_endian != CIVAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if(m_endian != CIVAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64 (int64_t value) {
    if(m_endian != CIVAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if(m_endian != CIVAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

//压缩
//将-1 转成 1， 1转成2， 正数都为偶数，负数都为奇数
static uint32_t EncodeZigzag32(const int32_t& v) {
    if (v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if (v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint64_t DecodeZigzag32(const int64_t& v) {
    return (v >> 1) ^ -(v & 1);
}

static uint64_t DecodeZigzag64(const int64_t& v) {
    return (v >> 1) ^ -(v & 1);
}   


void ByteArray::writeInt32(int32_t value) {
    writeUint32(EncodeZigzag32(value));
}

//TLV压缩
//7位放在一个字节中，最高位为1，只有最后一个字节最高位为0
void ByteArray::writeUint32(uint32_t value) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int64_t value) {
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint64_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}


void ByteArray::writeFloat(float value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble(double value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}


void ByteArray::writeStringF16(const std::string& value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}


void ByteArray::writeStringF32(const std::string& value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}


//length:int64 data
void ByteArray::writeStringF64(const std::string& value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

//length:varint data
void ByteArray::writeStringVint(const std::string& value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}
//data
void ByteArray::writeStringWithoutLength(const std::string& value) {
    write(value.c_str(), value.size());
}

//read
int8_t ByteArray::readFint8() {
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8() {
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

int16_t ByteArray::readFint16() {
    int16_t v;
    read(&v, sizeof(v));
    if(m_endian != CIVAN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

uint16_t ByteArray::readFuint16() {
    uint16_t v;
    read(&v, sizeof(v));
    if(m_endian != CIVAN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

int32_t ByteArray::readFint32() {
    int32_t v;
    read(&v, sizeof(v));
    if(m_endian != CIVAN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

uint32_t ByteArray::readFuint32() {
    uint32_t v;
    read(&v, sizeof(v));
    if(m_endian != CIVAN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

int64_t ByteArray::readFint64() {
    int64_t v;
    read(&v, sizeof(v));
    if(m_endian != CIVAN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

uint64_t ByteArray::readFuint64() {
    uint64_t v;
    read(&v, sizeof(v));
    if(m_endian != CIVAN_BYTE_ORDER) {
        v = byteswap(v);
    }
    return v;
}

int32_t ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}

uint32_t ByteArray::readUint32() {
    uint32_t v = 0;
    for (int i = 0; i < 32; i+=7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            v |= ((uint32_t)b) << i;
            break;
        } else {
            v |= ((uint32_t)(b & 0x7f)) << i;
        }
    }
    return v;
    
}

int64_t ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() {
    uint32_t v = 0;
    for (int i = 0; i < 64; i+=7) {
        uint8_t b = readFuint8();
        if (b < 0x80) {
            v |= ((uint32_t)b) << i;
            break;
        } else {
            v |= ((uint32_t)(b & 0x7f)) << i;
        }
    }
    return v;
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

//length:int16, data
std::string ByteArray::readStringF16() {
    uint16_t size = readFuint16();
    std::string result;
    result.resize(size);
    read(&result[0], size);
    return result;
}

//length:int32, data 
std::string ByteArray::readStringF32() {
    uint32_t size = readFuint32();
    std::string result;
    result.resize(size);
    read(&result[0], size);
    return result;
}

//length:int64, data 
std::string ByteArray::readStringF64() {
    uint64_t size = readFuint64();
    std::string result;
    result.resize(size);
    read(&result[0], size);
    return result;
}

//length:varint, data 
std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

//内部操作
void ByteArray::clear() {
    Node* tmp = m_root->next;
    while(tmp) {
        Node* cur = tmp;
        tmp = tmp->next;
        delete cur;
    }
    m_size = 0;
    m_capacity = m_root->size;
    m_position = 0;
    m_cur = m_root;
    m_root->next = NULL;

}

void ByteArray::write(const void* buf, size_t size) {
    if (size == 0) {
        return;
    }
    addCapacity(size);
    size_t pos = m_position % m_baseSize;
    size_t ncap = m_cur->size - pos;

    size_t src_pos = 0;
    size_t s = size;
    while (s > 0) {
        if (s <= ncap) {
            memcpy(m_cur->ptr + pos, (const char*)buf + src_pos, s);
            s = 0;

        } else {
            memcpy(m_cur->ptr + pos, (const char*)buf + src_pos, ncap);
            m_cur = m_cur->next;
            pos = 0;
            s -= ncap;
            src_pos += ncap;
            ncap = m_cur->size;
            
        }
    }

    m_position += size;
    m_size = std::max(m_position, m_size);
}

void ByteArray::read(void* buf, size_t size) {
    if(size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, size);
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
}

void ByteArray::read(void* buf, size_t size, size_t position) const {
    if(size > (m_size - position)) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    Node* cur = m_cur;
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos, cur->ptr + npos, size);
            if(cur->size == (npos + size)) {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, cur->ptr + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
}

void ByteArray::setPosition(size_t v) {
    if (m_capacity < v) {
        throw std::out_of_range("set position out of range");
    } 
    m_position = v;
    if (m_position > m_size) {
        m_size = m_position;
    }
    m_cur = m_root;
    while (v > m_cur->size) {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    if(v == m_cur->size) {
        m_cur = m_cur->next;
    }
    
}



bool ByteArray::writeToFile(const std::string& name) const {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs) {
        CIVAN_LOG_ERROR(g_logger) << "writeToFile name=" << name
            << " error , errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node* cur = m_cur;
    
    while (read_size > 0) {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string& name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) {
        CIVAN_LOG_ERROR(g_logger) << "readFromFile name=" << name
            << " error, errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }

    std::shared_ptr<char> buff(new char[m_baseSize], [](char* ptr){
        delete[] ptr;
    });
    while (!ifs.eof()) {
        ifs.read(buff.get(), m_baseSize);
        write(buff.get(), ifs.gcount());
    }
    return true;
}


void ByteArray::addCapacity(size_t size) {
    if (size == 0) {
        return;
    }
    size_t old_cap = getCapacity();
    if(old_cap >= size) {
        return;
    }
    size = size - old_cap;
    size_t count = ceil(1.0 * size / m_baseSize);
    Node* cur = m_cur;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    Node* first = NULL;
    for (size_t i = 0; i < count; i++) {
        Node* newNode = new Node(m_baseSize);
        if(first == NULL) {
            first = newNode;
        }
        cur->next = newNode;
        m_capacity += m_baseSize;
        cur = cur->next;
    }
    if (old_cap == 0) {
        m_cur = first;
    }
}

std::string ByteArray::toString() const {
    std::string res;
    size_t read_size = getReadSize();
    res.resize(read_size);
    if (res.empty()) {
        return res;
    }
    read(&res[0], read_size, m_position);
    return res;
}

std::string ByteArray::toHexString() const {
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0; i < str.size(); ++i) {
        if(i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }

    return ss.str();
}


uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
    len = std::min(getReadSize(), len);
    if (len == 0) {
        return 0;
    }
    struct iovec iov;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    Node* cur = m_cur;

    uint64_t len_ = len;
    while (len_ > 0) {
        if (len_ <= ncap) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len_;
            len_ = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;

            len_ -= ncap;
            npos = 0;
            ncap = m_baseSize;
            cur = cur->next;
        }
        buffers.push_back(iov);

    }

    return len;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const {
    len = std::min(m_size - position, len);
    if(len == 0) {
        return 0;
    }

    uint64_t size = len;

    size_t npos = position % m_baseSize;
    size_t count = position / m_baseSize;
    Node* cur = m_root;
    while(count > 0) {
        cur = cur->next;
        --count;
    }

    size_t ncap = cur->size - npos;
    struct iovec iov;
    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if(len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    struct Node* cur = m_cur;
    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

} //namespace