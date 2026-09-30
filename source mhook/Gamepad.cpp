#include "Gamepad.h"
#include "Settings.h"
#include "Scancode.h"
bool MHGamepad::enabled = false;
DWORD MHGamepad::prevButtons[4] = {0};
int MHGamepad::deadzoneX = 7849; // XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE (getDefault)
int MHGamepad::deadzoneY = 7849;
int MHGamepad::sensitivity = 3;
HMODULE MHGamepad::hXInput = NULL;
XInputGetStateFunc MHGamepad::pXInputGetState = NULL;
// Константы XInput (определяем вручную, чтобы не включать Xinput.h)
#define GP_A        0
#define GP_B        1
#define GP_X        2
#define GP_Y        3
#define GP_LB       4
#define GP_RB       5
#define GP_LTHUMB   6
#define GP_RTHUMB   7
#define GP_START    8
#define GP_BACK     9
#define GP_DPAD_UP      10
#define GP_DPAD_DOWN    11
#define GP_DPAD_LEFT    12
#define GP_DPAD_RIGHT   13
#define GP_NUM_BUTTONS  14
// XInput button masks
#define MY_XINPUT_GAMEPAD_A              0x1000
#define MY_XINPUT_GAMEPAD_B              0x2000
#define MY_XINPUT_GAMEPAD_X              0x4000
#define MY_XINPUT_GAMEPAD_Y              0x8000
#define MY_XINPUT_GAMEPAD_LEFT_SHOULDER  0x0100
#define MY_XINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define MY_XINPUT_GAMEPAD_LEFT_THUMB     0x0040
#define MY_XINPUT_GAMEPAD_RIGHT_THUMB    0x0080
#define MY_XINPUT_GAMEPAD_START          0x0010
#define MY_XINPUT_GAMEPAD_BACK           0x0020
#define MY_XINPUT_GAMEPAD_DPAD_UP        0x0001
#define MY_XINPUT_GAMEPAD_DPAD_DOWN      0x0002
#define MY_XINPUT_GAMEPAD_DPAD_LEFT      0x0004
#define MY_XINPUT_GAMEPAD_DPAD_RIGHT     0x0008
// XInput state structure (layout-совместимая с XINPUT_STATE)
typedef struct {
    DWORD dwPacketNumber;
    struct {
        WORD wButtons;
        BYTE bLeftTrigger;
        BYTE bRightTrigger;
        SHORT sThumbLX;
        SHORT sThumbLY;
        SHORT sThumbRX;
        SHORT sThumbRY;
    } Gamepad;
} MY_XINPUT_STATE;
static const DWORD gp_xinput_buttons[GP_NUM_BUTTONS] = {
	MY_XINPUT_GAMEPAD_A, MY_XINPUT_GAMEPAD_B, MY_XINPUT_GAMEPAD_X, MY_XINPUT_GAMEPAD_Y,
	MY_XINPUT_GAMEPAD_LEFT_SHOULDER, MY_XINPUT_GAMEPAD_RIGHT_SHOULDER,
	MY_XINPUT_GAMEPAD_LEFT_THUMB, MY_XINPUT_GAMEPAD_RIGHT_THUMB,
	MY_XINPUT_GAMEPAD_START, MY_XINPUT_GAMEPAD_BACK,
	MY_XINPUT_GAMEPAD_DPAD_UP, MY_XINPUT_GAMEPAD_DPAD_DOWN,
	MY_XINPUT_GAMEPAD_DPAD_LEFT, MY_XINPUT_GAMEPAD_DPAD_RIGHT
};
const DWORD* MHGamepad::GetXInputButtons()
{
	return gp_xinput_buttons;
}
//======================================================================
// Динамическая загрузка XInput: XInput1_4.dll → XInput9_1_0.dll → XInput1_3.dll
//======================================================================
bool MHGamepad::LoadXInput()
{
	if(pXInputGetState) return true; // уже загружен
	// Пробуем разные версии XInput (от newest к oldest)
	const wchar_t* dllNames[] = {
		L"XInput1_4.dll",  // Windows 10/11 (предпочтительно)
		L"XInput9_1_0.dll", // Windows Vista+ (minimal)
		L"XInput1_3.dll",  // DirectX SDK
		NULL
	};
	for(int i = 0; dllNames[i] != NULL; i++)
	{
		hXInput = LoadLibraryW(dllNames[i]);
		if(hXInput)
		{
			pXInputGetState = (XInputGetStateFunc)GetProcAddress(hXInput, "XInputGetState");
			if(pXInputGetState) return true;
			FreeLibrary(hXInput);
			hXInput = NULL;
		}
	}
	return false;
}
void MHGamepad::UnloadXInput()
{
	if(hXInput)
	{
		FreeLibrary(hXInput);
		hXInput = NULL;
	}
	pXInputGetState = NULL;
}
//======================================================================
bool MHGamepad::Initialize()
{
	if (!MHSettings::flag_gamepad_enabled) return false;
	sensitivity = MHSettings::gamepad_sensitivity;
	if(!LoadXInput()) return false;
	for (DWORD i = 0; i < 4; i++)
	{
		MY_XINPUT_STATE state;
		if (pXInputGetState(i, &state) == 0) // ERROR_SUCCESS = 0
		{
			enabled = true;
			return true;
		}
	}
	return false;
}
void MHGamepad::Update()
{
	if (!enabled || !pXInputGetState) return;
	sensitivity = MHSettings::gamepad_sensitivity;
	for (DWORD i = 0; i < 4; i++)
	{
		MY_XINPUT_STATE state;
		if (pXInputGetState(i, &state) == 0)
		{
			HandleGamepad(i, &state);
		}
	}
}
void MHGamepad::Shutdown()
{
	enabled = false;
	UnloadXInput();
}
bool MHGamepad::IsConnected(DWORD userIndex)
{
	if (userIndex >= 4 || !pXInputGetState) return false;
	MY_XINPUT_STATE state;
	return pXInputGetState(userIndex, &state) == 0;
}
void MHGamepad::SetEnabled(bool en)
{
	enabled = en;
}
bool MHGamepad::IsEnabled()
{
	return enabled;
}
void MHGamepad::HandleGamepad(DWORD userIndex, void* statePtr)
{
	MY_XINPUT_STATE* state = (MY_XINPUT_STATE*)statePtr;
	short stickX = state->Gamepad.sThumbLX;
	short stickY = state->Gamepad.sThumbLY;
	if (stickX > deadzoneX || stickX < -deadzoneX ||
		stickY > deadzoneY || stickY < -deadzoneY)
	{
		SimulateMouseMove(stickX, stickY);
	}
	WORD buttons = state->Gamepad.wButtons;
	for (int i = 0; i < GP_NUM_BUTTONS; i++)
	{
		WORD action = MHSettings::gamepad_mapping[i];
		HandleButton(userIndex, (WORD)gp_xinput_buttons[i], (buttons & gp_xinput_buttons[i]) != 0, action);
	}
	prevButtons[userIndex] = buttons;
}
void MHGamepad::SimulateMouseMove(short stickX, short stickY)
{
	double moveX = (stickX / 32767.0) * sensitivity;
	double moveY = (stickY / 32767.0) * sensitivity;
	INPUT input = {0};
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = (LONG)moveX;
	input.mi.dy = (LONG)moveY;
	SendInput(1, &input, sizeof(INPUT));
}
static void SendMouseButton(DWORD downFlag, DWORD upFlag, bool pressed)
{
	INPUT input = {0};
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = pressed ? downFlag : upFlag;
	input.mi.dwExtraInfo = 0;
	input.mi.mouseData = 0;
	input.mi.time = 0;
	input.mi.dx = 0;
	input.mi.dy = 0;
	SendInput(1, &input, sizeof(INPUT));
}
static void SendScroll(short delta)
{
	INPUT input = {0};
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	input.mi.mouseData = delta;
	input.mi.dwExtraInfo = 0;
	input.mi.time = 0;
	input.mi.dx = 0;
	input.mi.dy = 0;
	SendInput(1, &input, sizeof(INPUT));
}
static void SendKeyboardKey(WORD scancode, bool pressed)
{
	INPUT input = {0};
	input.type = INPUT_KEYBOARD;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	if (!pressed) input.ki.dwFlags |= KEYEVENTF_KEYUP;
	if (scancode > 0xFF)
		input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
	input.ki.wScan = scancode;
	SendInput(1, &input, sizeof(INPUT));
}
void MHGamepad::HandleButton(DWORD userIndex, WORD xinputButton, bool pressed, WORD actionScancode)
{
	bool wasPressed = (prevButtons[userIndex] & xinputButton) != 0;
	if (pressed && !wasPressed)
	{
		switch (actionScancode)
		{
		case SC_NONE:
			break;
		case SC_LMOUSE:
			SendMouseButton(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, true);
			break;
		case SC_RMOUSE:
			SendMouseButton(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, true);
			break;
		case SC_MIDDLEMB:
			SendMouseButton(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, true);
			break;
		case SC_WHEEL_UP:
			SendScroll(WHEEL_DELTA);
			break;
		case SC_WHEEL_DOWN:
			SendScroll(-WHEEL_DELTA);
			break;
		default:
			SendKeyboardKey(actionScancode, true);
			break;
		}
	}
	else if (!pressed && wasPressed)
	{
		switch (actionScancode)
		{
		case SC_NONE:
		case SC_WHEEL_UP:
		case SC_WHEEL_DOWN:
			break;
		case SC_LMOUSE:
			SendMouseButton(MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, false);
			break;
		case SC_RMOUSE:
			SendMouseButton(MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP, false);
			break;
		case SC_MIDDLEMB:
			SendMouseButton(MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP, false);
			break;
		default:
			SendKeyboardKey(actionScancode, false);
			break;
		}
	}
}