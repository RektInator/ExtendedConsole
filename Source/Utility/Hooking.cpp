/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        A hook takes control over code-execution and can
        redirect it to your function instead of the apps.
*/

#include "../Stdinclude.hpp"

// Restore the memory where the hook was placed.
bool Hooking::Stomphook::Removehook()
{
    auto Protection = Memprotect::Unprotectrange(Savedlocation, 20);
    {
        std::memcpy(Savedlocation, Savedcode, 20);
    }
    Memprotect::Protectrange(Savedlocation, 20, Protection);

    return true;
}
bool Hooking::Callhook::Removehook()
{
    auto Protection = Memprotect::Unprotectrange(Savedlocation, 20);
    {
        std::memcpy(Savedlocation, Savedcode, 20);
    }
    Memprotect::Protectrange(Savedlocation, 20, Protection);

    return true;
}

// Overwrite the games code with a redirection.
#if defined (ENVIRONMENT64)
bool Hooking::Stomphook::Installhook(void *Location, void *Target)
{
    Savedlocation = Location;
    Savedtarget = Target;

    auto Protection = Memprotect::Unprotectrange(Savedlocation, 20);
    {
        std::memcpy(Savedcode, Savedlocation, 20);

        *(uint8_t *)(uint64_t(Savedlocation) + 0) = 0x48;
        *(uint8_t *)(uint64_t(Savedlocation) + 1) = 0xB8;
        *(uint64_t *)(uint64_t(Savedlocation) + 2) = uint64_t(Target);
        *(uint8_t *)(uint64_t(Savedlocation) + 10) = 0xFF;
        *(uint8_t *)(uint64_t(Savedlocation) + 11) = 0xE0;
    }
    Memprotect::Protectrange(Savedlocation, 20, Protection);

    return true;
}
bool Hooking::Callhook::Installhook(void *Location, void *Target)
{
    Savedlocation = Location;
    Savedtarget = Target;

    auto Protection = Memprotect::Unprotectrange(Savedlocation, 20);
    {
        std::memcpy(Savedcode, Savedlocation, 20);

        *(uint8_t *)(uint64_t(Savedlocation) + 0) = 0x48;
        *(uint8_t *)(uint64_t(Savedlocation) + 1) = 0xB8;
        *(uint64_t *)(uint64_t(Savedlocation) + 2) = uint64_t(Target);
        *(uint8_t *)(uint64_t(Savedlocation) + 10) = 0xFF;
        *(uint8_t *)(uint64_t(Savedlocation) + 11) = 0xD0;
    }
    Memprotect::Protectrange(Savedlocation, 20, Protection);

    return true;
}

#else

bool Hooking::Stomphook::Installhook(void *Location, void *Target)
{
    Savedlocation = Location;
    Savedtarget = Target;

    auto Protection = Memprotect::Unprotectrange(Savedlocation, 20);
    {
        std::memcpy(Savedcode, Savedlocation, 20);

        *(uint8_t *)(uint32_t(Savedlocation) + 0) = 0xE9;
        *(uint32_t *)(uint32_t(Savedlocation) + 1) = uint32_t(Target) - uint32_t(Location) - 5;
    }
    Memprotect::Protectrange(Savedlocation, 20, Protection);

    return true;
}
bool Hooking::Callhook::Installhook(void *Location, void *Target)
{
    Savedlocation = Location;
    Savedtarget = Target;

    auto Protection = Memprotect::Unprotectrange(Savedlocation, 20);
    {
        std::memcpy(Savedcode, Savedlocation, 20);

        *(uint8_t *)(uint32_t(Savedlocation) + 0) = 0xE8;
        *(uint32_t *)(uint32_t(Savedlocation) + 1) = uint32_t(Target) - uint32_t(Location) - 5;
    }
    Memprotect::Protectrange(Savedlocation, 20, Protection);

    return true;
}
#endif

Hooking::Hook::Hook(bool uninstallOnDestruct)
{
    this->uninstallOnDestruct = uninstallOnDestruct;
}

Hooking::Hook::Hook(const Hook& hook)
{
    this->hookAddress = hook.hookAddress;
    this->uninstallOnDestruct = hook.uninstallOnDestruct;
    this->originalData = hook.originalData;
}

Hooking::Hook::Hook(const std::uintptr_t& address, bool uninstallOnDestruct)
{
    this->hookAddress = address;
    this->uninstallOnDestruct = uninstallOnDestruct;
}

Hooking::Hook::~Hook()
{
    if (this->uninstallOnDestruct)
    {
        this->Uninstall();
    }
}

Hooking::Hook* Hooking::Hook::Install(const std::uintptr_t& address)
{
    this->hookAddress = address;
    return this;
}

void Hooking::Hook::Uninstall()
{
    if (!this->originalData.empty())
    {
        auto protection = Hook::Unprotect(this->hookAddress, this->originalData.size());
        memcpy(reinterpret_cast<void*>(this->hookAddress), &this->originalData[0], this->originalData.size());
        Hook::Protect(this->hookAddress, this->originalData.size(), protection);
    }
}

Hooking::Hook::Ptr Hooking::Hook::make_shared(bool uninstallOnDestruct)
{
    return std::make_shared < Hooking::Hook >(uninstallOnDestruct);
}

Hooking::Hook::Ptr Hooking::Hook::make_shared(const Hook& hook)
{
    return std::make_shared < Hooking::Hook >(hook);
}

Hooking::Hook::Ptr Hooking::Hook::make_shared(const std::uintptr_t& address, bool uninstallOnDestruct)
{
    return std::make_shared < Hooking::Hook >(address, uninstallOnDestruct);
}

void Hooking::Hook::Nop(const std::size_t& size)
{
    this->SetOriginalData(size);
    Hooking::Hook::Nop(this->hookAddress, size);
}

void Hooking::Hook::Nop(const std::uintptr_t& address, const std::size_t& size)
{
    auto protection = Hook::Unprotect(address, size);
    memset(reinterpret_cast<void*>(address), 0x90, size);
    Hook::Protect(address, size, protection);
}

void Hooking::Hook::Write(const std::vector < std::uint8_t >& bytes)
{
    this->SetOriginalData(bytes.size());
    Hooking::Hook::Write(this->hookAddress, bytes);
}

void Hooking::Hook::Write(const std::uintptr_t& address, const std::vector < std::uint8_t >& bytes)
{
    auto protection = Hook::Unprotect(address, bytes.size());
    memcpy(reinterpret_cast<void*>(address), &bytes[0], bytes.size());
    Hook::Protect(address, bytes.size(), protection);
}

void Hooking::Hook::WriteString(const std::string& string)
{
    this->SetOriginalData(string.size() + 1);
    Hooking::Hook::WriteString(this->hookAddress, string);
}

void Hooking::Hook::WriteString(const std::uintptr_t& address, const std::string& string)
{
    auto protection = Hook::Unprotect(address, string.size());
    memcpy(reinterpret_cast<void*>(address), &string[0], string.size());
    Hook::Protect(address, string.size(), protection);
}

std::uint32_t Hooking::Hook::Unprotect(const std::uintptr_t& address, const std::size_t& size)
{
    return Memprotect::Unprotectrange(reinterpret_cast<void*>(address), size);
}

void Hooking::Hook::Protect(const std::uintptr_t& address, const std::size_t& size, const std::uint32_t& protection)
{
    return Memprotect::Protectrange(reinterpret_cast<void*>(address), size, protection);
}
