#ifndef ROARINGGEOMAPS_BLOCKINDEXWRITER_H
#define ROARINGGEOMAPS_BLOCKINDEXWRITER_H

#include <cstdint>
#include "WriteHelpers.h"  // Assuming this contains the write functions
#include "io/FileReadBuffer.h"
#include "unordered_set"
#include "VectorView.h"

template<typename T>
class BlockIndexWriter {
    static_assert(std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value,
                  "BlockIndexReader only supports uint32_t and uint64_t types.");

public:
    BlockIndexWriter() = default;

    void addValue(T value) {
        values.push_back(value);
    }

    uint64_t writeToFile(FileWriteBuffer &f) const {
        uint64_t size = 0;
        for (auto &value: values) {
            size += appendToBuffer(f, value);
        }
        return size;
    }

    uint64_t size() const {
        return values.size() * sizeof(T);
    }

private:
    std::vector<T> values;

    uint64_t appendToBuffer(FileWriteBuffer &f, T value) const;
};

#endif //ROARINGGEOMAPS_BLOCKINDEXWRITER_H
