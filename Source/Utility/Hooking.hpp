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
}
