#include <vector>

#ifndef ROARINGGEOMAPS_BLOCKFILTER_H
#define ROARINGGEOMAPS_BLOCKFILTER_H

template<typename T, typename H, typename IndexColumn, typename ValueColumn>
class BlockFilter {
private:
    uint64_t blockSize;
    IndexColumn &indexColumn;
    ValueColumn &valueColumn;
public:
    BlockFilter(IndexColumn &indexColumn, ValueColumn &valueColumn, uint64_t blockSize): indexColumn(indexColumn), valueColumn(valueColumn), blockSize(blockSize) {};
    std::vector<H> Filter(uint32_t blockId, std::vector<T> values) {

        assert(std::is_sorted(values.begin(), values.end()));

        // 1. search index column for indexes of values that we want to query.

        //indexColumn.



    }
};


#endif //ROARINGGEOMAPS_BLOCKFILTER_H
