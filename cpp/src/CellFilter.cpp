#include "CellFilter.h"
#include "surf.hpp"

CellFilter::CellFilter(const std::vector<std::string> &values)
        : filter(new surf::SuRF(values)) {}

CellFilter::CellFilter() = default;

CellFilter::CellFilter(surf::SuRF *filter) : filter(filter) {
}

CellFilter::~CellFilter() {
    delete filter;
}

CellFilter CellFilter::Builder::build() {
    std::sort(values.begin(), values.end());
    return CellFilter(values);
}

void CellFilter::Builder::insertMany(const std::vector<S2CellId> &insertValues) {
    std::transform(insertValues.begin(), insertValues.end(), std::back_inserter(values),
                   [](const auto &cellId) {
                       return surf::uint64ToString(cellId.id());
                   });
}

std::pair<uint64_t, uint64_t> CellFilter::serialize(FileWriteBuffer &f) {
    return f.write(filter->serialize(), filter->serializedSize());
}

CellFilter CellFilter::deserialize(FileReadBuffer &f, uint64_t pos, uint64_t size) {
    return CellFilter(surf::SuRF::deSerialize(const_cast<char *>(f.view(pos, size))));;
}

bool CellFilter::contains(uint64_t cellId) {
    return filter->lookupKey(surf::uint64ToString(cellId));
}

std::tuple<uint64_t, uint64_t, bool> CellFilter::containsRange(uint64_t minCellId, uint64_t maxCellId) {
    auto min = filter->moveToKeyGreaterThan(surf::uint64ToString(minCellId), true);
    auto max = filter->moveToKeyLessThan(surf::uint64ToString(maxCellId), true);

    auto minVal = surf::stringToUint64(min.getKey());
    auto maxVal = surf::stringToUint64(max.getKey());
    // If max is greater, then min then the value is not present in the filter.
    if (minVal < maxVal) {
        return {minVal, maxVal, true};
    }
    return {0, 0, false};
}
