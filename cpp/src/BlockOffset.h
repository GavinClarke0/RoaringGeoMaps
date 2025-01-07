#include "endian/endian.h"
#include "io/FileReadBuffer.h"
#include "io/FileWriteBuffer.h"
#include "VectorView.h"
#include "WriteHelpers.h"

#ifndef ROARINGGEOMAPS_BLOCKINDEX_H
#define ROARINGGEOMAPS_BLOCKINDEX_H

class BlockOffsetReader {
public:
    BlockOffsetReader(FileReadBuffer &f, uint64_t pos, uint64_t size) : blockOffsets(f, pos, size) {}

    std::pair<uint64_t, uint64_t> BlockPos(uint32_t blockId) {
        // TODO: throw exception if blockIndex is out of bounds;
        if (blockId == 0)
            return {0, blockOffsets[0]};

        return {blockOffsets[blockId - 1], blockOffsets[blockId - 1] - blockOffsets[blockId]};
    }

    uint64_t sizeOf() { return blockOffsets.size() * sizeof(uint64_t); }

private:
    VectorView<uint64_t> blockOffsets;
};

class BlockOffsetWriter {

public:
    BlockOffsetWriter() = default;

    void InsertOffset(uint64_t offset) {
        blockOffsets.emplace_back(offset);
    };

    uint64_t writeToFile(FileWriteBuffer &f) const {
        for (auto &offset: blockOffsets) {
            writeLittleEndianUint64(f, offset);
        }
        return sizeof(uint64_t) * blockOffsets.size();
    }

private:
    std::vector<uint64_t> blockOffsets;
};

#endif //ROARINGGEOMAPS_BLOCKINDEX_H
