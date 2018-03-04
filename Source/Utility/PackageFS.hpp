/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Plugins should keep all their files in their archive.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Package
{
    using Archivehandle = void *;

    // Operations on the default archive.
    std::string Read(std::string Filename);
    void Write(std::string Filename, std::string &Buffer);
    std::vector<std::string> Findfiles(std::string Criteria);
    bool Exists(std::string Filename);
    void Delete(std::string Filename);

    // Operations on a specific archive.
    Archivehandle Loadarchive(std::string Filename);
    void Savearchive(Archivehandle &Handle, std::string Filename);
    std::string Read(Archivehandle &Handle, std::string Filename);
    void Write(Archivehandle &Handle, std::string Filename, std::string &Buffer);
    std::vector<std::string> Findfiles(Archivehandle &Handle, std::string Criteria);
    bool Exists(Archivehandle &Handle, std::string Filename);
    void Delete(Archivehandle &Handle, std::string Filename);
}
