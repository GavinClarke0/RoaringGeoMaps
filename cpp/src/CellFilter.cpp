#include "CellFilter.h"
#include "s2/s2cell_union.h"

CellFilter::CellFilter(std::vector<std::string> values):
filter(std::make_unique<surf::SuRF>(values)) {
}

//void CellFilter::Builder::insert(S2CellId& value) {
//    values.push_back(surf::uint64ToString(value.id()));
//}

CellFilter CellFilter::Builder::build() {
    return CellFilter(std::move(values));
}

void CellFilter::Builder::insertMany(const S2CellUnion&  insertValues) {
    std::transform(insertValues.begin(), insertValues.end(), std::back_inserter(values), [](S2CellId value) {
        return surf::uint64ToString(value.id());
    });
}

std::pair<uint64_t, uint64_t>  CellFilter::serialize(FileWriteBuffer& f) {
    return f.write(filter->serialize(), filter->serializedSize());
}

CellFilter::CellFilter(std::unique_ptr<surf::SuRF> filter): filter(std::move(filter)) {}

CellFilter CellFilter::deserialize(FileReadBuffer& f, uint64_t pos, uint64_t size) {
    return CellFilter(std::unique_ptr<surf::SuRF>(surf::SuRF::deSerialize(const_cast<char*>(f.view(pos, size)))));
}