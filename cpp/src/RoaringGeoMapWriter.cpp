#include "RoaringGeoMapWriter.h"
#include "roaring64map.hh"
#include "CellIdColumnWriter.h"
#include "ByteColumnWriter.h"
#include "io/FileWriteBuffer.h"
#include "RoaringBitmapColumnWriter.h"
#include "Header.h"
#include <algorithm>

const int MIN_LEVEL = 3;
const uint64_t BLOCK_SIZE = 1024; // TODO: turn into const template -> rows per key

RoaringGeoMapWriter::RoaringGeoMapWriter(int levelIndexBucketRange) : levelIndexBucketRange(levelIndexBucketRange) {}

// Writes a new Key -> region cover pair to be indexed in the index. For now, we will assume the entire can be constructed
// only in memory.
bool RoaringGeoMapWriter::write(const S2CellUnion &region, const std::string &key) {

    // 1. Get normalized region cover
    auto normalizedRegion = std::make_unique<std::vector<S2CellId>>(); // May not need a unique pointer do to the lifetime
    region.Denormalize(MIN_LEVEL, levelIndexBucketRange, normalizedRegion.get());

    roaring::Roaring64Map regionCoverBitMap;
    regionCoverBitMap.addMany(region.size(), reinterpret_cast<const uint64_t*>(region.data()));

    auto keyId = keys.size();
    keys.emplace_back(key);
    for (auto cellId : *normalizedRegion){
        if (s2CellsToKeys.contains(cellId.id())) {
            s2CellsToKeys[cellId.id()]->add(keyId);
        } else {
           auto bitmap =  std::make_unique<roaring::Roaring>();
           bitmap->add(keyId);
           s2CellsToKeys[cellId.id()] = std::move(bitmap);
        }
    }
    return true;
}


struct IterateArgs {
    uint32_t keyId;
    CellKeyIdsMap* cellToKeyMap;
};

bool add_key_id(uint64_t cellId, void* argsV) {
    auto* values = static_cast<IterateArgs*>(argsV);
    auto cellKeyIdsIt = values->cellToKeyMap->find(cellId);
    if (cellKeyIdsIt != values->cellToKeyMap->end()) {
        cellKeyIdsIt->second->add(values->keyId);
    } else {
        auto keyIdBitMap = std::make_unique<roaring::Roaring>();
        keyIdBitMap->add(values->keyId);
        values->cellToKeyMap->emplace(cellId, std::move(keyIdBitMap));
    }
    return true;
}


void reserve_header(FileWriteBuffer *f) {
    f->write(std::vector<char>(HEADER_SIZE, 0).data(), HEADER_SIZE);
}


bool RoaringGeoMapWriter::build(std::string filePath) {


    Header header(levelIndexBucketRange, BLOCK_SIZE);
    // 3. Create the file and reserve the header space by write space of header as 0'd out memory.
    std::unique_ptr<FileWriteBuffer> f = std::make_unique<FileWriteBuffer>(filePath, 4096 * 4);
    reserve_header(f.get());

    // 4. Write the S2 cell hierarchical cover roaring bitmap.
    auto [coverOffset, coverSize] = f->write([&](char* data) {indexRegionCoversBitMap.writeFrozen(data);}, indexRegionCoversBitMap.getFrozenSizeInBytes());
    header.setCoverBitmapOffset(coverOffset, coverSize);

    // 5. Write the S2 cell intersection roaring bitmap 32 byte aligned
   auto [containsOffset, containsSize] = f->write([&](char* data) {indexRegionContainsBitMap.writeFrozen(data);}, indexRegionContainsBitMap.getFrozenSizeInBytes());
   header.setContainsBitmapOffset(containsOffset, containsSize);

    // 6. Write the key_id column to the roaring geomap, the keys position in the key_id column serves as it's index.
    ByteColumnWriter keyColumn(BLOCK_SIZE);
    for (auto key: keys) {
        keyColumn.addBytes(std::vector<char>(key.begin(), key.end()));
    }
    // There is no index for the key's as they are indexed by their relative position in the column and thus the block
    // a key resides in can be inferred from the block size and number of entries in the column
    uint64_t keyIndexOffset = f->offset();
    uint64_t keyIndexSize = keyColumn.writeToFile(*f);
    header.setKeyIndexOffset(keyIndexOffset, keyIndexSize);
    header.setKeyIndexEntries(keys.size());

    // Write the CellId to Key_Id section
    CellIdColumnWriter cellIdColumn(BLOCK_SIZE);
    RoaringBitmapColumnWriter bitmapColumn(BLOCK_SIZE);
    for (const auto& [cellId, keyIdBitmap] : s2CellsToKeys) {
        cellIdColumn.addValue(cellId);
        bitmapColumn.addBitmap(&(*keyIdBitmap)); // TODO: compression ?
    }

    uint64_t offset = f->offset();
    uint64_t cellIndexSize = cellIdColumn.writeToFile(*f);
    header.setCellIndexOffset(offset, cellIndexSize);
    header.setCellIndexEntries(s2CellsToKeys.size());

    offset = f->offset();
    uint64_t bitmapSize = bitmapColumn.writeToFile(*f);
    header.setBitmapOffset(offset, bitmapSize);

    // Seek head of file buffer and write header
    f->reset();
    header.writeToFile(*f);
    f->flush(0);
    return true;
}
