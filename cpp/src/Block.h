#ifndef ROARINGGEOMAPS_BLOCK_H
#define ROARINGGEOMAPS_BLOCK_H

#include "io/FileReadBuffer.h"
#include "WriteHelpers.h"
#include "VectorView.h"

uint64_t determineBlocks(uint32_t blockSize, uint32_t totalEntries);

template<typename T>
class BlockWriter {
public:
    explicit BlockWriter(uint64_t blockSize): blockSize(blockSize) {};

    std::pair<uint64_t, T> WriteBlock(FileWriteBuffer& f) {
        uint64_t valueDataSize = 0;
        for (uint64_t size : valueSizes) {
            valueDataSize += size;
            writeLittleEndianUint64(f, valueDataSize);
        }
        for (T value : values) {
            writeValue(f, value);
        }
        return {(values.size() * sizeof(uint64_t)) + valueDataSize, values.back()}; // Returns size of block and largest value in block;
    };

    std::pair<uint64_t, T> WriteBlockZstdCompressed(FileWriteBuffer& f) {
        return {};
    }; // Returns offset of block, largest value in block, and values in block

    bool insertValue(T value, uint64_t size) {
        // Reject write if block is full
        if (values.size() >= blockSize)
            return false;
        valueSizes.emplace_back(size);
        values.emplace_back(value);
        return true;
    };

    // Must be overridden by implementing class
    virtual void writeValue(FileWriteBuffer& f, T value) {
        auto x = 10;
    };
private:
    uint64_t blockSize;
    std::vector<uint64_t> valueSizes;
    std::vector<T> values;
};

template<typename T>
class FixedBlockWriter {
public:
    explicit FixedBlockWriter(uint64_t blockSize): blockSize(blockSize) {}

    std::pair<uint64_t, T> WriteBlock(FileWriteBuffer& f) {
        for (uint64_t value : values) {
            writeValue(f, value);
        }
        return {values.size() * sizeof(T), values.back()}; // Returns offset of block, largest value in block, and values in block;
    };

    std::pair<uint64_t, T> WriteBlockZstdCompressed(FileWriteBuffer& f) {
        return {};
    }; // Returns offset of block;

    // Must be overridden by implementing class
    bool insertValue(T value) {
        // Reject write if block is full
        if (values.size() >= blockSize)
            return false;
        values.emplace_back(value);
        return true;
    };
    // Must be overridden by implementing class
    virtual void writeValue(FileWriteBuffer& f, T value) {};

private:
    uint64_t blockSize;
    std::vector<T> values;
};


template<typename T>
class BlockReader {
public:
    explicit BlockReader(FileReadBuffer& f, uint64_t position, uint64_t size, uint64_t entries):
    f(f),
    position(position),
    valuePosition(position + (entries * sizeof(uint64_t))), // start position of the values.
    size(size),
    entries(entries),
    offsets(VectorView<uint64_t>(f, position, entries)){};

    std::vector<T> readIndexes(const std::vector<uint32_t>& indexes) {
        // TODO: validate it is not out of bounds
        std::vector<T> valueRefs;
        for (auto index: indexes) {
            uint64_t offset = index == 0 ? 0: offsets[index-1];
            auto pos = valuePosition + offset;
            std::integral auto valueSize = index == 0 ? offsets[index]: offsets[index] - offsets[index-1];
            valueRefs.emplace_back(readValue(f, pos, valueSize));
        }
        return valueRefs;
    };
    // Must be overridden by implementing class
    virtual T readValue(FileReadBuffer& f, uint64_t position, uint64_t size) {
        return {};
    };

private:
    FileReadBuffer& f;
    uint64_t position;
    uint64_t valuePosition;
    uint64_t size;
    uint64_t entries;
    VectorView<uint64_t> offsets;

};

template<std::integral T>
class FixedBlockReader {
public:
    explicit FixedBlockReader(FileReadBuffer& f, uint64_t position, uint64_t size, uint64_t entries): f(f), position(position), size(size), entries(entries),
    values(VectorView<uint64_t>(f, position, entries)){};

    std::vector<uint32_t> queryValueIndexes(const std::vector<T>& queryValues) {
        // TODO: validate it is not out of bounds
        std::vector<uint32_t> indexes;
        for (auto value: queryValues) {
            // todo use bitmask and vector instructions to find this faster. We likely can improve this with a modified
            // binary search that does not start from the beginning each time.
            auto it = std::lower_bound(values.begin(), values.end(), value); // bin

            // In theory, we should never be reading a block for a value we do not know is there.
            if (it != values.end()) {
                indexes.emplace_back(std::distance(values.begin(), it));
            }
        }
        return indexes;
    };
private:
    FileReadBuffer& f;
    uint64_t position;
    uint64_t size;
    uint64_t entries;
    VectorView<T> values;
};


#endif //ROARINGGEOMAPS_BLOCK_H
