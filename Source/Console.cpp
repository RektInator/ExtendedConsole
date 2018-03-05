#include "StdInclude.hpp"
#include "Function.hpp"

using namespace std::string_literals;

// Hooks
static Hooking::Stomphook OnFrameHook;

typedef float vec4_t[4];
static vec4_t colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };

static Function<void*(int, const char*)> DB_FindXAssetHeader;
static Function<void*()> ScrPlace_GetActivePlacement;
static Function<void __fastcall(void* placement, float x, float y, float width, float height, int XRect, int YRect, const vec4_t color, void* material)> UI_DrawHandlePic;
static Function<void __fastcall(void*, const char*, std::int32_t, void*, float, float, int, int, float scale, const vec4_t color, int style)> UI_DrawText;
static Function<void*(const char*)> Material_RegisterHandle;
static Function<void*(const char*)> R_RegisterFont;
static Function<void(int, const char*)> Cbuf_AddText;

std::string console_input = "";

void Console_DrawInputBox()
{
    auto branding = "^2ExtendedConsole^7 > "s;
    auto font = R_RegisterFont("fonts/consoleFont");
    auto console = Material_RegisterHandle("console");
    auto placement = ScrPlace_GetActivePlacement();

    // Draw input box
    UI_DrawHandlePic(placement, 5.0, 5.0, 630.0, 15.0, 4, 0, colorWhite, console);
    UI_DrawText(ScrPlace_GetActivePlacement(), branding.data(), branding.size(), font, 10.0, 15.6, 1, 1, 0.11, colorWhite, 0);

    // Draw input text
    UI_DrawText(ScrPlace_GetActivePlacement(), console_input.data(), console_input.size(), font, 10.0 + 55.0, 15.6, 1, 1, 0.11, colorWhite, 0);
}

bool console_isActive = false;

void Console_ToggleConsole()
{
    console_isActive = !console_isActive;
}

extern "C"
{
    // TODO: Base those on a pattern!
    std::uint64_t OnFrameHook_JumpBack_Jz =	0x140213B48;
    std::uint64_t OnFrameHook_JumpBack =	0x140213B3A;
    std::uint64_t OnFrameHook_Func =		0x1402138C0;
    std::uint64_t OnFrameHook_CmpAddr =		0x141CDBB28;

    void Console_OnFrame()
    {
        if (console_isActive)
        {
            Console_DrawInputBox();
        }
    }
    void OnFrameHookStub();
} 

static std::function<void(std::uint64_t a1, std::uint32_t key, std::uint32_t down, std::uint32_t a4)> CL_KeyEvent;

void CL_KeyEventHook(std::uint64_t a1, std::uint32_t key, std::uint32_t down, std::uint32_t a4)
{
    printf("key %i\n", key);

    if (down && (key == '~' || key == '`'))
    {
        Console_ToggleConsole();
    }

    if (console_isActive)
    {
        // enter pressed
        if (!down && key == 13)
        {
            if (!console_input.empty())
            {
                // append \n to the command (otherwise the command might not be executed)
                console_input.append("\n");
                Cbuf_AddText(0, &console_input[0]);
                console_input.clear();
            }
        }

        // backspace
        if (!down && key == 127)
        {
            if (!console_input.empty())
            {
                console_input = console_input.size() > 1 ? console_input.substr(0, console_input.size() - 1) : "";
            }
        }

        // letter keys
        if (down && key >= 32 && key < 127)
        {
            console_input += static_cast<char>(key);
        }
    }

    return CL_KeyEvent(a1, key, down, a4);
}

void Console_InstallHooks() 
{ 
    // Function pattenrs (tested on Advanced Warfare)
    ScrPlace_GetActivePlacement = Findpattern(
        Pattern::Textsegment,
        "8B 0D ? ? ? ? 85 C9 74 0F FF C9 74 0B 48 8D 05 ? ? ? ? FF C9 74 07 48 8D 05 ? ? ? ? F3 C3"
    );
    UI_DrawText = Findpattern(
        Pattern::Textsegment,
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 70 F3 0F 10 8C 24 ? ? ? ? 48 8B D9 49 8B C9 0F 29 7C 24 ? 49 8B F9 41 8B F0 48 8B EA E8 ? ? ? ? 8B 84 24 ? ? ? ? 89 44 24 30 8B 84 24 ? ? ? ? 4C 8D 4C 24 ? F3 0F 11 44 24 ? F3 0F 11 44 24 ? 89 44 24 28 48 8D 44 24 ? 4C 8D 84 24 ? ? ? ? 48 8D 94 24 ? ? ? ? 48 8B CB 48 89 44 24 ? E8 ? ? ? ?"
    );
    UI_DrawHandlePic = Findpattern(
        Pattern::Textsegment,
        "48 8B C4 48 81 EC ? ? ? ? F3 0F 10 2D ? ? ? ? F3 0F 10 25 ? ? ? ? 0F 29 70 E8 0F 29 78 D8 0F 57 C0 44 0F 29 40 ? 0F 2F C3 0F 28 FA 44 0F 28 C1 76 0B 0F 57 DD 0F 28 F4 0F 28 D0 EB 06"
    );
    DB_FindXAssetHeader = Findpattern(
        Pattern::Textsegment,
        "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 44 89 44 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC 40 33 DB 4C 8B EA 44 8B E1 44 8B FB BE ? ? ? ?"
    );
    Material_RegisterHandle = Findpattern(
        Pattern::Textsegment,
        "80 39 00 75 08 48 8B 05 ? ? ? ? C3 41 B8 ? ? ? ? 48 8B D1 41 8D 48 07 E9 ? ? ? ?"
    );
    R_RegisterFont = Findpattern(
        Pattern::Textsegment,
        "45 33 C0 83 FA 0C B0 01 41 0F 44 C0 48 8B D1 B9 ? ? ? ? 44 0F B6 C0 E9 ? ? ? ?"
    );
    Cbuf_AddText = Findpattern(
        Pattern::Textsegment,
        "48 89 5C 24 ? 57 48 83 EC 20 8B F9 B9 ? ? ? ? 48 8B DA E8 ? ? ? ? 0F B6 03 2C 50 A8 DF 75 26 0F B6 43 01 3C 30 75 1E 0F BE F8 48 83 C3 02 83 EF 30 80 3B 20 75 0F 0F 1F 80 ? ? ? ?"
    );

    // Hooks
    OnFrameHook.Installhook(
        reinterpret_cast<void*>(0x140213B2C), 
        reinterpret_cast<void*>(OnFrameHookStub)
    );
    CL_KeyEvent = Hooking::Hook::Detour(
        Findpattern(
            Pattern::Textsegment,
            "44 89 4C 24 ? 55 56 57 41 55 41 57 48 83 EC 50 48 63 F1 48 63 EA 41 8B F9 4C 8D 0D ? ? ? ? 4C 8B EE 45 8B F8 49 8D 91 ? ? ? ? 4C 8B C6 4D 69 ED ? ? ? ? 49 03 D5 43 83 BC 0D ? ? ? ? ? 74 32 81 FD ? ? ? ? 75 2A 45 85 FF 74 25 FF 15 ? ? ? ?"
        ), 
        CL_KeyEventHook, 
        12
    );
}
 