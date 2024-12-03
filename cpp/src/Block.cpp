#include <cstdint>
#include "Block.h"

uint64_t determineBlocks(uint32_t blockSize, uint32_t totalEntries) {
    return totalEntries % blockSize > 0 ? (totalEntries / blockSize) + 1 : totalEntries / blockSize;
}
