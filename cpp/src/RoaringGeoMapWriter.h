#ifndef ROARING_GEO_MAP_WRITER_H
#define ROARING_GEO_MAP_WRITER_H

#include <string>
#include <memory>
#include "s2/s2region.h"
#include "s2/s2cell_union.h"
#include "roaring64map.hh"


inline bool compareBitMapMin(std::pair<std::string, roaring::Roaring64Map> a, std::pair<std::string, roaring::Roaring64Map> b) {
    return a.second.minimum() < b.second.minimum();
}

using CellKeyIdsMap = std::map<uint64_t, std::unique_ptr<roaring::Roaring>>;
using KeyCoverPair = std::pair<std::string, roaring::Roaring64Map>;


struct CompareKeyCoverPair {
    bool operator()(const KeyCoverPair& a, const KeyCoverPair& b) const {
        return a.second.minimum() < b.second.minimum();
    }
};

// RoaringGeoMapWriter is responsible for writing geospatial data
// associated with an S2Region and a descriptive string (up to 512 characters).
class RoaringGeoMapWriter {


public:
    RoaringGeoMapWriter(int levelIndexBucketRange);
    // Writes the provided S2Region and a description (up to 512 characters).
    // If the description exceeds 512 characters, the method will return false.
    //
    // Parameters:
    // - region: An S2Region to be processed (e.g., S2Cap, S2Polygon, etc.)
    // - description: A string (max 512 characters)
    //
    // Returns:
    // - true if the region and description were successfully processed,
    //   false if the description exceeds 512 characters.
    bool write(const S2CellUnion& region, const std::string& key);
    bool build(std::string filePath);
private:
    int levelIndexBucketRange;
    roaring::Roaring64Map indexRegionContainsBitMap;
    roaring::Roaring64Map indexRegionCoversBitMap;
    std::multiset<KeyCoverPair, CompareKeyCoverPair> keysToRegionCover;
};

#endif // ROARING_GEO_MAP_WRITER_H