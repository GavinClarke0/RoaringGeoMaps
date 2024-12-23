#include "CellFilter.h"
#include "surf.hpp"

CellFilter::CellFilter(const std::vector<std::string>& values):filter(new surf::SuRF(values)) {}

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

bool CellFilter::containsRange(uint64_t minCellId, uint64_t maxCellId) {
    return filter->lookupRange(surf::uint64ToString(minCellId), true, surf::uint64ToString(maxCellId), true);
}
