/*
    Initial author: RektInator
    Started: 05-03-2018
    License: MIT
    Notes:
        A patch that provides a In-Game console in various installments of the Call of Duty franchise.
*/

#include "StdInclude.hpp"
#include "Function.hpp"

using namespace std::string_literals;

// Hooks
static Hooking::Stomphook OnFrameHook;

typedef float vec4_t[4];
static vec4_t colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
static vec4_t colorYellow = { 1.0f, 1.0f, 0.0f, 1.0f };
static vec4_t colorCyan = { 0.0f, 1.0f, 1.0f, 1.0f };

static Function<void*(int, const char*)> DB_FindXAssetHeader;
static Function<void*()> ScrPlace_GetActivePlacement;
static Function<void __fastcall(void* placement, float x, float y, float width, float height, int XRect, int YRect, const vec4_t color, void* material)> UI_DrawHandlePic;
static Function<void __fastcall(void*, const char*, std::int32_t, void*, float, float, int, int, float scale, const vec4_t color, int style)> UI_DrawText;
static Function<void*(const char*)> Material_RegisterHandle;
static Function<void*(const char*)> R_RegisterFont;
static Function<void(int, const char*)> Cbuf_AddText;
static Function<bool(std::uintptr_t, int)> Key_IsCatcherActive;
static Function<void(std::uintptr_t, int)> Key_SetCatcher;
static Function<void(std::uintptr_t, int)> Key_RemoveCatcher;
static Function<bool(int, int)> Key_IsDown;

std::string console_input = "";

struct dvar_t
{
    const char* name;
    // todo
};

auto dvarCount =    reinterpret_cast<std::size_t*>(0x14B304A30);
auto sortedDvars =  reinterpret_cast<dvar_t**>(0x14B304A50);

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

std::vector < std::string > Console_FindCommands(const std::string& input)
{
    std::vector < std::string > foundSuggestions;

    // loop through commands

    return foundSuggestions;
}

std::vector < std::string > Console_FindDvars(const std::string& input)
{
    std::vector < std::string > foundSuggestions;

    // loop through dvars
    for (auto i = 0u; i < 8192; i++)
    {
        if (sortedDvars[i] && sortedDvars[i]->name)
        {
            if (strstr(sortedDvars[i]->name, &console_input[0]))
            {
                // add dvar to list
                foundSuggestions.push_back(sortedDvars[i]->name);

                // if suggestions list is too big, break it.
                if (foundSuggestions.size() >= 20)
                {
                    break;
                }
            }
        }
    }

    return foundSuggestions;
}

void Console_DrawSuggestions()
{
    auto font = R_RegisterFont("fonts/consoleFont");
    auto console = Material_RegisterHandle("console");
    auto placement = ScrPlace_GetActivePlacement();

    auto dvars = Console_FindDvars(console_input);
    auto commands = Console_FindCommands(console_input);
   
    std::vector < std::pair < std::string, std::uint8_t > > foundSuggestions;
    for (auto &command : commands)
    {
        foundSuggestions.push_back({ command, 1 });
        if (foundSuggestions.size() >= 20) break;
    }
    for (auto &dvar : dvars)
    {
        foundSuggestions.push_back({ dvar, 0 });
        if (foundSuggestions.size() >= 20) break;
    }

    if (foundSuggestions.size() >= 20)
    {
        foundSuggestions.push_back({ "^220+ suggestions, too many to show.", 2 });
    }

    if (foundSuggestions.size())
    {
        // draw suggestions
        UI_DrawHandlePic(placement, 5.0 + 40.0, 25.0, 590.0, 5.0 + (10.0 * foundSuggestions.size()), 4, 0, colorWhite, console);

        for (auto i = 0u; i < foundSuggestions.size(); i++)
        {
            UI_DrawText(placement, &foundSuggestions[i].first[0], foundSuggestions[i].first.size(), font, 10.0 + 55.0, 15.6 + 20.0 + (10.0 * i), 1, 1, 0.1, foundSuggestions[i].second == 1 ? colorCyan : colorYellow, 0);
        }
    }
}

