/*
    Initial author: Convery (tcn@hedgehogscience.com)
    Started: 08-01-2018
    License: MIT
    Notes:
        Provides the entrypoint for Windows and Nix.
*/

#include "Stdinclude.hpp"

void Console_InstallHooks();

// Callbacks from the bootstrapper (Bootstrapmodule_cpp).
extern "C" EXPORT_ATTR void onInitializationStart(bool Reserved)
{
    /*
        ----------------------------------------------------------------------
        This callback is called when the game is initialized, which means that
        all other libraries are loaded; but maybe not all other plugins.
        Your plugins should take this time to modify the games .text segment
        as well as initializing all your own systems.
        ----------------------------------------------------------------------
    */

	// 
}
extern "C" EXPORT_ATTR void onInitializationDone(bool Reserved)
{
    /*
        ----------------------------------------------------------------------
        This callback is called when the platform notifies the bootstrapper,
        or at most 3 seconds after startup. This is the perfect time to start
        communicating with other plugins and modify the games .data segment.
        ----------------------------------------------------------------------
    */

	Console_InstallHooks();
}
extern "C" EXPORT_ATTR void onMessage(uint32_t MessageID, uint32_t Messagesize, const void *Messagedata)
{
    /*
        ----------------------------------------------------------------------
        This callback is called when another plugin broadcasts a message. They
        can safely be ignored, but if you want to make use of the system you
        should create a unique name for your messages. We recommend that you
        base it on your pluginname as shown below, we also recommend that you
        use the bytebuffer format for data.
        ----------------------------------------------------------------------
    */

    // Create a buffer from the data like such:
    // Bytebuffer Message{ Messagesize, Messagedata};

    // MessageID is a FNV1a_32 hash of a string.
    switch (MessageID)
    {
        case Hash::FNV1a_32(MODULENAME "::Default"):
        default: break;
    }
}

#if defined _WIN32
BOOLEAN WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    switch (nReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            // Opt-out of further thread notifications.
            DisableThreadLibraryCalls(hDllHandle);

            // Clear the previous sessions logfile.
            Clearlog();
        }
    }

    return TRUE;
}
#else
__attribute__((constructor)) void DllMain()
{
    // Clear the previous sessions logfile.
    Clearlog();
}
#endif
