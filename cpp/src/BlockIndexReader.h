#ifndef ROARINGGEOMAPS_BLOCKINDEXREADER_H
#define ROARINGGEOMAPS_BLOCKINDEXREADER_H

#include <cstdint>
#include <set>
#include "WriteHelpers.h"  // Assuming this contains the write functions
#include "io/FileReadBuffer.h"
#include "unordered_set"
#include "VectorView.h"

// Struct which maps a block id to values contained in the block.
template <typename T>
class BlockValues{
public:
    uint32_t blockId;
    std::vector<T> values;
};

template <typename T, typename Iterator>
class BlockIndexReader {
    static_assert(std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value,
                  "BlockIndexReader only supports uint32_t and uint64_t types.");
public:
    BlockIndexReader(FileReadBuffer& f, uint64_t pos, uint64_t size);
    std::vector<BlockValues<T>>  QueryValuesBlocks(std::set<T> values);
    uint64_t sizeOf() {return sizeof(T) * values.size();};
private:
    VectorView<T> values;
};


template <typename T,typename Iterator>
BlockIndexReader<T, Iterator>::BlockIndexReader(FileReadBuffer& f, uint64_t pos, uint64_t size): values(f, pos, size) {}

// TODO: use a iterator interface;
template<typename T, typename Iterator>
std::vector<BlockValues<T>> BlockIndexReader<T, Iterator>::QueryValuesBlocks(std::set<T> queryValues) {
    // Ensure the query values are sorted.
    assert(std::is_sorted(queryValues.begin(), queryValues.end()) && "query values must be sorted");
    std::vector<BlockValues<T>> results;

    // Iterator for traversing the stored values.
    auto valuesIt = values.begin();
    for (const auto& query : queryValues) {
        // Find the first index in `values` that is greater than or equal to `query`.
        auto it = std::lower_bound(valuesIt, values.end(), query);

        // Calculate the block index.
        auto blockIndex = static_cast<uint32_t>(std::distance(values.begin(), it));
        if (results.empty() || results.back().blockId != blockIndex) {
            results.push_back({blockIndex, {query}});
        } else {
            results.back().values.emplace_back(query);
        }
        valuesIt = it;
    }
    return results;
}

#endif // ROARINGGEOMAPS_BLOCKINDEXREADER_H