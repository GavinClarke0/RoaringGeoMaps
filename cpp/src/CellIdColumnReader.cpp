#include "CellIdColumnReader.h"
#include "Block.h"
#include "ReaderHelpers.h"
#include <iterator>
#include <span>

CellIdColumnReader::CellIdColumnReader(FileReadBuffer &f, uint64_t startPos, uint64_t size, uint32_t entries,
                                       uint16_t blockSize) :
        f(f),
        startPos(startPos),
        size(size),
        entries(entries),
        blockSize(blockSize),
        blockIndex(f, startPos, determineBlocks(blockSize, entries)),
        blockOffset(f, startPos + blockIndex.sizeOf(), determineBlocks(blockSize, entries)) {}

Uint64BlockReader CellIdColumnReader::ReadBlock(uint32_t block) {
    auto [start, sizeOf] = blockOffset.BlockPos(block);
    uint32_t blockEntries =
            (block + 1) * blockSize < entries ? blockSize : entries % blockSize; // TODO this is likely wrong
    return Uint64BlockReader(f, dataPos() + start, sizeOf, blockEntries);
};

std::vector<uint32_t> CellIdColumnReader::FilterIndexBlock(uint64_t blockId, std::vector<uint64_t> &values) {

    // Convert block to Vector view. In the future we may have block level compression which would allow to decompress the
    // block at this stage for final reading.

    // 1. read block offset.
    auto blockOffset = readLittleEndianUint64(f, startPos + (blockId * sizeof(uint64_t)));
    // 2. As in current version block is uncompressed, create vector view of values of the block;
    auto blockValues = VectorView<uint64_t>(f, dataPos(), size);

    // 3. Preform a linear search over the block values to find the indexes of values which match the value filters. Since
    // the values and blockValues are sorted we can use the two pointer method to quickly scan the block. This may
    // be a place where we can use SIMD instructions to vectorize the code.
    std::vector<uint32_t> valuesIndexes;
    uint32_t index = 0;
    uint32_t filterValueIndex = 0;
    for (auto value: blockValues) {
        if (value == values[filterValueIndex]) {
            valuesIndexes.emplace_back(index);
            filterValueIndex++;
        }
        index++;
    }
    return valuesIndexes;
}

// TODO figure out if this is how I want to handle the API or if this should be separate from the column.,
S2BlockIndexReader &CellIdColumnReader::BlockIndex() {
    return blockIndex;
}




