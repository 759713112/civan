#ifndef __CIVAN_ENDIAN_H__
#define __CIVAN_ENDIAN_H__

#define CIVAN_LITTLE_ENDIAN 1
#define CIVAN_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace civan {

template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#define CIVAN_BYTE_ORDER CIVAN_BIG_ENDIAN
#else
#define CIVAN_BYTE_ORDER CIVAN_LITTLE_ENDIAN
#endif

#if CIVAN_BYTE_ORDER == CIVAN_BIG_ENDIAN
template<typename T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template<typename T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else

template<typename T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template<typename T>
T byteswapOnBigEndian(T t) {
    return t;
}

#endif


}

#endif