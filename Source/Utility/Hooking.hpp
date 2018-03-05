/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        A hook takes control over code-execution and can
        redirect it to your function instead of the apps.
*/

#pragma once
#include "../Stdinclude.hpp"

namespace Hooking
{
    #define EXTENDEDHOOKDECL(Basehook)                                  \
    template <typename Signature>                                       \
    struct Basehook ##Ex : public Basehook                              \
    {                                                                   \
        std::pair<std::mutex, std::function<Signature>> Function;       \
        virtual bool Installhook(void *Location, void *Target) override \
        {                                                               \
            Function.second = *(Signature *)Location;                   \
            return Basehook::Installhook(Location, Target);             \
        }                                                               \
    }                                                                   \

    // Basic interface for hooks.
    struct IHook
    {
        uint8_t Savedcode[20]{};
        void *Savedlocation;
        void *Savedtarget;

        virtual bool Removehook() = 0;
        virtual bool Installhook(void *Location, void *Target) = 0;
        virtual bool Reinstall() { return Installhook(Savedlocation, Savedtarget); };
    };

    // A dumb hook that just inserts a jump.
    struct Stomphook : public IHook
    {
        virtual bool Removehook() override;
        virtual bool Installhook(void *Location, void *Target) override;
    };
    EXTENDEDHOOKDECL(Stomphook);

    // A dumb hook that just inserts a call.
    struct Callhook : public IHook
    {
        virtual bool Removehook() override;
        virtual bool Installhook(void *Location, void *Target) override;
    };
    EXTENDEDHOOKDECL(Callhook);

    // 
    class Hook
    {
    public:
        using Ptr = typename std::shared_ptr < Hook >;

        Hook(bool uninstallOnDestruct = true);
        Hook(const Hook& hook);
        Hook(const std::uintptr_t& address, bool uninstallOnDestruct = true);
        ~Hook();

        Hook* Install(const std::uintptr_t& address);
        void Uninstall();

        static Ptr make_shared(bool uninstallOnDestruct = true);
        static Ptr make_shared(const Hook& hook);
        static Ptr make_shared(const std::uintptr_t& address, bool uninstallOnDestruct = true);

        void Nop(const std::size_t& size);
        static void Nop(const std::uintptr_t& address, const std::size_t& size);
        void Write(const std::vector < std::uint8_t >& bytes);
        static void Write(const std::uintptr_t& address, const std::vector < std::uint8_t >& bytes);
        void WriteString(const std::string& string);
        static void WriteString(const std::uintptr_t& address, const std::string& string);

        template <typename T> void Set(const T& value)
        {
            this->SetOriginalData(sizeof T);
            Hook::Set<T>(this->hookAddress, value);
        }
        template <typename T> static void Set(const std::uintptr_t& address, const T& value)
        {
            auto protection = Hook::Unprotect(address, sizeof T);
            memcpy(reinterpret_cast<void*>(address), &value, sizeof T);
            Hook::Protect(address, sizeof T, protection);
        }

        template <typename T> void Call(const T& dest)
        {
#ifdef ENVIRONMENT64
            this->SetOriginalData(12);
#else
            this->SetOriginalData(5);
#endif

            Hook::Call<T>(this->hookAddress, dest);
        }

        template <typename T> static void Call(const std::uintptr_t& address, const T& dest)
        {
#ifdef ENVIRONMENT64
            auto protection = Hook::Unprotect(address, 12);
            Hook::Set<std::uint8_t>(address, 0x48);
            Hook::Set<std::uint8_t>(address + 1, 0xB8);
            Hook::Set<std::uintptr_t>(address + 2, std::uintptr_t(dest));
            Hook::Set<std::uint8_t>(address + 10, 0xFF);
            Hook::Set<std::uint8_t>(address + 11, 0xD0);
            Hook::Protect(address, 12, protection);
#else
            auto protection = Hook::Unprotect(address, 5);
            Hook::Set<std::uint8_t>(address, 0xE8);
            Hook::Set<std::uint32_t>(address + 1, std::uintptr_t(dest) - address - 5);
            Hook::Protect(address, 5, protection);
#endif
        }

        template <typename T> void Jump(const T& dest)
        {
#ifdef ENVIRONMENT64
            this->SetOriginalData(12);
#else
            this->SetOriginalData(5);
#endif

            Hook::Jump<T>(this->hookAddress, dest);
        }

        template <typename T> static void Jump(const std::uintptr_t& address, const T& dest)
        {
#ifdef ENVIRONMENT64
            auto protection = Hook::Unprotect(address, 12);
            Hook::Set<std::uint8_t>(address, 0x48);
            Hook::Set<std::uint8_t>(address + 1, 0xB8);
            Hook::Set<std::uintptr_t>(address + 2, std::uintptr_t(dest));
            Hook::Set<std::uint8_t>(address + 10, 0xFF);
            Hook::Set<std::uint8_t>(address + 11, 0xE0);
            Hook::Protect(address, 12, protection);
#else
            auto protection = Hook::Unprotect(address, 5);
            Hook::Set<std::uint8_t>(address, 0xE9);
            Hook::Set<std::uint32_t>(address + 1, std::uintptr_t(dest) - address - 5);
            Hook::Protect(address, 5, protection);
#endif
        }

        template <typename T> std::function<T> Detour(const T& dest, const std::size_t& size)
        {
            this->SetOriginalData(size);
            return Hook::Detour(this->hookAddress, dest, size);
        }

        template <typename T> static std::function<T> Detour(const std::uintptr_t& address, const T& dest, const std::size_t& size)
        {
#ifdef _WIN32
#ifdef ENVIRONMENT64
            static const constexpr auto jumpSize = 12;
#else
            static const constexpr auto jumpSize = 5;
#endif
            auto trampoline = VirtualAlloc(nullptr, size + jumpSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            memcpy(trampoline, reinterpret_cast<char*>(address), size + jumpSize);

            // redirect function to our function
            Hook::Jump<T>(address, dest);

            // redirect trampoline to the original function
            Hook::Jump(reinterpret_cast<std::uintptr_t>(trampoline) + size, address + size);

            // return trampoline
            return std::function<T>(reinterpret_cast<T*>(trampoline));
#else
            throw "Detour is not implemented for UNIX based systems.";
#endif
        }

    private:
        static std::uint32_t Unprotect(const std::uintptr_t& address, const std::size_t& size);
        static void Protect(const std::uintptr_t& address, const std::size_t& size, const std::uint32_t& protection);

        void SetOriginalData(const std::size_t& size)
        {
            if (this->originalData.empty())
            {
                this->originalData.resize(size);
                memcpy(&this->originalData[0], reinterpret_cast<void*>(this->hookAddress), size);
            }
        }

    protected:
        bool							uninstallOnDestruct;
        std::vector < std::uint8_t >	originalData;
        std::uintptr_t					hookAddress;

    };
}
