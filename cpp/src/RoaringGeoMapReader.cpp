#include "RoaringGeoMapReader.h"
#include "CellIdColumnReader.h"
#include "s2/s2latlng.h"
#include <s2/s2region_coverer.h>

const int MIN_LEVEL = 3;

RoaringGeoMapReader::RoaringGeoMapReader(const std::string& filePath) {
    // Create a read buffer from the file path using FileReadBuffer
    f = std::make_unique<FileReadBuffer>(filePath);
    // Initialize other members or perform additional setup as needed
    header = Header::readFromFile(*f);

    auto coverBitmapPos = header.getCoverBitmapOffset();
    cellFilter = CellFilter::deserialize(*f, coverBitmapPos.first, coverBitmapPos.second);

    //roaring::Roaring64Map::frozenView(f->view(coverBitmapPos.first, coverBitmapPos.second));
//
//    auto containsBitmapPos = header.getContainsBitmapOffset();
//    containsBitmap = roaring::Roaring64Map::frozenView(f->view(containsBitmapPos.first, containsBitmapPos.second));

    auto [keyColumnOffset, keyColumnSize] = header.getKeyIndexPos();
    keyColumn = std::make_unique<ByteColumnReader>(*f, keyColumnOffset, keyColumnSize, header.getKeyIndexEntries(), header.getBlockSize());

    auto [cellColumnOffset, cellColumnSize] = header.getCellIndexPos();
    cellIdColumn = std::make_unique<CellIdColumnReader>(*f, cellColumnOffset, cellColumnSize, header.getCellIndexEntries(), header.getBlockSize());

    auto [bitmapColumnOffset, bitmapColumnSize] = header.getBitmapPos();
    bitmapColumn = std::make_unique<RoaringBitmapColumnReader>(*f, bitmapColumnOffset, bitmapColumnSize, header.getCellIndexEntries(), header.getBlockSize());

}

RoaringGeoMapReader::~RoaringGeoMapReader() = default;


std::vector<std::vector<char>>  RoaringGeoMapReader::Contains(const S2CellUnion& queryRegionNormalized) {

    // 1. Denormalize the cell id to the same levels that we stored the cells at.
    auto queryRegion = std::vector<S2CellId>(); // May not need a unique pointer do to the lifetime
    queryRegionNormalized.Denormalize(MIN_LEVEL, header.getLevelIndexBucketRange(), &queryRegion);

    // Find the ranges of cellIds for each cell in the query region.
    std::set<std::pair<uint64_t, uint64_t>> cellRanges;
    for (auto cellId : queryRegion) {
        auto min = cellId.range_min().id();
        auto max = cellId.range_max().id();
        auto result = (cellFilter.containsRange(min, max));
        if (std::get<2>(result))
            cellRanges.insert({std::get<0>(result), std::get<1>(result)});
        // Insert the range of CellIds that contains the child cells of each cell in the query region.
    }

    // FInd the ancestor cells of each cell in the query region.
    std::set<uint64_t> cellAncestors;
    for (auto cellId : queryRegion) {
        for (int i = cellId.level()-header.getLevelIndexBucketRange(); i >= MIN_LEVEL; i -= header.getLevelIndexBucketRange() ) {
            if (cellFilter.contains(cellId.parent(i).id()))
                cellAncestors.insert(cellId.parent(i).id());
        }
    }

//    if (!std::is_sorted(cellRanges.begin(), cellRanges.end())){
//        auto x = 10;
//    }
    auto cellIdBlockIndex= cellIdColumn->BlockIndex();
    auto blocksValues = cellIdBlockIndex.QueryValuesBlocks(cellRanges, cellAncestors);
    roaring::Roaring resultKeyIds;

    for (auto blockValue: blocksValues) {
        // Collect the total set of key ids found in all blocks;
        resultKeyIds |= queryBlockValues(blockValue.blockId, blockValue.ranges, blockValue.values);
    }

    auto keyBlockValues = queryBlocksByIndexes(resultKeyIds);
    std::vector<std::vector<char>> results;
    for (const auto& blockValue: keyBlockValues) {
        auto block = keyColumn->ReadBlock(blockValue.blockId);
        auto keyValues = block.readIndexes(blockValue.values);
        results.insert(results.end(), keyValues.begin(), keyValues.end());
    }

    return results;
}

std::vector<std::vector<char>> RoaringGeoMapReader::Intersects(const S2CellUnion &cellIds) {
    return {};
}


roaring::Roaring RoaringGeoMapReader::queryBlockValues(uint32_t &blockId, std::vector<std::pair<uint64_t, uint64_t>> &ranges, std::vector<uint64_t> &values)  {
    // CellId and KeyId (RoaringBitMap columns are aligned. Cell Ids found at index x in the cell block's correspond
    // to bitmaps of all keyIds present in the cell at the same index.
    auto cellIdBlock = cellIdColumn->ReadBlock(blockId); // TODO should probably cache this
    auto keyIdBlock = bitmapColumn->ReadBlock(blockId);

    // Since we only query for values we know are in the index due to their presence in the contains and
    // covers index bitmaps we can construct the result set without worrying about allocating for a result
    // which may never be present.
    auto indexes = cellIdBlock.queryValueIndexes(values);
    auto indexRanges = cellIdBlock.queryValueRangesIndexes(ranges);

    std::vector<const roaring::Roaring*> keyIds;
    auto rangeBitmaps = keyIdBlock.readIndexRanges(indexRanges);
    auto indexBitmaps = keyIdBlock.readIndexes(indexes);
    keyIds.reserve(rangeBitmaps.size() + indexBitmaps.size());
    for (const auto& bitmap: rangeBitmaps)
        keyIds.emplace_back(bitmap.get());
    for (const auto& bitmap: indexBitmaps)
        keyIds.emplace_back(bitmap.get());

    return roaring::Roaring::fastunion(keyIds.size(), keyIds.data());
}

std::vector<BlockValues<uint32_t>> RoaringGeoMapReader::queryBlocksByIndexes(roaring::Roaring& queryValues) {
    std::vector<BlockValues<uint32_t>> results;
    for (const auto& query : queryValues) {
        // Find the first index in `values` that is greater than or equal to `query`.
        uint32_t blockIndex = query / header.getBlockSize();
        uint32_t blockQueryIndex = query - (blockIndex * header.getBlockSize());
        // Calculate the block index.
        if (results.empty() || results.back().blockId != blockIndex) {
            // normalize queryIndex by it's block. I.e we ask for index 513 of the entire column, however within the
            // block this is index 0;
            results.push_back({blockIndex, {blockQueryIndex}});
        } else {
            results.back().values.emplace_back(blockQueryIndex);
        }
    }
    return results;
}



