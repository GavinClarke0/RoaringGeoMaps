#ifndef ROARINGGEOMAPS_HEADER_H
#define ROARINGGEOMAPS_HEADER_H

#include <cstdint>
#include "io/FileWriteBuffer.h"
#include "io/FileReadBuffer.h"


const uint32_t HEADER_SIZE = 128; // Header is 32 byte aligned to allow for use with frozen bitmaps.

class Header {
public:

    explicit Header(int, int);
    Header();

    void writeToFile(FileWriteBuffer &file) const;

    static Header readFromFile(FileReadBuffer &buffer);

    std::pair<uint64_t, uint64_t>  getCellIdFilterOffset() const;

    void setCellIdFilterOffset(uint64_t offset, uint64_t size);

    std::pair<uint64_t, uint64_t>  getKeyIndexPos() const;

    void setKeyIndexOffset(uint64_t offset, uint64_t size);

    std::pair<uint64_t, uint64_t>  getCellIndexPos() const;

    void setCellIndexOffset(uint64_t offset, uint64_t size);

    std::pair<uint64_t, uint64_t>  getBitmapPos() const;

    void setBitmapOffset(uint64_t offset, uint64_t size);

    uint32_t getKeyIndexEntries() const;

    void setKeyIndexEntries(uint32_t size);

    uint32_t getCellIndexEntries() const;

    void setCellIndexEntries(uint32_t size);

    uint8_t getLevelIndexBucketRange() const;

    void setLevelIndexBucketRange(uint8_t levelIndexBucketRange);

    uint16_t getBlockSize() const;

    void setBlockSize(uint16_t blockSize);

    uint8_t getFileType() const;

    void setFileType(uint8_t fileType);

private:
    uint64_t cellIdFilterOffset = 0;
    uint64_t cellIdFilterSize = 0;
    uint64_t keyIndexOffset = 0;
    uint32_t keyIndexSize = 0;
    uint64_t cellIndexOffset = 0;
    uint64_t cellIndexSize = 0;
    uint64_t roaringIndexOffset = 0;
    uint64_t roaringIndexSize = 0;
    uint32_t keyIndexEntries = 0;
    uint32_t cellIndexEntries = 0;
    uint8_t levelIndexBucketRange = 1;
    uint16_t blockSize;
    uint8_t fileType;
};

#endif // ROARINGGEOMAPS_HEADER_H
