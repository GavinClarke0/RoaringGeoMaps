#ifndef ROARINGGEOMAPS_READERHELPERS_H
#define ROARINGGEOMAPS_READERHELPERS_H

#include <cstdint>
#include <stdexcept>
#include <bit>
#include <cstdint>
#include "io/FileReadBuffer.h"
#include "endian/endian.h"

// Helper function to read a little-endian uint16 from a FileReadBuffer
inline uint8_t readLittleEndianUint8(const FileReadBuffer& buffer, uint64_t offset) {
    if (offset + 1 > buffer.size()) {
        throw std::out_of_range("Offset out of buffer bounds");
    }

    const char* data = buffer.view(offset, 1);
    uint8_t value = *reinterpret_cast<const uint8_t*>(data);

    // Required to support bigEndian without recompilation
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    return value;
}

// Helper function to read a little-endian uint16 from a FileReadBuffer
inline uint16_t readLittleEndianUint16(const FileReadBuffer& buffer, uint64_t offset) {
    if (offset + 2 > buffer.size()) {
        throw std::out_of_range("Offset out of buffer bounds");
    }

    const char* data = buffer.view(offset, 2);
    uint16_t value = *reinterpret_cast<const uint16_t*>(data);

    // Required to support bigEndian without recompilation
    if constexpr(std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    return value;
}

// Helper function to read a little-endian uint32 from a FileReadBuffer
inline uint32_t readLittleEndianUint32(const FileReadBuffer& buffer, uint64_t offset) {
    if (offset + 4 > buffer.size()) {
        throw std::out_of_range("Offset out of buffer bounds");
    }
    const char* data = buffer.view(offset, 4);
    uint32_t value = *reinterpret_cast<const uint32_t*>(data);

    // Required to support bigEndian without recompilation
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    return value;
}

// Helper function to read a little-endian uint64 from a FileReadBuffer
inline uint64_t readLittleEndianUint64(const FileReadBuffer& buffer, uint64_t offset) {
    if (offset + 8 > buffer.size()) {
        throw std::out_of_range("Offset out of buffer bounds");
    }
    const char* data = buffer.view(offset, 8);
    uint64_t value = *reinterpret_cast<const uint64_t*>(data);

    // Required to support bigEndian without recompilation
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    return value;
}

#endif // ROARINGGEOMAPS_READERHELPERS_H
