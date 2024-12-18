#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "roaring/roaring.h" // Include the Roaring Bitmap library
#include "s2/s2cell_id.h"     // Include the S2 library
#include "Header.h"
#include "roaring64map.hh"
#include "CellIdColumnReader.h"
#include "s2/s2cell_union.h"
#include "ByteColumnReader.h"
#include "RoaringBitmapColumnReader.h"


class RoaringGeoMapReader {

public:
    // Constructor that takes a file path and constructs a read buffer
    explicit RoaringGeoMapReader(const std::string& filePath);
    ~RoaringGeoMapReader();
    std::vector<std::vector<char>> Contains(const S2CellUnion& cellIds);
    std::vector<std::vector<char>> Intersects(const S2CellUnion& cellIds);

private:
    std::unique_ptr<FileReadBuffer> f;
    Header header;
    roaring::Roaring64Map coversBitmap;
    roaring::Roaring64Map containsBitmap;
    std::unique_ptr<ByteColumnReader> keyColumn;
    std::unique_ptr<CellIdColumnReader> cellIdColumn;
    std::unique_ptr<RoaringBitmapColumnReader> bitmapColumn;

    roaring::Roaring queryBlock(uint32_t& blockId, std::vector<uint64_t>& values);
    roaring::Roaring queryBlockValues(uint32_t &blockId, std::vector<std::pair<uint64_t, uint64_t>> &valueRanges, std::vector<uint64_t> &values);
    std::vector<BlockValues<uint32_t>> queryBlocksByIndexes(roaring::Roaring& queryValues);
};

