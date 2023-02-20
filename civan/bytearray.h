#ifndef __CIVAN_BYTEARRAY_H__
#define __CIVAN_BYTEARRAY_H__

#include <memory>
#include <string>
#include <stdint.h>
#include <vector>
#include <sys/uio.h>
#include "endian.h"
namespace civan {

class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

    struct Node {
        Node(size_t size);
        Node();
        ~Node();
        char* ptr;
        size_t size;
        Node* next;
        
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    void writeFint8  (int8_t value);
    /**
     * @brief 写入固定长度uint8_t类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFuint8 (uint8_t value);
    /**
     * @brief 写入固定长度int16_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFint16 (int16_t value);
    /**
     * @brief 写入固定长度uint16_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFuint16(uint16_t value);

    /**
     * @brief 写入固定长度int32_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFint32 (int32_t value);

    /**
     * @brief 写入固定长度uint32_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFuint32(uint32_t value);

    /**
     * @brief 写入固定长度int64_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFint64 (int64_t value);

    /**
     * @brief 写入固定长度uint64_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFuint64(uint64_t value);

    /**
     * @brief 写入有符号Varint32类型的数据
     * @post m_position += 实际占用内存(1 ~ 5)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeInt32  (int32_t value);
    /**
     * @brief 写入无符号Varint32类型的数据
     * @post m_position += 实际占用内存(1 ~ 5)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeUint32 (uint32_t value);

    /**
     * @brief 写入有符号Varint64类型的数据
     * @post m_position += 实际占用内存(1 ~ 10)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeInt64  (int64_t value);

    /**
     * @brief 写入无符号Varint64类型的数据
     * @post m_position += 实际占用内存(1 ~ 10)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeUint64 (uint64_t value);

    /**
     * @brief 写入float类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeFloat  (float value);

    /**
     * @brief 写入double类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeDouble (double value);

    /**
     * @brief 写入std::string类型的数据,用uint16_t作为长度类型
     * @post m_position += 2 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeStringF16(const std::string& value);
    //length:int32 data
    void writeStringF32(const std::string& value);
    //length:int64 data
    void writeStringF64(const std::string& value);
    //length:varint data
    void writeStringVint(const std::string& value);
    //data
    void writeStringWithoutLength(const std::string& value);

    //read
    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();

    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    float readFloat();
    double readDouble();

    //length:int16, data
    std::string readStringF16();
    //length:int32, data 
    std::string readStringF32();
    //length:int64, data 
    std::string readStringF64();
    //length:varint, data 
    std::string readStringVint();

    //内部操作
    void clear();

    void write(const void* buf, size_t size);
    void read(void* buf, size_t size);
    void read(void* buf, size_t size, size_t position) const;

    size_t getPosition() const { return m_position; };
    void setPosition(size_t v);

    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const { return m_baseSize; }
    size_t getReadSize() const { return m_size - m_position; }
    size_t getSize() const { return m_position; }

    bool isLittleEndian() const { return m_endian == CIVAN_LITTLE_ENDIAN; }
    void setIsLittleEndian(bool val);

    std::string toString() const;
    std::string toHexString() const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
private:
    void addCapacity(size_t size);
    size_t getCapacity() const { return m_capacity - m_position; }
private:
    size_t m_baseSize;
    size_t m_position;
    size_t m_capacity;
    size_t m_size;
    int8_t m_endian;
    Node* m_root;
    Node* m_cur;

};

}

#endif