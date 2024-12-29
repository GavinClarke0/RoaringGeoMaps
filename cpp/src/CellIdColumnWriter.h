#ifndef ROARINGGEOMAPS_CELLIDCOLUMNWRITER_H
#define ROARINGGEOMAPS_CELLIDCOLUMNWRITER_H

#include <vector>
#include <cstdint>
#include "io/FileWriteBuffer.h"
#include "BlockIndexWriter.h"
#include "BlockOffset.h"
#include "Block.h"

class Uint64BlockWriter : public FixedBlockWriter<uint64_t> {
public:
    explicit Uint64BlockWriter(uint64_t blockSize): FixedBlockWriter<uint64_t>(blockSize) {};
    void writeValue(FileWriteBuffer& f, uint64_t value) {
        writeLittleEndianUint64(f, value);
    };
};

class CellIdColumnWriter {
public:
    explicit CellIdColumnWriter(uint64_t blockSize);
    void addValue(uint64_t value);
    uint64_t writeToFile(FileWriteBuffer& f);
    BlockIndexWriter<uint64_t> blockIndex(); // Returns a block index for the current state of the

private:
    uint64_t blockSize;
    Uint64BlockWriter currentWriteBlock;
    std::vector<Uint64BlockWriter> blocks;
};

#endif //ROARINGGEOMAPS_CELLIDCOLUMNWRITER_H