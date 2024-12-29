#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h" // For standard integer types

// Opaque pointer to the RoaringGeoMapReader class
typedef struct RoaringGeoMapReader RoaringGeoMapReader;

// Create a new RoaringGeoMapReader instance
RoaringGeoMapReader* RoaringGeoMapReader_New(const char* filePath);

// Destroy a RoaringGeoMapReader instance
void RoaringGeoMapReader_Delete(RoaringGeoMapReader* reader);

// Perform the "Contains" query
// The results will be a 2D array of data (serialized into a flat buffer).
// Returns the size of the results or -1 in case of an error.
int RoaringGeoMapReader_Contains(RoaringGeoMapReader* reader, const uint64_t* cellIds, uint64_t cellIdsCount, char** resultBuffer, uint64_t* resultSize);

// Perform the "Intersects" query
// Similar semantics as Contains.
int RoaringGeoMapReader_Intersects(RoaringGeoMapReader* reader, const uint64_t* cellIds, uint64_t cellIdsCount, char** resultBuffer, uint64_t* resultSize);

#ifdef __cplusplus
}
#endif
