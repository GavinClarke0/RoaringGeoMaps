#ifndef ROARING_GEO_MAP_READER_H
#define ROARING_GEO_MAP_READER_H

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
#include "CellFilter.h"

class RoaringGeoMapReader {

public:
    // Constructor that takes a file path and constructs a read buffer
    explicit RoaringGeoMapReader(const std::string &filePath);

    ~RoaringGeoMapReader();

    std::vector<std::vector<char>> Contains(const S2CellUnion &cellIds);

    std::vector<std::vector<char>> Intersects(const S2CellUnion &cellIds);

private:
    std::unique_ptr<FileReadBuffer> f;
    Header header;
    CellFilter cellFilter;
    std::unique_ptr<ByteColumnReader> keyColumn;
    std::unique_ptr<CellIdColumnReader> cellIdColumn;
    std::unique_ptr<RoaringBitmapColumnReader> bitmapColumn;

    std::unique_ptr<roaring::Roaring>
    queryBlockValues(uint32_t &blockId, std::vector<std::pair<uint64_t, uint64_t>> &valueRanges,
                     std::vector<uint64_t> &values);

    std::vector<BlockValues<uint32_t>> queryBlocksByIndexes(roaring::Roaring &queryValues);
};

#endif // ROARING_GEO_MAP_READER_H