/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Provides fast and simple storage for messages.
*/

#pragma once
#include "../Stdinclude.hpp"

// The types of data that can be handled.
enum Bytebuffertype : uint8_t
{
    BB_NONE = 0,
    BB_BOOL = 1,
    BB_SINT8 = 2,
    BB_UINT8 = 3,
    BB_SINT16 = 4,
    BB_UINT16 = 5,
    BB_SINT32 = 6,
    BB_UINT32 = 7,
    BB_SINT64 = 8,
    BB_UINT64 = 9,
    BB_FLOAT32 = 10,
    BB_FLOAT64 = 11,
    BB_STRING_WIDE = 12,
    BB_STRING_ASCII = 13,
    BB_BLOB = 14,
    BB_ARRAY = 15,
    BB_MAX
};

class Bytebuffer
{
    // Generic storage-type.
    using Type_t = std::pair<Bytebuffertype, void *>;
    
    // Internal state properties.
    std::unique_ptr<uint8_t[]> Internalbuffer;
    std::vector<Type_t> Internalvariables;
    size_t Internaliterator;
    size_t Internalsize;

public:
    // Core functionality.
    bool Readdatatype(Bytebuffertype Type);                         // Compares the next byte with the input.
    bool Writedatatype(Bytebuffertype Type);                        // Writes the input as the next byte.
    bool Rawread(size_t Readcount, void *Buffer = nullptr);         // Reads from the internal buffer.
    bool Rawwrite(size_t Writecount, const void *Buffer = nullptr); // Writes to the internal buffer.

    // Creates the internal state.
    Bytebuffer(size_t Datasize, const void *Databuffer);
    void Setbuffer(std::vector<uint8_t> &Data);
    Bytebuffer(std::vector<uint8_t> &Data);
    Bytebuffer(const Bytebuffer &Right);
    void Setbuffer(std::string &Data);
    Bytebuffer(Bytebuffer &&Right);
    Bytebuffer(std::string &Data);
    Bytebuffer();

    // Access the internal state.
    bool Setposition(size_t Newposition);                           // Sets the internal read/write iterator.
    const size_t Getposition();                                     // Gets the internal read/write iterator.
    std::string to_string();                                        // Print Internalvariables.
    const uint8_t *Data();                                          // Returns a pointer to the internal buffer.
    const uint8_t Peek();                                           // Returns the next byte in the buffer or -1.
    const size_t Size();                                            // Returns the size of the current buffer.
    void Deserialize();                                             // Deserialize the buffer into variables.
    void Rewind();                                                  // Resets the internal read/write iterator.
    void Clear();                                                   // Clears the internal buffer.

    // Single data IO.
    template <typename Type> Type Read(bool Typechecked = true);
    template <typename Type> bool Read(Type &Buffer, bool Typechecked = true);
    template <typename Type> bool Write(const Type Value, bool Typechecked = true);

    // Multiple data IO.
    template <typename Type> bool Readarray(std::vector<Type> &Data);
    template <typename Type> bool Writearray(std::vector<Type> Data);

    // Direct IO.
    template <typename Type> Bytebuffer &operator += (const Type &Right) noexcept;
    template <typename Type> Bytebuffer &operator += (const Type *Right) noexcept;
    template <typename Type> Bytebuffer &operator << (const Type &Right) noexcept;
    template <typename Type> Bytebuffer &operator << (const Type *Right) noexcept;

    // Supported operators, acts on the internal state.
    Bytebuffer &operator += (const Bytebuffer &Right) noexcept;
    Bytebuffer &operator + (const Bytebuffer &Right) noexcept;
    Bytebuffer &operator = (const Bytebuffer &Right) noexcept;
    Bytebuffer &operator = (Bytebuffer &&Right) noexcept;
    bool operator == (const Bytebuffer &Right) noexcept;
};
