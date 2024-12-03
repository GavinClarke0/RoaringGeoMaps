#ifndef ROARINGGEOMAPS_BYTECOLUMNREADER_H
#define ROARINGGEOMAPS_BYTECOLUMNREADER_H

#include "BlockOffset.h"
#include "Block.h"

class BytesBlockReader : public BlockReader<std::vector<char>> {
public:
    explicit BytesBlockReader(FileReadBuffer& f, uint64_t position, uint64_t size, uint64_t entries):
            BlockReader<std::vector<char>>(f,  position,  size,  entries) {};

    std::vector<char> readValue(FileReadBuffer& f, uint64_t position, uint64_t size) override {
        auto data = f.view(position, size);
        return {data, data+size};
    };
};

class ByteColumnReader {
public:
    ByteColumnReader(FileReadBuffer& f, uint64_t startPos, uint32_t size, uint32_t entries, uint16_t blockSize);
    BytesBlockReader ReadBlock(uint32_t blockIndex);

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


#endif //ROARINGGEOMAPS_BYTECOLUMNREADER_H
