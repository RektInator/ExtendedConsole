/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Fast but not actually cryptographically secure hashing.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Hash
{
    namespace Internal
    {
        constexpr uint32_t FNV1_Prime_32 = 16777619u;
        constexpr uint64_t FNV1_Prime_64 = 1099511628211u;
        constexpr uint32_t FNV1_Offset_32 = 2166136261u;
        constexpr uint64_t FNV1_Offset_64 = 14695981039346656037u;
    }

    constexpr uint32_t FNV1_32(const char *String, uint32_t LastValue = Internal::FNV1_Offset_32)
    {
        return *String ? FNV1_32(String + 1, (LastValue * Internal::FNV1_Prime_32) ^ *String) : LastValue;
    };
    constexpr uint64_t FNV1_64(const char *String, uint64_t LastValue = Internal::FNV1_Offset_64)
    {
        return *String ? FNV1_64(String + 1, (LastValue * Internal::FNV1_Prime_64) ^ *String) : LastValue;
    };
    constexpr uint32_t FNV1a_32(const char *String, uint32_t LastValue = Internal::FNV1_Offset_32)
    {
        return *String ? FNV1a_32(String + 1, (*String ^ LastValue) * Internal::FNV1_Prime_32) : LastValue;
    };
    constexpr uint64_t FNV1a_64(const char *String, uint64_t LastValue = Internal::FNV1_Offset_64)
    {
        return *String ? FNV1a_64(String + 1, (*String ^ LastValue) * Internal::FNV1_Prime_64) : LastValue;
    };
}
