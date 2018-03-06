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
static vec4_t colorGreen = { 0.0f, 1.0f, 0.0f, 1.0f };

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

static Function<dvar_t*(const char*)> Dvar_FindVar;

dvar_t** sortedDvars; // =  reinterpret_cast<dvar_t**>(0x14B304A50);

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

std::vector < std::string > Console_FindCommands(const std::string& input, const std::size_t max = 20)
{
    std::vector < std::string > foundSuggestions;

    // loop through commands

    return foundSuggestions;
}

std::vector < std::string > Console_FindDvars(const std::string& input, const std::size_t max = 20)
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
                if (foundSuggestions.size() >= max)
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

void Console_DrawInfo(const std::string& input)
{
    auto font = R_RegisterFont("fonts/consoleFont");
    auto console = Material_RegisterHandle("console");
    auto placement = ScrPlace_GetActivePlacement();
    auto dvar = Dvar_FindVar(&input[0]);

    if (dvar)
    {
        // draw suggestions
        UI_DrawHandlePic(placement, 5.0 + 40.0, 25.0, 590.0, 5.0 + (10.0 * 3), 4, 0, colorWhite, console);
        UI_DrawText(placement, dvar->name, strlen(dvar->name), font, 10.0 + 55.0, 15.6 + 20.0, 1, 1, 0.1, colorYellow, 0);
        UI_DrawText(placement, "Dvar value here, default value here", 36, font, 10.0 + 55.0, 15.6 + 30.0, 1, 1, 0.1, colorYellow, 0);
        UI_DrawText(placement, "Value is any x between y and z", 31, font, 10.0 + 55.0, 15.6 + 40.0, 1, 1, 0.1, colorGreen, 0);

        // UI_DrawText(placement, &foundSuggestions[i].first[0], foundSuggestions[i].first.size(), font, 10.0 + 55.0, 15.6 + 20.0 + (10.0 * i), 1, 1, 0.1, foundSuggestions[i].second == 1 ? colorCyan : colorYellow, 0);
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

void Console_OnFrame()
{
    if (Key_IsCatcherActive(0, 1))
    {
        Console_DrawInputBox();

        if (!console_input.empty())
        {
            std::size_t index;
            if ((index = console_input.find_first_of(' ')) != std::string::npos && index > 0)
            {
                Console_DrawInfo(console_input.substr(0, index));
            }
            else
            {
                Console_DrawSuggestions();
            }
        }
    }
}

static std::function<std::uint8_t(std::uint32_t a1, std::uint32_t a2)> SCR_DrawScreenField;
std::uint8_t SCR_DrawScreenFieldHook(std::uint32_t a1, std::uint32_t a2)
{
    auto value = SCR_DrawScreenField(a1, a2);
    Console_OnFrame();

    return value;
}

static std::function<void(std::uint64_t a1, std::uint32_t key, std::uint32_t down, std::uint32_t a4)> CL_KeyEvent;
void CL_KeyEventHook(std::uint64_t a1, std::uint32_t key, std::uint32_t down, std::uint32_t a4)
{
    printf("key %i\n", key);

    if (down && (key == '~' || key == '`'))
    {
        console_input.clear();
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

        // tab logic
        if (!down && key == 9)
        {
            auto dvars = Console_FindDvars(console_input, 1);
            auto cmds = Console_FindCommands(console_input, 1);

            // commands are prioritized in the suggestions popup over dvars, so check commands first.
            if (cmds.size())
            {
                console_input = cmds[0] + ' ';
            }
            else if (dvars.size())
            {
                console_input = dvars[0] + ' ';
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
    Dvar_FindVar = Findpattern(
        Pattern::Textsegment,
        "48 89 5C 24 ? 57 48 83 EC 20 48 8B F9 F0 FF 05 ? ? ? ? 8B 05 ? ? ? ? 85 C0 74 13 66 90 33 C9 E8 ? ? ? ? 8B 05 ? ? ? ? 85 C0 75 EF"
    );

    // Dvar list
    auto DvarFuncPattern = Findpattern(
        Pattern::Textsegment,
        "33 DB 39 1D ? ? ? ? 7E 2C 48 89 7C 24 ? 48 8D 3D ? ? ? ? 0F 1F 44 00 ? 48 8B 0F 48 8B D6 FF D5 FF C3 48 8D 7F 08 3B 1D ? ? ? ? 7C EA 48 8B 7C 24 ?"
    );
    DvarFuncPattern += 16;
    sortedDvars = reinterpret_cast<dvar_t**>(DvarFuncPattern + *reinterpret_cast<std::uint32_t*>(DvarFuncPattern + 2) + 6);

    // Hooks
    SCR_DrawScreenField = Hooking::Hook::Detour(
        Findpattern(
            Pattern::Textsegment,
            "48 89 5C 24 ? 56 48 83 EC 20 8B F2 8B D9 E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 83 3D ? ? ? ? ? 75 20 45 33 C9 0F 57 D2 48 8D 15 ? ? ? ? 41 8D 49 04 48 8B 5C 24 ? 48 83 C4 20 5E E9 ? ? ? ?"
        ),
        SCR_DrawScreenFieldHook,
        12
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
 