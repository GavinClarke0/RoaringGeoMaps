#ifndef ROARINGGEOMAPS_CELLIDCOLUMNREADER_H
#define ROARINGGEOMAPS_CELLIDCOLUMNREADER_H

#include "io/FileReadBuffer.h"
#include "BlockIndexReader.h"
#include "BlockOffset.h"
#include "Block.h"
#include "ReaderHelpers.h"
#include <set>
#include <cmath>

class Uint64BlockReader : public FixedBlockReader<uint64_t> {
public:
    explicit Uint64BlockReader(FileReadBuffer& f, uint64_t position, uint64_t size, uint32_t entries):
    FixedBlockReader<uint64_t>( f,  position,  size,  entries) {};
};

class CellIdColumnReader {
public:
    CellIdColumnReader(FileReadBuffer& f, uint64_t startPos, uint64_t size, uint32_t entries, uint16_t blockSize);
    Uint64BlockReader ReadBlock(uint32_t blockIndex);
    std::vector<uint32_t> FilterIndexBlock(uint64_t blockId, std::vector<uint64_t> &values);
    BlockIndexReader<uint64_t, std::set<uint64_t>::iterator>& BlockIndex();
private:
    FileReadBuffer& f;
    BlockIndexReader<uint64_t, std::set<uint64_t>::iterator> blockIndex;
    BlockOffsetReader blockOffset;
    uint64_t startPos;
    uint64_t size;
    uint32_t entries;
    uint64_t blockSize;

    inline uint64_t dataPos() {
        return startPos + blockIndex.sizeOf() + blockOffset.sizeOf();
    }
};
#endif //ROARINGGEOMAPS_CELLIDCOLUMNREADER_H
