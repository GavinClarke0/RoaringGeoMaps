#ifndef ROARINGGEOMAPS_ENDIAN_H
#define ROARINGGEOMAPS_ENDIAN_H

#include <cstdint>
#include <stdexcept>
#include <bit>
#include <cstdint>
#include <ranges>

// copy in byteswap from cpp 23 for now.
template<std::integral T>
constexpr T byteswap(T value) noexcept
{
static_assert(std::has_unique_object_representations_v<T>,
              "T may not have padding bits");
auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
std::ranges::reverse(value_representation);
return std::bit_cast<T>(value_representation);
}

template<std::integral T>
constexpr T littleEndian(T value) noexcept
{
    if constexpr (std::endian::native == std::endian::big) {
        value = byteswap(value);
    }
    return value;
}

#endif //ROARINGGEOMAPS_ENDIAN_H



