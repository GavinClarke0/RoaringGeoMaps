#ifndef ROARINGGEOMAPS_CELLFILTER_H
#define ROARINGGEOMAPS_CELLFILTER_H

#ifndef CELL_FILTER_H
#define CELL_FILTER_H

#include <vector>
#include <cstdint>
#include <iostream>
#include "surf.hpp"
#include "io/FileWriteBuffer.h"
#include "io/FileReadBuffer.h"


class CellFilter {
private:
    // Private constructor to enforce the builder pattern
    explicit CellFilter(std::vector<std::string> values);
    explicit CellFilter(std::unique_ptr<surf::SuRF> filter);
    std::unique_ptr<surf::SuRF> filter;

public:
    // Builder class to construct the CellFilter object
    class Builder {
    private:
        std::vector<std::string> values;

    public:
        //void insert(S2CellId& value);
        CellFilter build();

        void insertMany(const S2CellUnion &insertValues);
    };

    // Static methods for serialization and deserialization
    std::pair<uint64_t, uint64_t> serialize(FileWriteBuffer& f);
    static CellFilter deserialize(FileReadBuffer& f, uint64_t pos, uint64_t size);

};

#endif // CELL_FILTER_H


#endif //ROARINGGEOMAPS_CELLFILTER_H
