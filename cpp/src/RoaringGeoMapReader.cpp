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
                   coversBitmap = roaring::Roaring64Map::frozenView(f->view(coverBitmapPos.first, coverBitmapPos.second));

    auto containsBitmapPos = header.getContainsBitmapOffset();
    containsBitmap = roaring::Roaring64Map::frozenView(f->view(containsBitmapPos.first, containsBitmapPos.second));

    auto [keyColumnOffset, keyColumnSize] = header.getKeyIndexPos();
    keyColumn = std::make_unique<ByteColumnReader>(*f, keyColumnOffset, keyColumnSize, header.getKeyIndexEntries(), header.getBlockSize());

    auto [cellColumnOffset, cellColumnSize] = header.getCellIndexPos();
    cellIdColumn = std::make_unique<CellIdColumnReader>(*f, cellColumnOffset, cellColumnSize, header.getCellIndexEntries(), header.getBlockSize());

    auto [bitmapColumnOffset, bitmapColumnSize] = header.getBitmapPos();
    bitmapColumn = std::make_unique<RoaringBitmapColumnReader>(*f, bitmapColumnOffset, bitmapColumnSize, header.getCellIndexEntries(), header.getBlockSize());

}

RoaringGeoMapReader::~RoaringGeoMapReader()= default;


std::vector<std::vector<char>>  RoaringGeoMapReader::Contains(const S2CellUnion &queryRegion) {

    // 1. Normalize the cell id to the same levels that we stored the cells at.
    auto normalizedQueryRegion = std::vector<S2CellId>(); // May not need a unique pointer do to the lifetime
    queryRegion.Denormalize(MIN_LEVEL, header.getLevelIndexBucketRange(), &normalizedQueryRegion);

    // 2. Search the contains and the query bitmap for if any query cell id's are present. This is a naive approach
    // for now that can likely be improved via fastmap for large cell sets.
    std::set<uint64_t> queryCellIds; // TODO: determine a method without a set.
    for (auto cellId : normalizedQueryRegion) {
        for (int i = cellId.level(); i >= MIN_LEVEL; i -= header.getLevelIndexBucketRange() ) {
            queryCellIds.insert(cellId.parent(i).id());
        }
    }
    std::set<uint64_t> cellSet;
    for(auto cellId: normalizedQueryRegion) {
        if (coversBitmap.contains(cellId.id()))
                cellSet.insert(cellId.id());
    }
    for(auto cellId: queryCellIds) {
        if (containsBitmap.contains(cellId))
                cellSet.insert(cellId);
}

// 3. Index allows you to determine which blocks of the column hold query values. As the cellId and KeyId
// columns are align so that the cellId at index x corresponds to the bitmap of key id at index x. CellIds
// in block y, store the keyIds which are in the cellId also in block y of the bitmap column)
    BlockIndexReader<uint64_t, std::set<uint64_t>::iterator> cellIdBlockIndex= cellIdColumn->BlockIndex();
    auto blocksValues = cellIdBlockIndex.QueryValuesBlocks(cellSet);
    roaring::Roaring resultKeyIds;
    for (auto blockValue: blocksValues) {
        // Collect the total set of key ids found in all blocks;
        resultKeyIds |= queryBlock(blockValue.blockId, blockValue.values);
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

roaring::Roaring RoaringGeoMapReader::queryBlock(uint32_t &blockId, std::vector<uint64_t> &values) {
    // CellId and KeyId (RoaringBitMap columns are aligned. Cell Ids found at index x in the cell block's correspond
    // to bitmaps of all keyIds present in the cell at the same index.
    auto cellIdBlock = cellIdColumn->ReadBlock(blockId); // TODO should probably cache this
    auto keyIdBlock = bitmapColumn->ReadBlock(blockId);

    // Since we only query for values we know are in the index due to their presence in the contains and
    // covers index bitmaps we can construct the result set without worrying about allocating for a result
    // which may never be present.
    roaring::Roaring keyIds; // TODO see if a better result type is prefered. No idea if there is an advantage here.
    auto indexes = cellIdBlock.queryValueIndexes(values);
    auto keyIdsBitmaps = keyIdBlock.readIndexes(indexes);
    for (const auto& bitmap: keyIdsBitmaps){
        keyIds |= bitmap;
    }
    return keyIds;
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



