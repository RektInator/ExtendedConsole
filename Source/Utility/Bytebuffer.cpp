/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Provides fast and simple storage for messages.
*/

#include "../Stdinclude.hpp"

// Core functionality.
bool Bytebuffer::Readdatatype(Bytebuffertype Type)
{
    if (Type == Peek())
        return Setposition(Getposition() + 1);
    return false;
}
bool Bytebuffer::Writedatatype(Bytebuffertype Type)
{
    return Rawwrite(sizeof(uint8_t), &Type);
}
bool Bytebuffer::Rawread(size_t Readcount, void *Buffer)
{
    // Rangecheck, we do not do truncated reads as they are a pain to debug.
    if ((Internaliterator + Readcount) > Internalsize) return false;

    // Copy the data into the new buffer if provided.
    if (Buffer) std::memcpy(Buffer, Internalbuffer.get() + Internaliterator, Readcount);

    // Advance the internal iterator.
    Internaliterator += Readcount;
    return true;
}
bool Bytebuffer::Rawwrite(size_t Writecount, const void *Buffer)
{
    // If we are at the end of the buffer, increase it.
    if (Internaliterator == Internalsize)
    {
        auto Newbuffer = std::make_unique<uint8_t[]>(Internalsize + Writecount);
        std::memmove(Newbuffer.get(), Internalbuffer.get(), Internalsize);
        Internalbuffer.swap(Newbuffer);
        Internalsize += Writecount;
    }

    // If the buffer is already large enough, overwrite.
    if ((Internaliterator + Writecount) <= Internalsize)
    {
        if(Buffer) std::memcpy(Internalbuffer.get() + Internaliterator, Buffer, Writecount);
        Internaliterator += Writecount;
        return true;
    }

    // Else we need to make an overwrite and an append call.
    size_t Overwritecount = Internalsize - Internaliterator;
    return Rawwrite(Overwritecount, Buffer) && Rawwrite(Writecount - Overwritecount, Buffer ? (uint8_t *)Buffer + Overwritecount : Buffer);
}

// Creates the internal state.
Bytebuffer::Bytebuffer(size_t Datasize, const void *Databuffer)
{
    Internaliterator = 0;
    Internalsize = Datasize;
    Internalbuffer = std::make_unique<uint8_t[]>(Internalsize);
    std::memcpy(Internalbuffer.get(), Databuffer, Internalsize);
}
void Bytebuffer::Setbuffer(std::vector<uint8_t> &Data)
{
    Internaliterator = 0;
    Internalsize = Data.size();
    Internalbuffer = std::make_unique<uint8_t[]>(Internalsize);
    std::memcpy(Internalbuffer.get(), Data.data(), Internalsize);
}
Bytebuffer::Bytebuffer(std::vector<uint8_t> &Data)
{
    Setbuffer(Data);
}
Bytebuffer::Bytebuffer(const Bytebuffer &Right)
{
    Internalbuffer = std::make_unique<uint8_t[]>(Right.Internalsize);
    std::memcpy(Internalbuffer.get(), Right.Internalbuffer.get(), Right.Internalsize);

    Internaliterator = Right.Internaliterator;
    Internalsize = Right.Internalsize;
}
void Bytebuffer::Setbuffer(std::string &Data)
{
    Internaliterator = 0;
    Internalsize = Data.size();
    Internalbuffer = std::make_unique<uint8_t[]>(Internalsize);
    std::memcpy(Internalbuffer.get(), Data.data(), Internalsize);
}
Bytebuffer::Bytebuffer(Bytebuffer &&Right)
{
    Internaliterator = std::exchange(Right.Internaliterator, NULL);
    Internalbuffer = std::exchange(Right.Internalbuffer, nullptr);
    Internalsize = std::exchange(Right.Internalsize, NULL);
}
Bytebuffer::Bytebuffer(std::string &Data)
{
    Setbuffer(Data);
}
Bytebuffer::Bytebuffer()
{
    Internalbuffer = std::make_unique<uint8_t []>(0);
    Internaliterator = 0;
    Internalsize = 0;
}

