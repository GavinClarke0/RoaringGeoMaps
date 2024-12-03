#include "BlockIndexWriter.h"

#include "BlockIndexReader.h"
#include "WriteHelpers.h"  // Assuming this contains the write functions


// Specialization for uint32_t
template <>
uint64_t BlockIndexWriter<uint32_t>::appendToBuffer(FileWriteBuffer& f, uint32_t value) const {
    writeLittleEndianUint32(f, value);
    return 4;
}

// Specialization for uint64_t
template <>
uint64_t BlockIndexWriter<uint64_t>::appendToBuffer(FileWriteBuffer& f, uint64_t value) const {
    writeLittleEndianUint64(f, value);
    return 8;
}
