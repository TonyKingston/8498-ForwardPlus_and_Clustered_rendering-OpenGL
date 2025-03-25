#pragma once
#include "Build.h"
#include <spdlog/logger.h>
#include <spdlog/fmt/ranges.h>

// TODO: Add option that uses SPDLOG macros so we can get source info in the log e.g. __FUNC__
#define USE_ASYNC_LOGGER 1

// Optionally compile out logs
#define NCL_USE_LOGS 1

#if NCL_USE_LOGS
    #define LOG_DEBUG ::NCL::g_logger->debug
    #define LOG_TRACE ::NCL::g_logger->trace
    #define LOG_INFO ::NCL::g_logger->info
    #define LOG_WARN ::NCL::g_logger->warn
    #define LOG_ERROR ::NCL::g_logger->error
    #define LOG_CRITICAL ::NCL::g_logger->critical
#else
    #define LOG_DEBUG (void)0
    #define LOG_TRACE (void)0
    #define LOG_INFO (void)0
    #define LOG_WARN (void)0
    #define LOG_ERROR (void)0
    #define LOG_CRITICAL (void)0
#endif

// Only log if the condition is met.
#define CLOG_DEBUG(cond, ...) if (cond) LOG_DEBUG(__VA_ARGS__)
#define CLOG_TRACE(cond, ...) if (cond) LOG_TRACE(__VA_ARGS__)
#define CLOG_INFO(cond, ...) if (cond) LOG_INFO(__VA_ARGS__)
#define CLOG_WARN(cond, ...) if (cond) LOG_WARN(__VA_ARGS__)
#define CLOG_ERROR(cond, ...) if (cond) LOG_ERROR(__VA_ARGS__)

#define NCL_FATAL(...) \
    do { \
        LOG_CRITICAL(__VA_ARGS__); \
        NCL_DEBUG_BREAK(); \
        ::std::abort(); \
    } while(0)

#ifndef NCL_ASSERT
    #if NCL_DEBUG
    #define NCL_ASSERT(x) \
            do { \
                if(!static_cast<bool>(x)) [[unlikely]] { \
                    NCL_FATAL("Assertion: {}\n\t{}:{}", XSTR(x), __FILE__, __LINE__); \
                } \
            } while(0)
    #else
    #define NCL_ASSERT(x) (void)(x)
#endif
#endif // NCL_ASSERT

// Promises that an expression is true. In release builds, the compiler can optimise accordingly
#ifndef  NCL_ASSUME
    #ifdef _MSC_VER
        #define NCL_ASSUME(x) (((e) || (NCL_ASSERT(e), (e))), __assume(e))
    #elif defined(__clang__)
        #define NCL_ASSUME(x) (((e) || (NCL_ASSERT(e), (e))), _builtin_assume(e))
    #else
        #define NCL_ASSUME(x) (void)(x)
    #endif
#endif

#define NCL_VERIFY_UNREACHABLE() NCL_ASSERT(false)

namespace NCL {
    //TODO: Should we have macros to declare/define categories like in Unreal
    // Each category would need their own logger.
    extern std::shared_ptr<spdlog::logger> g_logger;

} // namespace NCL