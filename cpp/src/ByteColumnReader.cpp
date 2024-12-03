#include "ByteColumnReader.h"

ByteColumnReader::ByteColumnReader(FileReadBuffer &f, uint64_t startPos, uint32_t size, uint32_t entries, uint16_t blockSize):
f(f),
startPos(startPos),
size(size),
entries(entries),
blockSize(blockSize),
blockOffset(f, startPos, determineBlocks(blockSize, entries))
{}

BytesBlockReader ByteColumnReader::ReadBlock(uint32_t block) {
    auto [start, sizeOf] = blockOffset.BlockPos(block);
    uint32_t blockEntries = (block + 1) * blockSize <= entries ? blockSize : entries % blockSize; // TODO this is likely wrong
    return BytesBlockReader(f, dataPos() + start, sizeOf, blockEntries);
};