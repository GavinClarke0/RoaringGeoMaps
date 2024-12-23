#ifndef ROARINGGEOMAPS_CELLFILTER_H
#define ROARINGGEOMAPS_CELLFILTER_H

#include <vector>
#include <cstdint>
#include <iostream>
#include "memory"
#include "s2/s2cell_id.h"
#include "io/FileWriteBuffer.h"
#include "io/FileReadBuffer.h"

namespace surf {
    class SuRF; // Forward declaration
}

class CellFilter {
private:
    // Private constructor to enforce the builder pattern
    surf::SuRF* filter;
    explicit CellFilter(surf::SuRF* filter);

public:
    explicit CellFilter(const std::vector<std::string>& values);
    explicit CellFilter();
    ~CellFilter();

    // Builder class to construct the CellFilter object
    class Builder {
    private:
        std::set<std::string> values;
    public:
        //void insert(S2CellId& value);
        CellFilter build();
        void insertMany(const std::vector<S2CellId> &insertValues);
    };

    // Static methods for serialization and deserialization
    std::pair<uint64_t, uint64_t> serialize(FileWriteBuffer& f);
    static CellFilter deserialize(FileReadBuffer& f, uint64_t pos, uint64_t size);

    bool contains(uint64_t cellId);
    std::tuple<uint64_t, uint64_t, bool> containsRange(uint64_t minCellId, uint64_t maxCellId);
};

#endif //ROARINGGEOMAPS_CELLFILTER_H
