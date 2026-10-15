#pragma once
#include "windowTypes.hpp"

struct WindowInfo
{
	uint32_t width_, height_;
	std::wstring title_;
	HWND hwnd_;
	HINSTANCE hInstance_;
};

class Window
{
	UserInput userInput_;
	WindowInfo info_;
	RAWINPUTDEVICE rid_[1];

	LRESULT winEvent(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	static LRESULT WINAPI winProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	void init();
public:
	Window(uint32_t width, uint32_t height, std::wstring title);
	bool pullEvents(UserInput& userInput);
	WindowInfo& getInfo() { return info_; }
};