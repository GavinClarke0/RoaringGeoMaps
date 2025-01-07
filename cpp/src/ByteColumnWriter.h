#include <vector>
#include <memory>
#include "io/FileWriteBuffer.h"
#include "Block.h"


class BytesBlockWriter : public BlockWriter<std::vector<char>> {
public:
    explicit BytesBlockWriter(uint64_t blockSize) : BlockWriter<std::vector<char>>(blockSize) {};

    void writeValue(FileWriteBuffer &f, std::vector<char> value) {
        f.write(value.data(), value.size());
    };
};

class ByteColumnWriter {
public:
    ByteColumnWriter(uint64_t blockSize);

    void addBytes(const std::vector<char> &data);

    uint64_t writeToFile(FileWriteBuffer &f);

private:
    BytesBlockWriter blockSize;
    BytesBlockWriter currentWriteBlock;
    std::vector<BytesBlockWriter> blocks;
};
