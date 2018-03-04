/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Provides a global configuration, defaults.
*/

#pragma once
#pragma warning(disable: 4100 4307)

// Identifying name for this module.
#define MODULENAME "ExtendedConsole"
#define MODULEEXTENSION "Ayria"

// Fixup for Visual Studio 2015 no longer defining this.
#if !defined(_DEBUG) && !defined(NDEBUG)
#define NDEBUG
#endif

// Platform identification.
#if defined(_MSC_VER)
    #define EXPORT_ATTR __declspec(dllexport)
    #define IMPORT_ATTR __declspec(dllimport)
#elif defined(__GNUC__)
    #define EXPORT_ATTR __attribute__((visibility("default")))
    #define IMPORT_ATTR
#else
    #define EXPORT_ATTR
    #define IMPORT_ATTR
    #error Compiling for unknown platform.
#endif

// Environment identification.
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)
    #define ENVIRONMENT64
#endif

// Remove some Windows annoyance.
#if defined(_WIN32)
    #define _CRT_SECURE_NO_WARNINGS
#endif
