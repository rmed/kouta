#pragma once

#include <cstdint>
#include <climits>

namespace kouta::parsing::sizes
{
    constexpr std::size_t UINT8_BITS{sizeof(std::uint8_t) * CHAR_BIT};
    constexpr std::size_t INT8_BITS{UINT8_BITS};
    constexpr std::size_t UINT16_BITS{sizeof(std::uint16_t) * CHAR_BIT};
    constexpr std::size_t INT16_BITS{UINT16_BITS};
    constexpr std::size_t UINT24_BITS{(sizeof(std::uint16_t) * CHAR_BIT) + UINT8_BITS};
    constexpr std::size_t INT24_BITS{UINT24_BITS};
    constexpr std::size_t UINT32_BITS{sizeof(std::uint32_t) * CHAR_BIT};
    constexpr std::size_t INT32_BITS{UINT32_BITS};
    constexpr std::size_t UINT40_BITS{(sizeof(std::uint32_t) * CHAR_BIT) + UINT8_BITS};
    constexpr std::size_t INT40_BITS{UINT40_BITS};
    constexpr std::size_t UINT48_BITS{(sizeof(std::uint32_t) * CHAR_BIT) + UINT16_BITS};
    constexpr std::size_t INT48_BITS{UINT48_BITS};
    constexpr std::size_t UINT56_BITS{(sizeof(std::uint32_t) * CHAR_BIT) + UINT24_BITS};
    constexpr std::size_t INT56_BITS{UINT56_BITS};
    constexpr std::size_t UINT64_BITS{(sizeof(std::uint64_t) * CHAR_BIT)};
    constexpr std::size_t INT64_BITS{UINT64_BITS};

    constexpr std::size_t FLOAT_BITS{sizeof(float) * CHAR_BIT};
    constexpr std::size_t DOUBLE_BITS{sizeof(double) * CHAR_BIT};
}  // namespace kouta::parsing::sizes
