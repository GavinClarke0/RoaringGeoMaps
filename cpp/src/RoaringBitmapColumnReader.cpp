#include "RoaringBitmapColumnReader.h"

RoaringBitmapColumnReader::RoaringBitmapColumnReader(FileReadBuffer &f, uint64_t startPos, uint64_t size,
                                                     uint32_t entries, uint16_t blockSize) :
        f(f),
        startPos(startPos),
        size(size),
        entries(entries),
        blockSize(blockSize),
        blockOffset(f, startPos, determineBlocks(blockSize, entries)) {}

RoaringBitmapBlockReader RoaringBitmapColumnReader::ReadBlock(uint32_t block) {
    auto [start, sizeOf] = blockOffset.BlockPos(block);
    uint32_t blockEntries = (block + 1) * blockSize <= entries ? blockSize : entries % blockSize;
    return RoaringBitmapBlockReader(f, dataPos() + start, sizeOf, blockEntries);
};

