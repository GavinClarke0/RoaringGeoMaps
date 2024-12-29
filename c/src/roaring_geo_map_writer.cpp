#include "../include/roaring_geo_map_writer.h"
#include "RoaringGeoMapWriter.h" // Include the C++ class header
#include <vector>
#include <cstring> // For strlen

extern "C" {

// Wrapper for creating a new RoaringGeoMapWriter instance
RoaringGeoMapWriter *RoaringGeoMapWriter_New(int levelIndexBucketRange) {
    try {
        return reinterpret_cast<RoaringGeoMapWriter *>(new RoaringGeoMapWriter(levelIndexBucketRange));
    } catch (...) {
        return nullptr; // Return nullptr on failure
    }
}

// Wrapper for deleting a RoaringGeoMapWriter instance
void RoaringGeoMapWriter_Delete(RoaringGeoMapWriter *writer) {
    delete reinterpret_cast<RoaringGeoMapWriter *>(writer);
}

// Wrapper for the write method
bool
RoaringGeoMapWriter_Write(RoaringGeoMapWriter *writer, const uint64_t *cellIds, uint64_t cellIdsCount, const char *key) {
    if (!writer || !cellIds || !key) {
        return false; // Invalid arguments
    }

    try {
        auto cppWriter = reinterpret_cast<RoaringGeoMapWriter *>(writer);

        // Convert the input cell IDs to an S2CellUnion
        S2CellUnion cellUnion;
        cellUnion.Init(std::vector<uint64_t>(cellIds, cellIds + cellIdsCount));

        // Call the write method
        return cppWriter->write(cellUnion, std::string(key));
    } catch (...) {
        return false; // Return false on failure
    }
}

// Wrapper for the build method
bool RoaringGeoMapWriter_Build(RoaringGeoMapWriter *writer, const char *filePath) {
    if (!writer || !filePath) {
        return false; // Invalid arguments
    }

    try {
        auto cppWriter = reinterpret_cast<RoaringGeoMapWriter *>(writer);
        return cppWriter->build(std::string(filePath));
    } catch (...) {
        return false; // Return false on failure
    }
}

}
