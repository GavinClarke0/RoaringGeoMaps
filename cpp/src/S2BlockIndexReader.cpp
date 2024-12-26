#include <S2BlockIndexReader.h>
#include <cassert>

S2BlockIndexReader::S2BlockIndexReader(FileReadBuffer &f, uint64_t pos, uint64_t size) : values(f, pos, size) {}

std::vector<S2BlockValues<uint64_t>>
S2BlockIndexReader::QueryValuesBlocks(std::set<std::pair<uint64_t, uint64_t>> &cellRanges,
                                      std::set<uint64_t> &cellValues) {

    // This function assumes that cellValues is not over lapping with cell ranges. We make this assumption because this
    // function is only ever called with values derived from s2 region covers and the ancestors of any cell in the s2
    // region cover cannot overlap in CellID value with the children cell ranges of any other cell in the region cover.

    // Ensure the query ranges and cell values are sorted.
    assert(std::is_sorted(cellRanges.begin(), cellRanges.end()) && "query ranges must be sorted");
    assert(std::is_sorted(cellValues.begin(), cellValues.end()) && "query values must be sorted");

    std::vector<S2BlockValues<uint64_t>> results;
    // Search for which blocks contain values in the search range.
    for (const auto &query: cellRanges) {
        auto blockIndexRangeOption = findBlockRange(values, query);
        if (blockIndexRangeOption == std::nullopt)
            continue;

        auto blockIndexRange = blockIndexRangeOption.value();
        uint32_t startBlock = blockIndexRange.first;
        uint32_t endBlock = blockIndexRange.second;

        for (uint32_t blockId = startBlock; blockId <= endBlock; ++blockId) {

            // Find the correct position to insert or update in results. We likely can assume that each value will be inserted
            // in the end block or a block near it but to keep the code consistent will use binary search for now.
            auto it = std::lower_bound(results.begin(), results.end(), blockId,
                                       [](const S2BlockValues<uint64_t> &block, uint32_t id) {
                                           return block.blockId < id;
                                       });

            if (it != results.end() && it->blockId == blockId) {
                // Block ID exists; update it.
                it->ranges.push_back(query);
            } else {
                // Block ID does not exist; insert a new entry at the correct position.
                results.insert(it, S2BlockValues<uint64_t>{blockId, {query}, {}});
            }
        }
    }

    // Search for which blocks contain values we search for.
    for (uint64_t cellValue: cellValues) {
        auto blockIdOption = findBlockValue(values, cellValue);
        if (blockIdOption == std::nullopt)
            continue;


        auto blockId = blockIdOption.value();
        // Find the correct position to insert or update in results.
        auto it = std::lower_bound(results.begin(), results.end(), blockId,
                                   [](const S2BlockValues<uint64_t> &block, uint32_t id) {
                                       return block.blockId < id;
                                   });

        if (it != results.end() && it->blockId == blockId) {
            // Block ID exists; update it.
            it->values.push_back(cellValue);
        } else {
            // Block ID does not exist; insert a new entry at the correct position.
            results.insert(it, S2BlockValues<uint64_t>{blockId, {}, {cellValue}});
        }
    }

    return results;
}
