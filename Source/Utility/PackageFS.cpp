/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Plugins should keep all their files in their archive.
*/

#include "../Stdinclude.hpp"
#include "Thirdparty/zip_file.hpp"

namespace Package
{
    // Ensure that the names are defined.
    #if !defined (MODULENAME)
        #define MODULENAME "Invalid"
    #endif
    #if !defined (MODULEEXTENSION)
        #define MODULEEXTENSION "Invalid"
    #endif

    // Operations on the default archive.
    std::string Read(std::string Filename)
    {
        auto Handle = Loadarchive("./Plugins/" MODULENAME "." MODULEEXTENSION);
        return Read(Handle, Filename);
    }
    void Write(std::string Filename, std::string &Buffer)
    {
        auto Handle = Loadarchive("./Plugins/" MODULENAME "." MODULEEXTENSION);
        return Write(Handle, Filename, Buffer);
    }
    std::vector<std::string> Findfiles(std::string Criteria)
    {
        auto Handle = Loadarchive("./Plugins/" MODULENAME "." MODULEEXTENSION);
        return Findfiles(Handle, Criteria);
    }
    bool Exists(std::string Filename)
    {
        auto Handle = Loadarchive("./Plugins/" MODULENAME "." MODULEEXTENSION);
        return Exists(Handle, Filename);
    }
    void Delete(std::string Filename)
    {
        auto Handle = Loadarchive("./Plugins/" MODULENAME "." MODULEEXTENSION);
        return Delete(Handle, Filename);
    }

    // Operations on a specific archive.
    Archivehandle Loadarchive(std::string Filename)
    {
        // Workaround for dev-plugins not having the file.
        if (!Fileexists(Filename))
        {
            miniz_cpp::zip_file Archive;
            Archive.save(Filename);
        }

        return new miniz_cpp::zip_file(Filename);
    }
    void Savearchive(Archivehandle &Handle, std::string Filename)
    {
        auto Archive = reinterpret_cast<miniz_cpp::zip_file *>(Handle);
        Archive->save(Filename);
    }
    std::string Read(Archivehandle &Handle, std::string Filename)
    {
        auto Archive = reinterpret_cast<miniz_cpp::zip_file *>(Handle);

        if (!Archive->has_file(Filename)) return {};
        else return Archive->read(Filename);
    }
    void Write(Archivehandle &Handle, std::string Filename, std::string &Buffer)
    {
        auto Archive = reinterpret_cast<miniz_cpp::zip_file *>(Handle);
        Delete(Handle, Filename);

        Archive->writestr(Filename, Buffer);
        Savearchive(Handle, Archive->get_filename());
    }
    std::vector<std::string> Findfiles(Archivehandle &Handle, std::string Criteria)
    {
        auto Archive = reinterpret_cast<miniz_cpp::zip_file *>(Handle);
        auto Filelist = Archive->namelist();
        std::vector<std::string> Filenames;

        // Enqueue the files matching the extension.
        for (auto &Item : Filelist)
            if (std::strstr(Item.c_str(), Criteria.c_str()))
                Filenames.push_back(Item);

        return std::move(Filenames);
    }
    bool Exists(Archivehandle &Handle, std::string Filename)
    {
        auto Archive = reinterpret_cast<miniz_cpp::zip_file *>(Handle);
        return Archive->has_file(Filename);
    }
    void Delete(Archivehandle &Handle, std::string Filename)
    {
        if (!Exists(Handle, Filename)) return;

        auto Archive = reinterpret_cast<miniz_cpp::zip_file *>(Handle);
        auto Newarchive = new miniz_cpp::zip_file();
        auto Filelist = Archive->namelist();

        for (auto &Item : Filelist)
        {
            if (0 != std::strcmp(Item.c_str(), Filename.c_str()))
            {
                Newarchive->writestr(Item, Archive->read(Item));
            }
        }

        std::vector<uint8_t> Buffer;
        Newarchive->save(Buffer);
        Archive->load(Buffer);
    }
}
