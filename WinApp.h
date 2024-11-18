#pragma once

#include <cstdint>
#include <Windows.h>


class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM Lparam);
	
public:
	void Initialize();
	void Finalize();

	bool ProcessMassage();

	HWND GetHwnd() const {return hwnd;}
	HINSTANCE GetInstance() const {return wc.hInstance;}
	
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

private:
	

	bool ProcessMessage();

	HWND hwnd = nullptr;
	WNDCLASS wc{};
};

