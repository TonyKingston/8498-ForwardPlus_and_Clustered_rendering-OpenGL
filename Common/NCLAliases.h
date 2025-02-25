#pragma once
#include <cstdint>
#include <type_traits>

// Include common aliases here.
namespace NCL {

#define TYPE_ALIASES_XMACRO \
    X(int8, int8_t, 1)   \
    X(int16, int16_t, 2) \
    X(int32, int32_t, 4) \
    X(int64, int64_t, 8) \
    X(uint8, uint8_t, 1) \
    X(uint16, uint16_t, 2) \
    X(uint32, uint32_t, 4) \
    X(uint64, uint64_t, 8) \
    X(uint, unsigned int, 4)

#define X(alias, type, size) \
    using alias = type; \
    static_assert(sizeof(alias) == size, #alias " expected size does not match"); \
    static_assert(std::is_same_v<alias, type>, #alias " types do not match");

    TYPE_ALIASES_XMACRO

#undef X
}