// Access the internal state.
bool Bytebuffer::Setposition(size_t Newposition)
{
    if (Newposition > Internalsize) return false;
    Internaliterator = Newposition;
    return true;
}
const size_t Bytebuffer::Getposition()
{
    return Internaliterator;
}
std::string Bytebuffer::to_string()
{
    std::string Result = "{\n";

    auto Localprint = [](Type_t Item) -> std::string
    {
        std::string Result = "\t";

        switch (Item.first)
        {
            case Bytebuffertype::BB_BOOL: Result += va("bool = %s;\n", *(bool *)Item.second ? "true" : "false"); break;
            case Bytebuffertype::BB_SINT8: Result += va("int8_t = %i;\n", *(int8_t *)Item.second); break;
            case Bytebuffertype::BB_UINT8: Result += va("uint8_t = 0x%02X;\n", *(uint8_t *)Item.second); break;
            case Bytebuffertype::BB_SINT16: Result += va("int16_t = %i;\n", *(int16_t *)Item.second); break;
            case Bytebuffertype::BB_UINT16: Result += va("uint16_t = 0x%04X;\n", *(uint16_t *)Item.second); break;
            case Bytebuffertype::BB_SINT32: Result += va("int32_t = %i;\n", *(int32_t *)Item.second); break;
            case Bytebuffertype::BB_UINT32: Result += va("uint32_t = 0x%X;\n", *(uint32_t *)Item.second); break;
            case Bytebuffertype::BB_SINT64: Result += va("int64_t = %lli;\n", *(int64_t *)Item.second); break;
            case Bytebuffertype::BB_UINT64: Result += va("uint64_t = 0x%llX;\n", *(uint64_t *)Item.second); break;

            case Bytebuffertype::BB_FLOAT32: Result += va("float = %f;\n", *(float *)Item.second); break;
            case Bytebuffertype::BB_FLOAT64: Result += va("double = %f;\n", *(double *)Item.second); break;

            case Bytebuffertype::BB_STRING_WIDE: Result += va("std::wstring = \"%ls\";\n", ((std::wstring *)Item.second)->c_str()); break;
            case Bytebuffertype::BB_STRING_ASCII: Result += va("std::string = \"%s\";\n", ((std::string *)Item.second)->c_str()); break;

            case Bytebuffertype::BB_BLOB:
                Result += va("std::array<uint8_t>[%u] = { \"", ((std::string *)Item.second)->size());
                for (size_t i = 0; i < ((std::string *)Item.second)->size(); ++i)
                    Result += va("\\x%02X", (uint8_t)((std::string *)Item.second)->at(i));
                Result += "\" };\n";
                break;

            // NONE, MAX
            default: Result += va("Type_%i = NULL;\n", Item.first); break;
        }

        return Result;
    };

    Deserialize();
    for (auto &Item : Internalvariables)
    {
        // Simple type.
        if (Item.first >= BB_BOOL && Item.first <= BB_BLOB)
        {
            Result += Localprint(Item);
            continue;
        }

        // Collection.
        if(Item.first == BB_ARRAY)
        {
            Result += va("\tstd::array<T>[%u] = \n\t{\n", ((std::vector<Type_t> *)Item.second)->size());

            for (auto &Entry : *((std::vector<Type_t> *)Item.second))
            {
                Result += "\t";
                Result += Localprint(Entry);
            }

            Result += "\t}\n";
            continue;
        }

        Result += va("Type_%i = NULL;\n", Item.first);
        break;
    }

    Result += "}";
    return Result;
}
const uint8_t *Bytebuffer::Data()
{
    return Internalbuffer.get();
}
const uint8_t Bytebuffer::Peek()
{
    uint8_t Byte = uint8_t(-1);

    // Read the single byte.
    if (Rawread(sizeof(uint8_t), &Byte))
    {
        // Rewind to before the read.
        Setposition(Getposition() - 1);
    }

    return Byte;
}
const size_t Bytebuffer::Size()
{
    return Internalsize;
}
void Bytebuffer::Deserialize()
{
    uint8_t *Localpointer = Internalbuffer.get();
    size_t Localiterator = 0;
    uint8_t Localtype = 0;

    // Clear any old data.
    Internalvariables.clear();
    Internalvariables.shrink_to_fit();

    auto Localread = [&](uint8_t Type) -> Type_t
    {
        switch (Bytebuffertype(Type))
        {
            case Bytebuffertype::BB_BOOL: return { BB_BOOL, Localpointer + Localiterator };
            case Bytebuffertype::BB_SINT8: return { BB_SINT8, Localpointer + Localiterator };
            case Bytebuffertype::BB_UINT8: return { BB_UINT8, Localpointer + Localiterator };
            case Bytebuffertype::BB_SINT16: return { BB_SINT16, Localpointer + Localiterator };
            case Bytebuffertype::BB_UINT16: return { BB_UINT16, Localpointer + Localiterator };
            case Bytebuffertype::BB_SINT32: return { BB_SINT32, Localpointer + Localiterator };
            case Bytebuffertype::BB_UINT32: return { BB_UINT32, Localpointer + Localiterator };
            case Bytebuffertype::BB_SINT64: return { BB_SINT64, Localpointer + Localiterator };
            case Bytebuffertype::BB_UINT64: return { BB_UINT64, Localpointer + Localiterator };
            case Bytebuffertype::BB_FLOAT32: return { BB_FLOAT32, Localpointer + Localiterator };
            case Bytebuffertype::BB_FLOAT64: return { BB_FLOAT64, Localpointer + Localiterator };
            case Bytebuffertype::BB_STRING_WIDE: return { BB_STRING_WIDE, new std::wstring((wchar_t *)(Localpointer + Localiterator)) };
            case Bytebuffertype::BB_STRING_ASCII: return { BB_STRING_ASCII, new std::string((char *)(Localpointer + Localiterator)) };

            default: break;
        }

        return { BB_NONE, nullptr };
    };
    auto Localincrement = [&](uint8_t Type) -> void
    {
        switch (Bytebuffertype(Type))
        {
            case Bytebuffertype::BB_BOOL: Localiterator += sizeof(bool); break;
            case Bytebuffertype::BB_SINT8: Localiterator += sizeof(int8_t); break;
            case Bytebuffertype::BB_UINT8: Localiterator += sizeof(uint8_t); break;
            case Bytebuffertype::BB_SINT16: Localiterator += sizeof(int16_t); break;
            case Bytebuffertype::BB_UINT16: Localiterator += sizeof(uint16_t); break;
            case Bytebuffertype::BB_SINT32: Localiterator += sizeof(int32_t); break;
            case Bytebuffertype::BB_UINT32: Localiterator += sizeof(uint32_t); break;
            case Bytebuffertype::BB_SINT64: Localiterator += sizeof(int64_t); break;
            case Bytebuffertype::BB_UINT64: Localiterator += sizeof(uint64_t); break;
            case Bytebuffertype::BB_FLOAT32: Localiterator += sizeof(float); break;
            case Bytebuffertype::BB_FLOAT64: Localiterator += sizeof(double); break;

            case Bytebuffertype::BB_STRING_ASCII: Localiterator += std::strlen((char *)(Localpointer + Localiterator)) + sizeof(char); break;
            case Bytebuffertype::BB_STRING_WIDE: Localiterator += std::wcslen((wchar_t *)(Localpointer + Localiterator)) * sizeof(wchar_t) + sizeof(wchar_t); break;

            default: break;
        }
    };

    while (Localiterator < Internalsize)
    {
        Localtype = *(Localpointer + Localiterator);
        Localiterator += sizeof(uint8_t);

        // Simple type.
        if (Localtype >= BB_BOOL && Localtype <= BB_STRING_WIDE)
        {
            Internalvariables.push_back(Localread(Localtype));
            Localincrement(Localtype);
            continue;
        }

        // Collection.
        if (Localtype >= BB_BOOL + 100 && Localtype <= BB_STRING_WIDE + 100)
        {
            
            uint32_t Arraysize = *(uint32_t *)(Localpointer + Localiterator);
            auto Arraydata = new std::vector<Type_t>();
            Localiterator += sizeof(uint32_t);

            for (uint32_t i = 0; i < Arraysize; ++i)
            {
                Arraydata->push_back(Localread(Localtype - 100));
                Localincrement(Localtype - 100);
            }

            Internalvariables.push_back({ BB_ARRAY, Arraydata });
            continue;
        }

        // Blob data.
        if (Localtype == BB_BLOB)
        {
            Localiterator += sizeof(uint8_t);
            uint32_t Blobsize = *(uint32_t *)(Localpointer + Localiterator);
            Localiterator += sizeof(uint32_t);

            Internalvariables.push_back({ BB_BLOB, new std::string((char *)(Localpointer + Localiterator), Blobsize) });
            Localiterator += Blobsize;
            continue;
        }

        break;
    }
}
void Bytebuffer::Rewind()
{
    Internaliterator = 0;
}
void Bytebuffer::Clear()
{
    Internalbuffer = std::make_unique<uint8_t[]>(0);
    Internaliterator = 0;
    Internalsize = 0;
}

