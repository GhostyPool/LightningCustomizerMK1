// dllmain.cpp : Defines the entry point for the DLL application.
#include "SDK.h"
#include "unreal\UObject.h"
#include <Windows.h>
#include <mutex>
#include <iostream>

static bool s_bFailed = false;
static bool s_bEnabled = false;
static float s_fLightningColour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static std::mutex mtx_lightningColour;

extern "C" PLUGIN_API const char* GetPluginName()
{
    return "LightningCustomizerMK1";
}

extern "C" PLUGIN_API const char* GetPluginProject()
{
    return "MK12HOOK";
}

extern "C" PLUGIN_API const char* GetPluginTabName()
{
    return "Lightning customizer";
}

static void PrintError(const char* msg)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    bool bFailed = false;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi))
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    else
        bFailed = true;

    printf("LightningCustomizerMK1:ERROR | %s", msg);

    if (!bFailed)
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);
    return;
}

static void AActor_BeginPlay(__int64 actorPtr);
static void(*oBeginPlay)(__int64) = nullptr;
static UObject* (*StaticFindObject)(UClass*, UObject*, const wchar_t*, bool) = nullptr;
static bool (*ClassIsChildOf)(UClass*, UClass*) = nullptr;

extern "C" PLUGIN_API void OnInitialize()
{
    MK12HOOKSDK::Initialize();
    
    if (!MK12HOOKSDK::IsOK())
        s_bFailed = true;
    
    if (!s_bFailed)
    {
        uintptr_t beginPlayPat = MK12HOOKSDK::GetPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? ? ? ? 48 8B F1 F3 0F 10 89", 0);
        StaticFindObject = (UObject*(*)(UClass*, UObject*, const wchar_t*, bool))MK12HOOKSDK::GetPattern("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC ? 80 3D ? ? ? ? 00 45 0F B6 F1", 0);

        ClassIsChildOf = (bool(*)(UClass*, UClass*))MK12HOOKSDK::GetPattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B DA 48 8B F9 48 85 D2 0F 84 ? ? ? ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8D 50 ? 48 63 40 ? 3B 43 ? 0F 8F ? ? ? ? 48 8B C8 48 8B 43 ? ? ? ? ? 0F 85 ? ? ? ? 48 85 FF", 0);

        if (!StaticFindObject || !ClassIsChildOf || !beginPlayPat || MK12HOOKSDK::CreateHook((void*)beginPlayPat, &AActor_BeginPlay, (void**)&oBeginPlay) != 0)
            s_bFailed = true;
    }

    if (s_bFailed)
        PrintError("Could not start as EHP plugin!");
}

struct FLinearColor
{
    float R,
          G,
          B,
          A;
};

struct MKCharacterActor
{
    char pad1[0x528];
    FLinearColor SkinFXColorProperties;
};

static void AActor_BeginPlay(__int64 actorPtr)
{
    if (!s_bEnabled)
        goto leave;

    UObject* raidenClass = StaticFindObject(nullptr, nullptr, L"/Game/Disk/Char/Raiden/Template/Blueprint/BP_Raiden_Char.BP_Raiden_Char_C", false);
    if (!raidenClass)
        goto leave;

    UObject* actor = reinterpret_cast<UObject*>(actorPtr);

    if (ClassIsChildOf(actor->Class, static_cast<UClass*>(raidenClass)))
    {
        MKCharacterActor* mkActor = reinterpret_cast<MKCharacterActor*>(actor);

        std::lock_guard<std::mutex> lock(mtx_lightningColour);
        mkActor->SkinFXColorProperties.R = s_fLightningColour[0];
        mkActor->SkinFXColorProperties.G = s_fLightningColour[1];
        mkActor->SkinFXColorProperties.B = s_fLightningColour[2];
        mkActor->SkinFXColorProperties.A = s_fLightningColour[3];
    }

leave:
    if (oBeginPlay)
        oBeginPlay(actorPtr);
}

extern "C" PLUGIN_API void TabFunction()
{
    if (!MK12HOOKSDK::IsOK() || s_bFailed)
        return;

    MK12HOOKSDK::ImGui_Checkbox("Enable##lc_enable", &s_bEnabled);

    if (MK12HOOKSDK::ImGui_ColorEdit4("Lightning colour##lc", s_fLightningColour))
        std::lock_guard<std::mutex> lock(mtx_lightningColour);

    MK12HOOKSDK::ImGui_Text("The colour change takes effect on character reload.");
}

extern "C" PLUGIN_API void OnShutdown()
{
}

extern "C" PLUGIN_API void OnFrameTick()
{
}

extern "C" PLUGIN_API void OnFightStartup()
{
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

