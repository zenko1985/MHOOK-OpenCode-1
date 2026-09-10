#pragma once
#include <Windows.h>
// Windows 11 24H2: динамическая загрузка XInput вместо статической линковки
// Позволяет работать с разными версиями XInput (1.3, 1.4) без привязки к конкретной DLL
// Попытка загрузить: XInput1_4.dll → XInput9_1_0.dll → XInput1_3.dll
typedef DWORD (WINAPI* XInputGetStateFunc)(DWORD dwUserIndex, void* pState);
class MHGamepad
{
public:
    static bool Initialize();
    static void Update();
    static void Shutdown();
    static bool IsConnected(DWORD userIndex);
    static void SetEnabled(bool en);
    static bool IsEnabled();
    static const DWORD* GetXInputButtons();
private:
    static bool enabled;
    static DWORD prevButtons[4];
    static int deadzoneX;
    static int deadzoneY;
    static int sensitivity;
    static void HandleGamepad(DWORD userIndex, void* state);
    static void SimulateMouseMove(short stickX, short stickY);
    static void HandleButton(DWORD userIndex, WORD xinputButton, bool pressed, WORD actionScancode);
    // Динамическая загрузка XInput
    static HMODULE hXInput;
    static XInputGetStateFunc pXInputGetState;
    static bool LoadXInput();
    static void UnloadXInput();
};