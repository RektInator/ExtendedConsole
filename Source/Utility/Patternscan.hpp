/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 14-01-2018
    License: MIT
    Notes:
        Provides a simple scanner for the host image.
        Should lookinto SSE 4.2 for faster scanning.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Pattern
{
    using Range_t = std::pair<size_t, size_t>;
    extern Range_t Textsegment;
    extern Range_t Datasegment;

    // Find a single pattern in the range.
    size_t _Findpattern(Range_t &Range, std::vector<uint8_t> Pattern, std::vector<uint8_t> Mask);

    // Scan until the end of the range and return all result.
    std::vector<size_t> Findpatterns(Range_t &Range, std::vector<uint8_t> Pattern, std::vector<uint8_t> Mask);

    // Create a pattern or mask from a readable string.
    std::vector<uint8_t> Stringtopattern(std::string Humanreadable);
    std::vector<uint8_t> Stringtomask(std::string Humanreadable);
}
