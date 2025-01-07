#include <roaring.hh>
#include <vector>
#include <memory>
#include "io/FileWriteBuffer.h"
#include "Block.h"


class RoaringBitMapBlockWriter : public BlockWriter<roaring::Roaring *> {
public:
    explicit RoaringBitMapBlockWriter(uint64_t blockSize) : BlockWriter<roaring::Roaring *>(blockSize) {};

    void writeValue(FileWriteBuffer &f, roaring::Roaring *value) override {
        f.write([&](char *data) { value->write(data, false); }, value->getSizeInBytes(false));
    };
};


class RoaringBitmapColumnWriter {
public:
    explicit RoaringBitmapColumnWriter(uint64_t blockSize);

    void addBitmap(roaring::Roaring *bitmap);

    uint64_t writeToFile(FileWriteBuffer &f);

private:
    RoaringBitMapBlockWriter blockSize;
    RoaringBitMapBlockWriter currentWriteBlock;
    std::vector<RoaringBitMapBlockWriter> blocks;
};
