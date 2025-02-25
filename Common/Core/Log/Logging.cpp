#include "pch.h"
#include "Logging.h"
#include "Resources/Assets.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <source_location>
#ifdef _MSC_VER
#include "spdlog/sinks/msvc_sink.h"
//#include "spdlog/sinks/vs_sink.h"
#endif

namespace NCL {
    std::shared_ptr<spdlog::logger> g_logger;

    // Copied from spdlog as it's private
    tm now_tm(spdlog::log_clock::time_point tp) {
        time_t tnow = spdlog::log_clock::to_time_t(tp);
        return spdlog::details::os::localtime(tnow);
    }

    // Example: for log NCL_2025-02-22_1.log, will return the string NCL_2025-02-22_2.log
    std::optional<std::filesystem::path> generate_unique_log_name(const std::string& original_filename) {
        namespace fs = std::filesystem;
        // Remove extension
        const std::string stem = fs::path(original_filename).stem().string();
        // Extract the base filename (NCL_YYYY-MM-DD)
        size_t last_separator = std::max(stem.rfind('_'), stem.rfind('-'));
        if (last_separator == std::string::npos) {
            return {};
        }
        std::string base_name = stem.substr(0, last_separator + 1);
        std::string date_part = stem.substr(last_separator + 1);

        int counter = 1;
        fs::path log_dir = Assets::CONFIGDIR;
        fs::path new_path;
        do {
            new_path = log_dir / fs::path{ std::format("{}{}_{:d}.log", base_name, date_part, counter)};
            counter++;
        } while (fs::exists(new_path));

        return {new_path};
    }

    // Static initialization of logger. Will properly replaces this if I get proper engine systems in place.
    struct LoggerSetup {
        LoggerSetup() {
            if (g_logger) {
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
                
#if !NCL_DEBUG
            g_logger->flush_on(spdlog::level::warn);
#else
            g_logger->set_level(spdlog::level::debug);
            g_logger->flush_on(spdlog::level::debug);
#endif
            // TODO: Probably have custom sync to have a more Unreal-like log system.
            try {
                const auto logFile = Assets::CONFIGDIR / std::filesystem::path{ NCL_NAME ".log" };
                // Calculate what the sink is going to name our file so we can keep a copy of the old one.
                auto now = spdlog::log_clock::now();
                auto calc = spdlog::sinks::daily_filename_calculator{};
                auto filename = calc.calc_filename(logFile.string(), now_tm(spdlog::log_clock::now()));

                // If we have a log file already for the day from a previous run, 
                // copy it with a new name and delete the original.
                if (std::filesystem::exists(filename)) {
                    auto newFilename = generate_unique_log_name(filename);
                    if (newFilename) {
                       std::filesystem::copy(filename, newFilename.value());
                    }
                    std::filesystem::remove(filename);
                }

                const auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFile.string(), 12, 0);
                g_logger->sinks().push_back(fileSink);
            }
            catch (const std::filesystem::filesystem_error& e) {
                g_logger->warn("Unable to use log file: {}", e.what());
            }

#if defined(_MSC_VER) && NCL_DEBUG
            // Log to Debug pane in output window of Visual Studio
            auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>(/*checkDebuggerPresent*/ true);
            g_logger->sinks().push_back(msvcSink);

            // Log to my custom output window pane.
            /*auto vsSink = std::make_shared<spdlog::sinks::vs_sink_mt>();
            g_logger->sinks().push_back(vsSink);*/
#endif
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