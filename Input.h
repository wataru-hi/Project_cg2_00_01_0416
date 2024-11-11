#pragma once
#include <Windows.h>
#include <wrl.h>
#define DIRECTINPUT_VERSION 0x0800
#include<dinput.h>

using namespace Microsoft::WRL;

class Input
{
public:
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
private:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	ComPtr<IDirectInput8> directInput;
	ComPtr<IDirectInputDevice8> keyboard;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};
};

