#include "CellIdColumnWriter.h"
#include "WriteHelpers.h"
#include "Block.h"

CellIdColumnWriter::CellIdColumnWriter(uint64_t blockSize) : blockSize(blockSize), currentWriteBlock(blockSize) {}

void CellIdColumnWriter::addValue(uint64_t value) {
    bool blockComplete = !currentWriteBlock.insertValue(value);
    if (blockComplete) {
        blocks.push_back(std::move(currentWriteBlock));
        currentWriteBlock = Uint64BlockWriter(blockSize);
        currentWriteBlock.insertValue(value);
    }
}

uint64_t CellIdColumnWriter::writeToFile(FileWriteBuffer &f) {
    // 0. Write current uncompleted block to list of blocks.
    blocks.push_back(std::move(currentWriteBlock));
    // 1. Reserve space for block index and block Index by seeking to write position beyond position for these 2 values
    uint64_t blockIndexAndOffsetSize = blocks.size() * 2 * sizeof(uint64_t);
    f.seek(blockIndexAndOffsetSize);
    // 2. write each block;
    BlockOffsetWriter blockOffsets;
    BlockIndexWriter<uint64_t> blockIndex;
    uint64_t blockOffset = 0;
    for (auto block: blocks) {
        auto blockInfo = block.WriteBlock(f);
        blockOffset += blockInfo.first;
        blockOffsets.InsertOffset(blockOffset);
        blockIndex.addValue(blockInfo.second);
    }
    // 3. seek back to the start of buffer and write block index and block offset
    f.seek(-1 * (blockOffset + blockIndexAndOffsetSize));
    blockIndex.writeToFile(f);
    blockOffsets.writeToFile(f);
    // 4. seek back to the next write position which is the end of the block data.
    f.seek(blockOffset + blockIndexAndOffsetSize);
    // 5. return overall size of all bytes written.
    return blockIndexAndOffsetSize + blockOffset;
}
