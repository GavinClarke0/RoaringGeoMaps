#ifndef ROARING_GEO_MAP_WRITER_H
#define ROARING_GEO_MAP_WRITER_H

#include <string>
#include <memory>
#include "s2/s2region.h"
#include "s2/s2cell_union.h"
#include "roaring64map.hh"
#include "CellFilter.h"

inline bool
compareBitMapMin(std::pair<std::string, roaring::Roaring64Map> a, std::pair<std::string, roaring::Roaring64Map> b) {
    return a.second.minimum() < b.second.minimum();
}

using CellKeyIdsMap = std::map<uint64_t, std::unique_ptr<roaring::Roaring>>;
using KeyCoverPair = std::pair<std::string, std::set<uint64_t>>;


struct CompareKeyCoverPair {
    bool operator()(const KeyCoverPair &a, const KeyCoverPair &b) const {
        return *a.second.begin() < *b.second.begin();
    }
};

// RoaringGeoMapWriter is responsible for writing geospatial data
// associated with an S2Region and a descriptive string (up to 512 characters).
class RoaringGeoMapWriter {


public:
    RoaringGeoMapWriter(int levelIndexBucketRange);

    // Writes the provided S2Region and associated bytes key.
    // If the description exceeds 512 characters, the method will return false.
    //
    // Parameters:
    // - region: An S2Region to be processed (e.g., S2Cap, S2Polygon, etc.)
    // - key/data: Byte array vector<char>
    //
    // Returns:
    // - true if the region and description were successfully processed,
    //   false if the description exceeds 512 characters.
    bool write(const S2CellUnion &region, const std::string &key);

    bool build(const std::string &filePath);

private:
    int levelIndexBucketRange;
    CellFilter::Builder filterBuilder;
    // TODO: We can use a regular hash set and then use a value to store the minimum value for sorting.
    std::multiset<KeyCoverPair, CompareKeyCoverPair> keysToRegionCover;
};

#endif // ROARING_GEO_MAP_WRITER_H