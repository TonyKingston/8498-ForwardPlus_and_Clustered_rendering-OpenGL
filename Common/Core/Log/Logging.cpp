#include "pch.h"
#include "Logging.h"
#include "Resources/Assets.h"
#include "spdlog/sinks/daily_file_sink.h"

namespace NCL {
    std::shared_ptr<spdlog::logger> g_logger;

    // Static initialization of logging module. Will properly replaces this if I get proper engine systems in place.
    struct LoggerSetup {
        LoggerSetup() {
            if (g_logger) {
                // Logger already initialized.
                return;
            }

            const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#if USE_ASYNC_LOGGER
            auto onThreadStart = [](){std::cout << "spdlog thread started: " << std::this_thread::get_id() << std::endl; };
            auto onThreadEnd = [](){std::cout << "spdlog thread ended: " << std::this_thread::get_id() << std::endl; };
            spdlog::init_thread_pool(8192, 1, std::move(onThreadStart), std::move(onThreadEnd));
            g_logger = std::make_shared<spdlog::async_logger>(NCL_NAME, consoleSink, 
                                        spdlog::thread_pool(), spdlog::async_overflow_policy::block);

#else
            g_logger = std::make_shared<spdlog::logger>(NCL_NAME, consoleSink);
#endif
            // [Time] [LogLevel] [Thread id] [LoggerName] [Message]
            g_logger->set_pattern("[%T.%e] [%^%L%$] [thread %t] [%n] %v");

#if NCL_DEBUG
            g_logger->flush_on(spdlog::level::warn);
#else
            g_logger->set_level(spdlog::level::debug);
            g_logger->flush_on(spdlog::level::debug);
#endif

            try {
                const auto logFile = Assets::CONFIGDIR / std::filesystem::path{ NCL_NAME ".log" };
                //std::filesystem::remove(logFile);
                auto now = spdlog::log_clock::now();
                const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFile.string(), 12, 0);
                g_logger->sinks().push_back(fileSink);
            }
            catch (const std::filesystem::filesystem_error& e) {
                g_logger->warn("Unable to use log file: {}", e.what());
            }    

            g_logger->set_error_handler([](const std::string& msg) {
                printf("*** Failed to log to logger %s: %s ***\n", g_logger->name().c_str(), msg.c_str());
            });

            g_logger->info("Running on " NCL_NAME " " NCL_VERSION);
        }

        ~LoggerSetup() {    
            // Needed if using async logger.
            spdlog::shutdown();
        }
    } loggerSetup;
}