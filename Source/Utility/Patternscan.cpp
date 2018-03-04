/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 14-01-2018
    License: MIT
    Notes:
        Provides a simple scanner for the host image.
        Should lookinto SSE 4.2 for faster scanning.
*/

#include "../Stdinclude.hpp"

// GCC exported.
#if !defined(_WIN32)
extern char _etext, _end;
#endif

namespace Pattern
{
    // Predefined ranges.
    Range_t Textsegment{};
    Range_t Datasegment{};

    // Find a single pattern in the range.
    size_t _Findpattern(Range_t &Range, std::vector<uint8_t> Pattern, std::vector<uint8_t> Mask)
    {
        uint8_t Firstbyte = Pattern[0];
        auto Base = (const uint8_t *)Range.first;
        size_t Count = Range.second - Range.first - Pattern.size();

        // Inline compare.
        auto Compare = [&](const uint8_t *Address) -> bool
        {
            size_t Patternlength = Pattern.size();
            for (size_t i = 0; i < Patternlength; ++i)
            {
                if (Mask[i] == '\x01' && Address[i] != Pattern[i])
                    return false;
            }
            return true;
        };

        // We could do a runtime check for Pattern < Mask.
        assert(Pattern.size() == Mask.size());

        // Iterate over the range.
        for (size_t Index = 0; Index < Count; ++Index)
        {
            if (likely(Base[Index] != Firstbyte))
                continue;

            if (Compare(Base + Index))
                return Range.first + Index;
        }

        return 0;
    }

    // Scan until the end of the range and return all result.
    std::vector<size_t> Findpatterns(Range_t &Range, std::vector<uint8_t> Pattern, std::vector<uint8_t> Mask)
    {
        std::vector<std::size_t> Results;
        Range_t Localrange = Range;
        size_t Lastresult = 0;

        while (true)
        {
            Lastresult = _Findpattern(Localrange, Pattern, Mask);

            if (Lastresult == 0)
                break;

            Localrange.first = Lastresult + 1;
            Results.push_back(Lastresult);
        }

        return std::move(Results);
    }

    // Create a pattern or mask from a readable string.
    std::vector<uint8_t> Stringtopattern(std::string Humanreadable)
    {
        std::vector<uint8_t> Result;

        for (auto Iterator = Humanreadable.c_str(); *Iterator; ++Iterator)
        {
            if (*Iterator == ' ') continue;
            if (*Iterator == '?') Result.push_back('\x00');
            else
            {
                Result.push_back(uint8_t(strtoul(std::string(Iterator, 2).c_str(), nullptr, 16)));
                ++Iterator;
            }
        }

        return std::move(Result);
    }
    std::vector<uint8_t> Stringtomask(std::string Humanreadable)
    {
        std::vector<uint8_t> Result;

        for (auto Iterator = Humanreadable.c_str(); *Iterator; ++Iterator)
        {
            if (*Iterator == ' ') continue;
            if (*Iterator == '?') Result.push_back('\x00');
            else
            {
                Result.push_back('\x01');
                ++Iterator;
            }
        }

        return std::move(Result);
    }

    // Fetch the segment information on startup.
    void Updatesegments()
    {
        /*
            NOTE(Convery):
            While the GCC exports defined above should be available on
            OSX, I can not confirm that they actually do exist.
            This code may be preferable:

            #include <mach-o/getsect.h>

            Textsegment.second = size_t(get_etext());
            Datasegment.first = size_t(get_etext());
            Datasegment.second = size_t(get_end());
        */

    #if defined(_WIN32)
        HMODULE Module = GetModuleHandleA(NULL);
        if (!Module) return;

        PIMAGE_DOS_HEADER DOSHeader = (PIMAGE_DOS_HEADER)Module;
        PIMAGE_NT_HEADERS NTHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)Module + DOSHeader->e_lfanew);

        Textsegment.first = size_t(Module) + NTHeader->OptionalHeader.BaseOfCode;
        Textsegment.second = Textsegment.first + NTHeader->OptionalHeader.SizeOfCode;

        Datasegment.first = Textsegment.second;
        Datasegment.second = Datasegment.first + NTHeader->OptionalHeader.SizeOfInitializedData;
    #else
        Textsegment.first = *(size_t *)dlopen(NULL, RTLD_LAZY);
        Textsegment.second = size_t(&_etext);
        Datasegment.first = size_t(&_etext);
        Datasegment.second = size_t(&_end);
    #endif
    }
    struct Startup { Startup() { Updatesegments(); }; };
    static Startup Updater{};
}
