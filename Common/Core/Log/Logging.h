#pragma once
#include "Build.h"
#include <spdlog/logger.h>

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

#define NCL_FATAL(...) \
    do { \
        LOG_CRITICAL(__VA_ARGS__); \
        NCL_DEBUG_BREAK(); \
        ::std::abort(); \
    } while(0)

namespace NCL {
    
    extern std::shared_ptr<spdlog::logger> g_logger;
    
} // namespace NCL