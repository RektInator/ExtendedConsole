/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Base64 encoding and decoding of strings.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Base64
{
    namespace Internal
    {
        static constexpr char Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        static constexpr char Reversetable[128] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
        };
    }

    inline std::string Encode(std::string_view Input)
    {
        std::string Result((((Input.size() + 2) / 3) * 4), '=');
        size_t Outputposition = 0;
        uint32_t Accumulator = 0;
        int32_t Bits = 0;

        for (auto &Item : Input)
        {
            Accumulator = (Accumulator << 8) | (Item & 0xFF);
            Bits += 8;
            while (Bits >= 6)
            {
                Bits -= 6;
                Result[Outputposition++] = Internal::Table[(Accumulator >> Bits) & 0x3F];
            }
        }

        if (Bits)
        {
            Accumulator <<= 6 - Bits;
            Result[Outputposition++] = Internal::Table[Accumulator & 0x3F];
        }

        return Result;
    }
    inline std::string Decode(std::string_view Input)
    {
        std::string Result;
        uint32_t Accumulator = 0;
        int32_t Bits = 0;

        for (auto &Item : Input)
        {
            if (Item == '=') continue;

            Accumulator = (Accumulator << 6) | Internal::Reversetable[uint8_t(Item)];
            Bits += 6;

            if (Bits >= 8)
            {
                Bits -= 8;
                Result += char((Accumulator >> Bits) & 0xFF);
            }
        }

        return Result;
    }
}
