#include "../include/roaring_geo_map_reader.h"
#include "RoaringGeoMapReader.h" // The actual C++ class header
#include <vector>
#include <cstring> // For memcpy

extern "C" {

// Wrapper for creating a new RoaringGeoMapReader instance
RoaringGeoMapReader* RoaringGeoMapReader_New(const char* filePath) {
    try {
        return reinterpret_cast<RoaringGeoMapReader*>(new RoaringGeoMapReader(filePath));
    } catch (...) {
        return nullptr; // Return nullptr on failure
    }
}

// Wrapper for deleting a RoaringGeoMapReader instance
void RoaringGeoMapReader_Delete(RoaringGeoMapReader* reader) {
    delete reinterpret_cast<RoaringGeoMapReader*>(reader);
}

// Wrapper for the Contains method
int RoaringGeoMapReader_Contains(RoaringGeoMapReader* reader, const uint64_t* cellIds, uint64_t cellIdsCount, char** resultBuffer, uint64_t* resultSize) {
    if (!reader || !cellIds || !resultBuffer || !resultSize) {
        return -1; // Error: Invalid arguments
    }

    try {
        auto cppReader = reinterpret_cast<RoaringGeoMapReader*>(reader);

        // Convert cellIds to S2CellUnion (assumes S2CellUnion can be constructed this way)
        S2CellUnion cellUnion;
        cellUnion.Init(std::vector<uint64_t>(cellIds, cellIds + cellIdsCount));

        // Call the C++ method
        auto result = cppReader->Contains(cellUnion);

        // Serialize results into a flat buffer
        size_t totalSize = 0;
        for (const auto& vec : result) {
            totalSize += vec.size();
        }

        *resultBuffer = new char[totalSize];
        *resultSize = totalSize;

        char* writePtr = *resultBuffer;
        for (const auto& vec : result) {
            memcpy(writePtr, vec.data(), vec.size());
            writePtr += vec.size();
        }

        return 0; // Success
    } catch (...) {
        return -1; // Error
    }
}

// Wrapper for the Intersects method (similar to Contains)
int RoaringGeoMapReader_Intersects(RoaringGeoMapReader* reader, const uint64_t* cellIds, uint64_t cellIdsCount, char** resultBuffer, uint64_t* resultSize) {
    if (!reader || !cellIds || !resultBuffer || !resultSize) {
        return -1; // Error: Invalid arguments
    }

    try {
        auto cppReader = reinterpret_cast<RoaringGeoMapReader*>(reader);

        // Convert cellIds to S2CellUnion
        S2CellUnion cellUnion;
        cellUnion.Init(std::vector<uint64_t>(cellIds, cellIds + cellIdsCount));

        // Call the C++ method
        auto result = cppReader->Intersects(cellUnion);

        // Serialize results into a flat buffer
        size_t totalSize = 0;
        for (const auto& vec : result) {
            totalSize += vec.size();
        }

        *resultBuffer = new char[totalSize];
        *resultSize = totalSize;

        char* writePtr = *resultBuffer;
        for (const auto& vec : result) {
            memcpy(writePtr, vec.data(), vec.size());
            writePtr += vec.size();
        }

        return 0; // Success
    } catch (...) {
        return -1; // Error
    }
}

}
