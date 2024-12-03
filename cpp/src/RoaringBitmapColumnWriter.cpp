#include "RoaringBitmapColumnWriter.h"
#include "WriteHelpers.h"
#include "Block.h"
#include "BlockOffset.h"

RoaringBitmapColumnWriter::RoaringBitmapColumnWriter(uint64_t blockSize): blockSize(blockSize), currentWriteBlock(blockSize) {}

void RoaringBitmapColumnWriter::addBitmap(roaring::Roaring* bitmap) {
    bool blockComplete = !currentWriteBlock.insertValue(bitmap, bitmap->getSizeInBytes(false));
    if (blockComplete) {
        blocks.push_back(std::move(currentWriteBlock));
        currentWriteBlock = RoaringBitMapBlockWriter(blockSize);
        currentWriteBlock.insertValue(bitmap, bitmap->getSizeInBytes(false));
    }
}

uint64_t RoaringBitmapColumnWriter::writeToFile(FileWriteBuffer& f) {
    // 0. push current block which is not yet in blocks vector;
    blocks.push_back(std::move(currentWriteBlock));
    // 1. Reserve space for block index and block Index by seeking to write position beyond position for these 2 values
    uint64_t blockOffsetSize = blocks.size() * sizeof(uint64_t);
    f.seek(blockOffsetSize);
    // 2. write each block;
    BlockOffsetWriter blockOffsets;
    uint64_t blockOffset = 0;
    for (auto block: blocks) {
        auto blockInfo = block.WriteBlock(f);
        blockOffset += blockInfo.first;
        blockOffsets.InsertOffset(blockOffset);
    }
    // 3. seek back to the start of buffer and write block index and block offset
    f.seek(-1 * (blockOffset + blockOffsetSize));
    blockOffsets.writeToFile(f);
    // 4. seek back to the next write position which is the end of the block data.
    f.seek(blockOffset + blockOffsetSize);
    // 5. return overall size of all bytes written.
    return blockOffsetSize + blockOffset;
}


