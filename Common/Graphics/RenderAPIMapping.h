#pragma once
#include "Macros.h"
#include "Core/Log/Logging.h"
#include "Core/Misc/TypeUtils.h"

#include <array>
#include <utility>
#include <type_traits>
#include <tuple>
#include <ranges>

// Here we define a utilties for Mapping between graphics API types and our own defined enums
// E.g. TEX_2D -> GL_TEXTURE_2D

// Graphics API tags for tag dispatching
// Perhaps these should be declared elsewhere?
struct OGLTag {};
struct VulkanTag {};

// Compile-time Map for lookup
template <typename K, typename V, size_t N>
struct ConstMap {
    std::array<std::pair<K, V>, N> data;

    template <class... Entries>
    constexpr ConstMap(Entries&&...entries) :
        data{ std::forward<Entries>(entries)... } {}

    // Our enum to Backend API
    constexpr V At(K key) const {
        auto it = std::ranges::find_if(data, [key](const auto& pair) {
            return pair.first == key;
        });

        if (it != data.end()) {
            return it->second;
        }

        static_assert(AlwaysFalse<T>, "Key not in map");
    }

    // Backend API to our Enum
    constexpr K Reverse(V value) const {
        auto it = std::ranges::find_if(data, [value](const auto& pair) {
            return pair.second == value;
        });

        if (it != data.end()) {
            return it->first;
        }

        static_assert(AlwaysFalse<T>, "Key not in map");
    }

    constexpr bool Contains(K key) const {
        // TODO: C++23
        return std::any_of(data.begin, data.end(), [key](const auto& pair) {
            return pair.first == key;
        });
    }
};

template <typename K, typename V, typename... Entries>
constexpr auto CreateConstMap(Entries &&...entry) {
    return ConstMap<K, V, sizeof...(entry)>{entry...};
}

template <typename Backend, typename Enum>
struct EnumMapping {
    static constexpr auto Map(Enum) {
        static_assert(sizeof(Enum) == 0, "EnumMapping specialization missing for this backend and enum type.");
    }
};

template <typename Backend, typename Enum>
concept HasEnumMapping = requires(Enum e) {
    { EnumMapping<Backend, Enum>::Map(e) };
};

// This will dispatch to the right API function.
template <typename Backend, typename Enum>
requires std::is_enum_v<Enum> && HasEnumMapping<Backend, Enum>
constexpr auto EnumToAPI(Enum value) {
    return EnumMapping<Backend, Enum>::Map(value);
}

#define MAP(KEY, VALUE) std::pair{ KEY, VALUE }

// Helper macro to slightly reduce verbosity.
#define DEFINE_ENUM_MAPPING(API_TAG, ENUM_TYPE, BACKEND_TYPE, ...)                   \
template <>                                                                          \
struct EnumMapping<API_TAG, ENUM_TYPE> {                                             \
    static constexpr ConstMap<ENUM_TYPE, BACKEND_TYPE,                          \
        std::tuple_size_v<decltype(std::make_tuple(__VA_ARGS__))>> values {        \
        __VA_ARGS__                                                                  \
    };                                                                              \
                                                                                     \
    static constexpr BACKEND_TYPE Map(ENUM_TYPE e) {                                 \
        return values.At(e);                                                         \
    }                                                                                \
};