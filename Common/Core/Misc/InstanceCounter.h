#pragma once
#include <iostream>
#include <format>
#include <atomic>
#include <source_location>
#include <string_view>
#include "Core/Log/Logging.h"
#include "Macros.h"

// Deducing type at compile-time based on https://stackoverflow.com/a/68523970
template <typename T>
consteval std::string_view func_name() {
    return std::source_location::current().function_name();
}

template <typename T>
consteval std::string_view type_of_impl_() {
    constexpr std::string_view functionName = func_name<T>();

    constexpr std::string_view pattern = "T = ";
    size_t start = functionName.find(pattern) + pattern.size();
    size_t end = functionName.find(']', start);

    return functionName.substr(start, end - start);
}

template <typename T>
constexpr auto type_of() {
    return type_of_impl_<T>();
}

// Unique names for InstanceCounters.
struct CounterTag {
    const char* name;
    const char* file;
    unsigned int line;
    const char* function;

    constexpr bool operator==(const CounterTag& other) const noexcept {
        return name == other.name && file == other.file && line == other.line && function == other.function;
    }
};

namespace {
    template <typename T>
    consteval CounterTag MakeCounterTag(std::source_location loc = std::source_location::current()) {
        return { type_of<T>().data(), loc.file_name(), loc.line(), loc.function_name() };
    }

    consteval CounterTag MakeCounterTag(std::string_view name, std::source_location loc = std::source_location::current()) {
        return { name.data(), loc.file_name(), loc.line(), "N/A"};
    }
}

template <typename T>
struct DefaultCounterTag {
    static constexpr auto tag = MakeCounterTag<T>();
};

// Inspired by https://tristanbrindle.com/posts/beware-copies-initializer-list

/*
* A helper class for counting invocations of ctors and copies. Should be thread safe.
* Usage: Counted<MyType>; or using the ENABLE_INSTANCE_COUNTING macro.
*/
template <typename T, typename Tag = DefaultCounterTag<T>>
struct InstanceCounter {
    InstanceCounter() noexcept { ++icounter.numConstruct; }

    template <typename U = T, std::enable_if_t<std::is_copy_constructible_v<U>, int> = 0>
    InstanceCounter(const InstanceCounter&) noexcept { ++icounter.numCopy; }

    template <typename U = T, std::enable_if_t<std::is_move_constructible_v<U>, int> = 0>
    InstanceCounter(InstanceCounter&&) noexcept { ++icounter.numMove; }

    InstanceCounter& operator=(InstanceCounter) noexcept { return *this; }
    ~InstanceCounter() { ++icounter.numDestruct; }

private:
    struct Counter {
        std::atomic<int> numConstruct = 0;
        std::atomic<int> numCopy = 0;
        std::atomic<int> numMove = 0;
        std::atomic<int> numDestruct = 0;

        ~Counter() {
            PrintReport();
        }

        void PrintReport() {
            LOG_DEBUG("[{}:{}:{}] ({}) Stats:", Tag::tag.file, Tag::tag.line, Tag::tag.function, Tag::tag.name);
            LOG_DEBUG("{} direct constructions.", numConstruct.load());
            LOG_DEBUG("{} copies", numCopy.load());
            LOG_DEBUG("{} moves", numMove.load());
            const int total_construct = numConstruct + numCopy + numMove;
            LOG_DEBUG("{} total constructions", total_construct);
            LOG_DEBUG("{} destructions ", numDestruct.load());
            CLOG_WARN(!(total_construct == numDestruct), "Potential leak!");
        }
    };

    inline static Counter icounter;
};

template <typename T>
concept Trackable = std::is_class_v<T>;  // Ensure only class types are used

// Wrapper for the above InstanceCounter class
template <Trackable T>
struct Counted : T, private InstanceCounter<T>
{
    using T::T;
};

/* Macro alternative to be added to class e.g.
   MyClass {
   public:
        ENABLE_INSTANCE_COUNTING(MyClass)
   }
*/
#if NCL_DEBUG
//#define ENABLE_INSTANCE_COUNTING(Type) InstanceCounter<Type, MakeCounterTag(#Type)> XCONCAT(Type, Counter);
#define ENABLE_INSTANCE_COUNTING(Type) // Not Implemented
    /*static constexpr auto XCONCAT(Type, Tag) = MakeCounterTag(XSTR(Type)); \
    InstanceCounter<Type, decltype(XCONCAT(Type, Tag))> XCONCAT(Type, Counter);*/
#else
#define ENABLE_INSTANCE_COUNTING(Name)
#endif