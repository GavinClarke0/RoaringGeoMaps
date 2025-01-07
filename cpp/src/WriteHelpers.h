#ifndef ROARINGGEOMAPS_FUNCTIONS_H
#define ROARINGGEOMAPS_FUNCTIONS_H

#include <cstdint>
#include <vector>
#include "io/FileWriteBuffer.h"
#include "endian/endian.h"

// Helper function to write a little-endian uint8 to a FileWriteBuffer
inline void writeLittleEndianUint8(FileWriteBuffer &buffer, uint8_t value) {
    buffer.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

// Helper function to write a little-endian uint16 to a FileWriteBuffer
inline void writeLittleEndianUint16(FileWriteBuffer &buffer, uint16_t value) {
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    buffer.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

// Helper function to write a little-endian uint32 to a FileWriteBuffer
inline void writeLittleEndianUint32(FileWriteBuffer &buffer, uint32_t value) {
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    buffer.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

// Helper function to write a little-endian uint64 to a FileWriteBuffer
inline void writeLittleEndianUint64(FileWriteBuffer &buffer, uint64_t value) {
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    buffer.write(reinterpret_cast<const char *>(&value), sizeof(value));
}


#endif // ROARINGGEOMAPS_FUNCTIONS_H