// Single data IO.
#pragma region SINGLE_IO
#define SINGLE_TEMPLATE(Type, Enum)                                         \
template <> bool Bytebuffer::Read(Type &Buffer, bool Typechecked)           \
{                                                                           \
    if(!Typechecked || Readdatatype(Enum))                                  \
        return Rawread(sizeof(Buffer), &Buffer);                            \
    else return false;                                                      \
}                                                                           \
template <> Type Bytebuffer::Read(bool Typechecked)                         \
{                                                                           \
    Type Result{};                                                          \
    Read(Result, Typechecked);                                              \
    return Result;                                                          \
}                                                                           \
template <> bool Bytebuffer::Write(const Type Buffer, bool Typechecked)     \
{                                                                           \
    if(Typechecked) Writedatatype(Enum);                                    \
    return Rawwrite(sizeof(Buffer), &Buffer);                               \
}                                                                           \

SINGLE_TEMPLATE(bool, BB_BOOL);
SINGLE_TEMPLATE(char, BB_SINT8);
SINGLE_TEMPLATE(int8_t, BB_SINT8);
SINGLE_TEMPLATE(uint8_t, BB_UINT8);
SINGLE_TEMPLATE(int16_t, BB_SINT16);
SINGLE_TEMPLATE(uint16_t, BB_UINT16);
SINGLE_TEMPLATE(int32_t, BB_SINT32);
SINGLE_TEMPLATE(uint32_t, BB_UINT32);
SINGLE_TEMPLATE(int64_t, BB_SINT64);
SINGLE_TEMPLATE(uint64_t, BB_UINT64);
SINGLE_TEMPLATE(float, BB_FLOAT32);
SINGLE_TEMPLATE(double, BB_FLOAT64);

