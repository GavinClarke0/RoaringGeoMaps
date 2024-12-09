#pragma once
#ifndef ROARINGGEOMAPS_S2BLOCKINDEXREADER_H
#define ROARINGGEOMAPS_S2BLOCKINDEXREADER_H

#include <cstdint>
#include <set>
#include "WriteHelpers.h"  // Assuming this contains the write functions
#include "io/FileReadBuffer.h"
#include "unordered_set"
#include "VectorView.h"
#include "s2/s2cell_id.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include "BlockIndexReader.h"

template <typename T>
class S2BlockValues{
public:
    uint32_t blockId;
    std::vector<std::pair<T, T>> ranges;
    std::vector<T> values;
};

// Binary search for matching prefix
inline std::optional<std::pair<uint32_t, uint32_t>> findBlockRange(VectorView<uint64_t>& sortedValues, std::pair<uint64_t, uint64_t> cellRange) {

    // Perform binary search to find the smallest child CellID that we are an ancestor of.
    auto lowerIt = std::lower_bound(sortedValues.begin(), sortedValues.end(), cellRange.first);
    if (lowerIt == sortedValues.end())
        return std::nullopt;


    auto upperIt = lowerIt;
    while(upperIt != sortedValues.end() && *upperIt <= cellRange.second)
        upperIt++;

    // TODO: probably a more efficient way to do this but this involves the least amount evaluated statements;
    if (upperIt == sortedValues.end())
        upperIt--;

    return std::pair<uint32_t, uint32_t>(std::distance(sortedValues.begin(), lowerIt), std::distance(sortedValues.begin(), upperIt));
}

inline std::optional<uint32_t> findBlockValue(VectorView<uint64_t>& sortedValues, uint64_t cellId) {
    // Perform binary search to find the smallest child  CellId that we are an ancestor of.
    auto lowerIt = std::lower_bound(sortedValues.begin(), sortedValues.end(), cellId);
    if (lowerIt == sortedValues.end()) {
        return std::nullopt;
    }

    return std::distance(sortedValues.begin(), lowerIt);
}


class S2BlockIndexReader {
public:
    S2BlockIndexReader(FileReadBuffer& f, uint64_t pos, uint64_t size);
    std::vector<S2BlockValues<uint64_t>>  QueryValuesBlocks(std::vector<std::pair<uint64_t, uint64_t>>& ranges, std::set<uint64_t>& values);
    uint64_t sizeOf() {return sizeof(uint64_t) * values.size();};
private:
    VectorView<uint64_t> values;
};


#endif //ROARINGGEOMAPS_S2BLOCKINDEXREADER_H
