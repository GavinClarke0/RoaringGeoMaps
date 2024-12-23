#include "CellFilter.h"
#include "surf.hpp"

CellFilter::CellFilter(const std::vector<std::string>& values)
:filter(new surf::SuRF(values, true, 16, surf::SuffixType::kNone, 0, 0))
{}

CellFilter::CellFilter() = default;

CellFilter::CellFilter(surf::SuRF* filter): filter(filter) {}

CellFilter::~CellFilter() {
   //	filter_->destroy();
   //	delete filter_;
}

//void CellFilter::Builder::insert(S2CellId& value) {
//    values.push_back(surf::uint64ToString(value.id()));
//}

CellFilter CellFilter::Builder::build() {
    return CellFilter( std::vector<std::string>(values.begin(), values.end()));
}

void CellFilter::Builder::insertMany( const std::vector<S2CellId>&  insertValues) {
    for (auto cellId: insertValues) {
        values.insert( surf::uint64ToString(cellId.id()));
    }
}

std::pair<uint64_t, uint64_t>  CellFilter::serialize(FileWriteBuffer& f) {
    return f.write(filter->serialize(), filter->serializedSize());
}

CellFilter CellFilter::deserialize(FileReadBuffer& f, uint64_t pos, uint64_t size) {
    auto filter =  CellFilter(surf::SuRF::deSerialize(const_cast<char*>(f.view(pos, size))));
    return filter;
}

bool CellFilter::contains(uint64_t cellId) {
    return filter->lookupKey(surf::uint64ToString(cellId));
}

std::tuple<uint64_t, uint64_t, bool> CellFilter::containsRange(uint64_t minCellId, uint64_t maxCellId) {
    auto min = filter->moveToKeyGreaterThan(surf::uint64ToString(minCellId), true);
    auto max = filter->moveToKeyLessThan(surf::uint64ToString(maxCellId), true);

    auto minVal = surf::stringToUint64(min.getKey());
    auto maxVal = surf::stringToUint64(max.getKey());
    // If max is greater thqn min then the value is not present in the filter
    if (minVal < maxVal) {
        return {minVal, maxVal, true};
    }
//    if (min.isValid() && max.isValid())
//        return {minVal, maxVal, true};

    return {0, 0, false};
}