template <> bool Bytebuffer::Read(std::string &Buffer, bool Typechecked)
{
    if (!Typechecked || Readdatatype(BB_STRING_ASCII))
    {
        size_t Stringlength = std::strlen((const char *)Data() + Internaliterator) + 1;
        Buffer.append((const char *)Data() + Internaliterator);
        return Setposition(Getposition() + Stringlength);
    }
    return false;
}
template <> std::string Bytebuffer::Read(bool Typechecked)
{
    std::string Result{};
    Read(Result, Typechecked);
    return Result;
}
template <> bool Bytebuffer::Write(const std::string Buffer, bool Typechecked)
{
    if(Typechecked) Writedatatype(BB_STRING_ASCII);
    return Rawwrite(Buffer.size() + 1, Buffer.c_str());
}

template <> bool Bytebuffer::Read(std::wstring &Buffer, bool Typechecked)
{
    if (!Typechecked || Readdatatype(BB_STRING_WIDE))
    {
        size_t Stringlength = std::wcslen((const wchar_t *)(Data() + Internaliterator)) + 1;
        Buffer.append((const wchar_t *)(Data() + Internaliterator));
        return Setposition(Getposition() + Stringlength * sizeof(wchar_t));
    }
    return false;
}
template <> std::wstring Bytebuffer::Read(bool Typechecked)
{
    std::wstring Result{};
    Read(Result, Typechecked);
    return Result;
}
template <> bool Bytebuffer::Write(const std::wstring Buffer, bool Typechecked)
{
    if (Typechecked) Writedatatype(BB_STRING_WIDE);
    return Rawwrite((Buffer.size() + 1) * sizeof(wchar_t), Buffer.c_str());
}

