#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h" // For `bool`
#include "stddef.h"  // For `size_t`
#include "stdint.h" // For standard integer types

// Opaque pointer to the RoaringGeoMapWriter class
typedef struct RoaringGeoMapWriter RoaringGeoMapWriter;

// Create a new RoaringGeoMapWriter instance
RoaringGeoMapWriter* RoaringGeoMapWriter_New(int levelIndexBucketRange);

// Destroy a RoaringGeoMapWriter instance
void RoaringGeoMapWriter_Delete(RoaringGeoMapWriter* writer);

// Write a region and its description
// Returns true on success, false on failure (e.g., if the description is too long).
bool RoaringGeoMapWriter_Write(RoaringGeoMapWriter* writer, const uint64_t* cellIds, uint64_t cellIdsCount, const char* key);

// Build the RoaringGeoMap and write it to a file
// Returns true on success, false on failure.
bool RoaringGeoMapWriter_Build(RoaringGeoMapWriter* writer, const char* filePath);

#ifdef __cplusplus
}
#endif
