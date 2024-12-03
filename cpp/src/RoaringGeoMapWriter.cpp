#include "RoaringGeoMapWriter.h"
#include "roaring64map.hh"
#include "CellIdColumnWriter.h"
#include "ByteColumnWriter.h"
#include "io/FileWriteBuffer.h"
#include "RoaringBitmapColumnWriter.h"
#include "Header.h"
#include <algorithm>

const int MIN_LEVEL = 3;
const uint64_t BLOCK_SIZE = 512; // TODO: turn into const template -> rows per key

RoaringGeoMapWriter::RoaringGeoMapWriter(int levelIndexBucketRange) : levelIndexBucketRange(levelIndexBucketRange) {}

// Writes a new Key -> region cover pair to be indexed in the index. For now, we will assume the entire can be constructed
// only in memory.
bool RoaringGeoMapWriter::write(const S2CellUnion &region, const std::string &key) {

    // 1. Get normalized region cover
    auto normalizedRegion = std::make_unique<std::vector<S2CellId>>(); // May not need a unique pointer do to the lifetime
    region.Denormalize(MIN_LEVEL, levelIndexBucketRange, normalizedRegion.get());

    roaring::Roaring64Map regionCoverBitMap;
    roaring::Roaring64Map regionContainsBitmap; // Do I need this one
    for (auto cell : *normalizedRegion) {

        auto normalizedCellId = cell.parent(cell.level() - (cell.level() % levelIndexBucketRange));
        regionContainsBitmap.add(normalizedCellId.id());
        regionCoverBitMap.add(normalizedCellId.id());
        for (int i = normalizedCellId.level(); i >= MIN_LEVEL; i -= levelIndexBucketRange ) {
            regionCoverBitMap.add(normalizedCellId.parent(i).id()); // TODO figure out the level mod!
        }
    }
    // 2. Add new region cover to global cell inclusion map by taking the union of the new regions bit map
    // and the index entire region cover.

    // TODO: expose batch method which uses fastUnion on multiple maps at once.
    indexRegionCoversBitMap |= regionCoverBitMap;
    indexRegionContainsBitMap |= regionContainsBitmap;
    // 3. Store the key -> pair mapping for finally constructing the index.

    keysToRegionCover.insert({key, std::move(regionCoverBitMap)});
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

    // 2. Iterate through the keys -> cells build up the map of cells ids -> key_ids. The key_id of a
    // key is the uint32 index of the value when sorted in the keysToRegionCover vector. In theory, key_ids
    // with near key_id values should represent data that is close spatially.
    auto cellToKeyMap = std::make_unique<CellKeyIdsMap>();
    uint32_t index = 0;
    for (auto keyToCover = keysToRegionCover.begin(); keyToCover != keysToRegionCover.end(); ++keyToCover, ++index) {
        auto regionCover = (*keyToCover).second;
        // Note, the index is now the uint32 id of the key
        auto args = IterateArgs{index ,cellToKeyMap.get()};
        regionCover.iterate(add_key_id, static_cast<void*>(&args));
    }

    Header header(MIN_LEVEL);
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
    for (auto keyToCover = keysToRegionCover.begin(); keyToCover != keysToRegionCover.end(); ++keyToCover) {
        const std::string& key = (*keyToCover).first;
        keyColumn.addBytes(std::move(std::vector<char>(key.begin(), key.end())));
    }
    // There is no index for the key's as they are indexed by their relative position in the column and thus the block
    // a key resides in can be inferred from the block size and number of entries in the column
    uint64_t keyIndexOffset = f->offset();
    uint64_t keyIndexSize = keyColumn.writeToFile(*f);
    header.setKeyIndexOffset(keyIndexOffset, keyIndexSize);
    header.setKeyIndexEntries(keysToRegionCover.size());

    // Write the CellId to Key_Id section
    CellIdColumnWriter cellIdColumn(BLOCK_SIZE);
    RoaringBitmapColumnWriter bitmapColumn(BLOCK_SIZE);
    for (const auto& [cellId, keyIdBitmap] : *cellToKeyMap) {
        cellIdColumn.addValue(cellId);
        bitmapColumn.addBitmap(&(*keyIdBitmap)); // TODO: compression ?
    }

    uint64_t offset = f->offset();
    uint64_t cellIndexSize = cellIdColumn.writeToFile(*f);
    header.setCellIndexOffset(offset, cellIndexSize);
    header.setCellIndexEntries(cellToKeyMap->size());

    offset = f->offset();
    uint64_t bitmapSize = bitmapColumn.writeToFile(*f);
    header.setBitmapOffset(offset, bitmapSize);

    // Seek head of file buffer and write header
    f->reset();
    header.writeToFile(*f);
    f->flush(0);
    return true;
}
