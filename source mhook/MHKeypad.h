// четыре клавиши, которые может нажимать. Возможно, две соседние
#ifndef __MH_KEYPAD
#define __MH_KEYPAD
class MHKeypad
{
public:
	static void Init(const WORD (&scancodes)[17]);
	static void Reset(int shift=0);
	static __forceinline int GetPosition(){return keypad_position;};
	static void Press(int position, bool down, int shift=0);
	static bool Press4(int position, bool down, int shift=0);
	static void Press8(int position, bool down);
	static void Tap(int position, int shift=0);
protected:
	static int keypad_position;
	static WORD scancode[17];
	static int PressKeyToInput(int position, bool down, int shift, INPUT *out);
};
#endif