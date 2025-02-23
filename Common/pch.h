#pragma once
#pragma message("Compiling precompiled headers.\n")

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
//#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>

#undef FAR
#undef NEAR
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cassert>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <source_location>

#include <spdlog/spdlog.h>	
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>	

#include <Common.h>

//#if !defined(NDEBUG) && !defined(_DEBUG)
//#define _DEBUG
//#endif

namespace NCL {

}