template <> bool Bytebuffer::Read(std::vector<uint8_t> &Buffer, bool Typechecked)
{
    if (!Typechecked || Readdatatype(BB_BLOB))
    {
        uint32_t Bloblength = Read<uint32_t>();
        auto Localbuffer = std::make_unique<uint8_t[]>(Bloblength);

        if (Rawread(Bloblength, Localbuffer.get()))
        {
            for (uint32_t i = 0; i < Bloblength; ++i)
                Buffer.push_back(Localbuffer[i]);

            return true;
        }
    }
    return false;
}
template <> std::vector<uint8_t> Bytebuffer::Read(bool Typechecked)
{
    std::vector<uint8_t> Result{};
    Read(Result, Typechecked);
    return Result;
}
template <> bool Bytebuffer::Write(const std::vector<uint8_t> Buffer, bool Typechecked)
{
    if (Typechecked) Writedatatype(BB_BLOB);
    return Write(uint32_t(Buffer.size())) && Rawwrite(Buffer.size(), Buffer.data());
}

template <> bool Bytebuffer::Write(const char *Buffer, bool Typechecked)
{
    return Write(std::string(Buffer), Typechecked);
}
template <> bool Bytebuffer::Write(const wchar_t *Buffer, bool Typechecked)
{
    return Write(std::wstring(Buffer), Typechecked);
}

#pragma endregion

// Multiple data IO.
#pragma region MULTIPLE_IO
#define MULTI_TEMPLATE(Type, Enum)                                          \
template <> bool Bytebuffer::Readarray(std::vector<Type> &Data)             \
{                                                                           \
    uint8_t Storedtype = Read<uint8_t>(false);                              \
    if (Storedtype != Enum + 100) return false;                             \
                                                                            \
    uint32_t Storedcount = Read<uint32_t>(false);                           \
    for (; Storedcount; --Storedcount)                                      \
        Data.push_back({ Read<Type>(false) });                              \
    return true;                                                            \
}                                                                           \
template <> bool Bytebuffer::Writearray(std::vector<Type> Data)             \
{                                                                           \
    Write(uint8_t(Enum + 100), false);                                      \
    Write(uint32_t(Data.size()), false);                                    \
                                                                            \
    for (auto Item : Data) Write(Type(Item), false);                        \
    return true;                                                            \
}                                                                           \

MULTI_TEMPLATE(bool, BB_BOOL);
MULTI_TEMPLATE(char, BB_SINT8);
MULTI_TEMPLATE(int8_t, BB_SINT8);
MULTI_TEMPLATE(uint8_t, BB_UINT8);
MULTI_TEMPLATE(int16_t, BB_SINT16);
MULTI_TEMPLATE(uint16_t, BB_UINT16);
MULTI_TEMPLATE(int32_t, BB_SINT32);
MULTI_TEMPLATE(uint32_t, BB_UINT32);
MULTI_TEMPLATE(int64_t, BB_SINT64);
MULTI_TEMPLATE(uint64_t, BB_UINT64);
MULTI_TEMPLATE(float, BB_FLOAT32);
MULTI_TEMPLATE(double, BB_FLOAT64);

