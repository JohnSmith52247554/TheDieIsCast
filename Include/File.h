#pragma once

#include "Config.h"

#include <filesystem>

#ifdef WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <unistd.h>
#include <limits.h>
#endif

namespace File 
{
    namespace fs = std::filesystem;

    fs::path getExecutableDir();
    fs::path getResourceDirectory();
}