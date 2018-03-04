/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Provides a single include-file for all modules.
*/

#pragma once

// The configuration settings.
#include "Configuration/Defines.hpp"
#include "Configuration/Macros.hpp"

// Standard libraries.
#include <unordered_map>
#include <string_view>
#include <functional>
#include <assert.h>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <string>
#include <mutex>
#include <ctime>

// Platformspecific libraries.
#if defined(_WIN32)
    #include <Windows.h>
    #include <direct.h>
    #include <intrin.h>
    #undef min
    #undef max
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/mman.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <dlfcn.h>
#endif

// Utility modules.
#include "Utility/Variadicstring.hpp"
#include "Utility/Patternscan.hpp"
#include "Utility/Filesystem.hpp"
#include "Utility/Memprotect.hpp"
#include "Utility/Bytebuffer.hpp"
#include "Utility/PackageFS.hpp"
#include "Utility/FNV1Hash.hpp"
#include "Utility/Hooking.hpp"
#include "Utility/Logfile.hpp"
#include "Utility/Base64.hpp"