void Console_ToggleConsole()
{
    if (!Key_IsCatcherActive(0, 1))
    {
        Key_SetCatcher(0, 1);
    }
    else
    {
        Key_RemoveCatcher(0, -2);
    }
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
        if (Key_IsCatcherActive(0, 1))
        {
            Console_DrawInputBox();

            if (!console_input.empty())
            {
                Console_DrawSuggestions();
            }
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

    if (Key_IsCatcherActive(0, 1))
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

        // 
        if (down)
        {
            if (key >= 97 && key < 126)
            {
                console_input += Key_IsDown(0, 160) ? key - 0x20 : key;
            }
            else if (key == 32)
            {
                console_input += key;
            }
            else if (key == 45)
            {
                console_input += Key_IsDown(0, 160) ? 95 : key;
            }
            else if (key >= 48 && key < 64)
            {
                console_input += Key_IsDown(0, 160) ? key - 0x10 : key;
            }
        }
        
    }

    return CL_KeyEvent(a1, key, down, a4);
}

extern "C"
{
    std::uint64_t CL_CharEventLoc;
    std::uint64_t CL_CharEventHook_JumpBack;

    void CL_CharEventHook(std::uint32_t a1, std::uint32_t key)
    {
        static Function<void(std::uint32_t, std::uint32_t)> CL_CharEvent(CL_CharEventLoc);
        return CL_CharEvent(a1, key);
    }
    void CL_CharEventHookStub();
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
    Key_IsCatcherActive = Findpattern(
        Pattern::Textsegment,
        "85 15 ? ? ? ? 0F 95 C0 C3"
    );
    Key_SetCatcher = Findpattern(
        Pattern::Textsegment,
        "F6 05 ? ? ? ? ? 74 03 83 CA 01 89 15 ? ? ? ? C3"
    );
    Key_RemoveCatcher = Findpattern(
        Pattern::Textsegment,
        "21 15 ? ? ? ? C3"
    );
    Key_IsDown = Findpattern(
        Pattern::Textsegment,
        "48 63 C2 83 F8 FF 75 03 33 C0 C3 48 63 D1 48 8D 0C 40 48 69 D2 ? ? ? ? 48 8D 04 8A 48 8D 0D ? ? ? ? 8B 04 08 C3"
    );

    // Hooks
    OnFrameHook.Installhook(
        reinterpret_cast<void*>(0x140213B2C),           // CHANGE TO PATTERN
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

    // The original function location
    /*CL_CharEventLoc = Findpattern(
        Pattern::Textsegment,
        "83 FA 60 0F 84 ? ? ? ? 48 89 74 24 ? 57 48 83 EC 20 8B FA 48 63 F1 83 FA 7E 74 79 8B 05 ? ? ? ? A8 01 75 6F A8 20 74 3F"
    );

    // Will be used as trampoline
    auto CL_CharEventTrampolineLoc = Findpattern(
        Pattern::Textsegment,
        "CC CC CC CC CC CC CC CC CC CC CC CC CC"
    );
    CL_CharEventTrampolineLoc++;

    // The call to be patched
    auto CL_CharEventHookLoc = Findpattern(
        Pattern::Textsegment,
        "49 C1 E8 20 48 C1 EA 20 33 C9 45 8B C8 44 8B C3 E8 ? ? ? ? EB B3 8B 54 24 2C 33 C9 E8 ? ? ? ? EB A6"
    );
    CL_CharEventHookLoc += 29;

    // Set the jump back address for the stub
    CL_CharEventHook_JumpBack = CL_CharEventHookLoc + 5;

    // Redirect call to our trampoline
    Hooking::Hook::Set<std::uint8_t>(CL_CharEventHookLoc, 0xEB);
    Hooking::Hook::Set<std::uint32_t>(CL_CharEventHookLoc + 1, static_cast<std::uint32_t>(CL_CharEventTrampolineLoc - CL_CharEventHookLoc - 5));

    // Redirect our trampoline to our stub
    Hooking::Hook::Jump(CL_CharEventTrampolineLoc, CL_CharEventHookStub);

    printf("%llX\n", CL_CharEventTrampolineLoc);*/
}
 