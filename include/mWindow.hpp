#pragma once
#include "windowTypes.hpp"
#include <functional>

#include <array>
#include <type_traits>
#include <utility>

class mWindow
{
	UserInput userInput_;
	MonitorInfo monitorInfo_;
	WindowHandle windowHandle_;
	WindowStateInfo windowState_;
	std::wstring title_;
	std::wstring className_;
	// Layout:
	// 1 - Mouse
	// 2 - Keyboard
	RAWINPUTDEVICE rid_[2];

	LRESULT winEvent(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK winProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	struct WindowCallbackData
	{
		genericCallback callback{ nullptr };
		void* userData{ nullptr };
		bool isEmpty() const { return callback == nullptr || userData == nullptr; }
	};
	std::array<WindowCallbackData, WindowEventType::MAX> windowCallbacks_;

	void setWindowState(const WindowState& state);
	void setWindowPos(const WindowPos& pos);

public:

	mWindow(uint32_t width, uint32_t height, std::wstring title);
	~mWindow();
	mWindow(const mWindow&) = delete;
	mWindow& operator=(const mWindow&) = delete;
	bool pullEvents();

	void setWindowStyle(WindowStateInfo style);
	WindowTitleError setWindowTitle(const std::wstring& title);

	const UserInput& getUserInput() const { return userInput_; }
	const MonitorInfo& getMonitorInfo() const { return monitorInfo_; }
	const WindowHandle& getWindowHandle() const { return windowHandle_; }
	const WindowStateInfo& getWindowState() const { return windowState_; }

	template<WindowCallback W>
	void AddEventWindowCallback(WindowCallbackFnc<W> callback, void* pUserData = nullptr)
	{
		auto eventType = getWindowEventType<W>();

		windowCallbacks_[eventType].callback = reinterpret_cast<genericCallback>(callback);
		windowCallbacks_[eventType].userData = pUserData;
	}
};