MULTI_TEMPLATE(std::string, BB_STRING_ASCII);
MULTI_TEMPLATE(std::wstring, BB_STRING_WIDE);
MULTI_TEMPLATE(std::vector<uint8_t>, BB_BLOB);

#pragma endregion

// Direct IO.
#pragma region DIRECT_IO
#define DIRECT_TEMPLATE(Type)                                                   \
template <> Bytebuffer &Bytebuffer::operator += (const Type &Right) noexcept    \
{ Write(Right); return *this; }                                                 \
template <> Bytebuffer &Bytebuffer::operator << (const Type &Right) noexcept    \
{ Write(Right); return *this; }                                                 \

DIRECT_TEMPLATE(bool);
DIRECT_TEMPLATE(char);
DIRECT_TEMPLATE(int8_t);
DIRECT_TEMPLATE(uint8_t);
DIRECT_TEMPLATE(int16_t);
DIRECT_TEMPLATE(uint16_t);
DIRECT_TEMPLATE(int32_t);
DIRECT_TEMPLATE(uint32_t);
DIRECT_TEMPLATE(int64_t);
DIRECT_TEMPLATE(uint64_t);
DIRECT_TEMPLATE(float);
DIRECT_TEMPLATE(double);

DIRECT_TEMPLATE(std::string);
DIRECT_TEMPLATE(std::wstring);
DIRECT_TEMPLATE(std::vector<uint8_t>);

template <> Bytebuffer &Bytebuffer::operator += (const char *Right) noexcept
{ Write(std::string(Right)); return *this; }
template <> Bytebuffer &Bytebuffer::operator += (const wchar_t *Right) noexcept
{ Write(std::wstring(Right)); return *this; }
template <> Bytebuffer &Bytebuffer::operator << (const char *Right) noexcept
{ Write(std::string(Right)); return *this; }
template <> Bytebuffer &Bytebuffer::operator << (const wchar_t *Right) noexcept
{ Write(std::wstring(Right)); return *this; }

#pragma endregion

// Supported operators, acts on the internal state.
Bytebuffer &Bytebuffer::operator + (const Bytebuffer &Right) noexcept
{
    *this += Right;
    return *this;
}
Bytebuffer &Bytebuffer::operator += (const Bytebuffer &Right) noexcept
{
    Rawwrite(Right.Internalsize, Right.Internalbuffer.get());
    return *this;
}
Bytebuffer &Bytebuffer::operator = (const Bytebuffer &Right) noexcept
{
    if (this != &Right)
    {
        if(Internalsize != Right.Internalsize) Internalbuffer = std::make_unique<uint8_t[]>(Right.Internalsize);
        std::memcpy(Internalbuffer.get(), Right.Internalbuffer.get(), Right.Internalsize);

        Internaliterator = Right.Internaliterator;
        Internalsize = Right.Internalsize;

        Internalvariables.clear();
        Deserialize();
    }

    return *this;
}
Bytebuffer &Bytebuffer::operator = (Bytebuffer &&Right) noexcept
{
    if (this != &Right)
    {
        Internaliterator = std::exchange(Right.Internaliterator, NULL);
        Internalbuffer = std::exchange(Right.Internalbuffer, nullptr);
        Internalsize = std::exchange(Right.Internalsize, NULL);

        Internalvariables.clear();
        Deserialize();
    }

    return *this;
}
bool Bytebuffer::operator == (const Bytebuffer &Right) noexcept
{
    if (Internalsize != Right.Internalsize) return false;
    return 0 == std::memcmp(Internalbuffer.get(), Right.Internalbuffer.get(), Internalsize);
}
