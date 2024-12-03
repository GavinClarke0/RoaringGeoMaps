#ifndef ROARINGGEOMAPS_ROARINGBITMAPCOLUMNREADER_H
#define ROARINGGEOMAPS_ROARINGBITMAPCOLUMNREADER_H

#include "BlockOffset.h"
#include "Block.h"
#include "roaring.hh"

class RoaringBitmapBlockReader : public BlockReader<roaring::Roaring> {
public:
    explicit RoaringBitmapBlockReader(FileReadBuffer& f, uint64_t position, uint64_t size, uint32_t entries):
            BlockReader<roaring::Roaring>(f,  position,  size,  entries) {};

    roaring::Roaring readValue(FileReadBuffer& f, uint64_t position, uint64_t size) override {
        return roaring::Roaring::read(f.view(position, size), false);
    };
};

class RoaringBitmapColumnReader {
public:
    RoaringBitmapColumnReader(FileReadBuffer& f, uint64_t startPos, uint64_t size, uint32_t entries, uint16_t blockSize);
    RoaringBitmapBlockReader ReadBlock(uint32_t blockIndex);

private:
    FileReadBuffer& f;
    BlockOffsetReader blockOffset;
    uint64_t startPos;
    uint64_t size;
    uint32_t entries;
    uint64_t blockSize;

    inline uint64_t dataPos() {
        return startPos + blockOffset.sizeOf();
    }
};


#endif //ROARINGGEOMAPS_ROARINGBITMAPCOLUMNREADER_H
