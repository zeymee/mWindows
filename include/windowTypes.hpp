#pragma once
#include <Windows.h>
#include <ShellScalingApi.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <bitset>
#include <utility>

enum class MouseState : uint8_t
{
	eLeft = 0,
	eRight = 1,
	eMiddle = 2,
	eX1 = 3,
	eX2 = 4
};

class MouseButtonState
{
	uint8_t mask{ 0 };
public:
	MouseButtonState() = default;

	void add(MouseState state)
	{ mask |= (1 << static_cast<uint8_t>(state)); }
	void remove(MouseState state)
	{ mask &= ~(1 << static_cast<uint8_t>(state)); }

	bool contains(MouseState state) const
	{ return (mask & (1 << static_cast<uint8_t>(state))) != 0; }

};

class MouseInput
{
	int32_t dX{ 0 }, dY{ 0 };
	MouseButtonState mouseState;

	int32_t posX{ 0 }, posY{ 0 };
	int32_t wheelDelta{ 0 };

public:
	void resetDelta()
	{ dX = 0, dY = 0; }

	void resetWheelDelta()
	{ wheelDelta = 0; }

	void resetPos()
	{ posX = 0, posY = 0; }

	void setMouseState(const MouseButtonState& state)
	{ mouseState = state; }

	void setDelta(int32_t dX, int32_t dY)
	{
		this->dX = dX;
		this->dY = dY;
	}
	void addDelta(int32_t dX, int32_t dY)
	{
		this->dX += dX;
		this->dY += dY;
	}

	void setWheelDelta(int32_t wheelDelta)
	{ this->wheelDelta = wheelDelta; }
	void addWheelDelta(int32_t wheelDelta)
	{ this->wheelDelta += wheelDelta; }

	void setPos(int32_t posX, int32_t posY)
	{
		this->posX = posX;
		this->posY = posY;
	}

	void getDelta(int32_t& dX, int32_t& dY) const
	{
		dX = this->dX;
		dY = this->dY;
	}
	void getPos(int32_t& posX, int32_t& posY) const
	{
		posX = this->posX;
		posY = this->posY;
	}

	int32_t getWheelDelta() const { return wheelDelta; }

	MouseButtonState getMouseState() const { return mouseState; }
	
	MouseInput() = default;
};

enum KeyState : bool
{
	ePressed = true,
	eReleased = false
};

class KeyboardInput
{
	std::bitset<256> keyState{};

public:
	void setKeyState(uint8_t key, KeyState state)
	{ keyState[key] = (state == KeyState::ePressed); }

	std::bitset<256> getKeyState() const
	{ return keyState; }

	bool isKeyPressed(uint8_t key) const
	{ return keyState[key]; }
};

struct UserInput
{
	KeyboardInput keyboardInput;
	MouseInput mouseInput;
	void reset()
	{
		keyboardInput = KeyboardInput();
		mouseInput = MouseInput();
	}
};

struct WindowHandle
{
	HWND hWnd{ nullptr };
	HINSTANCE hInstance{ nullptr };
};

struct MonitorHandle
{
	HMONITOR hMonitor{ nullptr };
};

struct WindowPos
{

	int32_t width{ INT32_MIN }, height{ INT32_MIN };
	int32_t posX{ INT32_MIN }, posY{ INT32_MIN };

	bool isEmpty() const
	{ return width == INT32_MIN && height == INT32_MIN && posX == INT32_MIN && posY == INT32_MIN; }
	void operator=(const WindowPos& other)
	{
		width = (other.width == INT32_MIN) ? width : other.width;
		height = (other.height == INT32_MIN) ? height : other.height;
		posX = (other.posX == INT32_MIN) ? posX : other.posX;
		posY = (other.posY == INT32_MIN) ? posY : other.posY;
	}
};

struct MonitorInfo
{
	MonitorHandle handle;
	WindowPos pos;
	uint32_t dpiX{ 0 }, dpiY{ 0 };
	float aspectRatio{ 0.f };
	float refreshRate{ 0.f };
	std::wstring name;

	void initializeMonitorInfo(const WindowHandle& windowHandle)
	{
		handle.hMonitor = MonitorFromWindow(windowHandle.hWnd, MONITOR_DEFAULTTONEAREST);
		if (handle.hMonitor)
		{
			MONITORINFOEXW monitorInfo{};
			monitorInfo.cbSize = sizeof(MONITORINFOEXW);
			if (GetMonitorInfoW(handle.hMonitor, &monitorInfo))
			{
				pos.width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
				pos.height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
				pos.posX = monitorInfo.rcMonitor.left;
				pos.posY = monitorInfo.rcMonitor.top;
				name = monitorInfo.szDevice;
				aspectRatio = static_cast<float>(pos.width) / static_cast<float>(pos.height);
			}
			HDC hdc = GetDC(windowHandle.hWnd);
			if(hdc)
			{
				dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
				dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
				refreshRate = static_cast<float>(GetDeviceCaps(hdc, VREFRESH));
				ReleaseDC(windowHandle.hWnd, hdc);
			}
		}
	}
};

enum class WindowState : uint8_t
{
	eNone,
	eMinimized,
	eWindowed,
	eWindowedFullscreen,
	eFullscreen,
	eHidden
};

enum WindowEventType : uint8_t
{
	eNone = 0,
	eClose = 1,
	eSizing = 2,
	eResize = 3,
	eFocus = 4,
	eLostFocus = 5,
	eMinimize = 6,
	eMaximize = 7,
	eRestore = 8,
	eMove = 9,
	MAX = 10
};

using WindowCallbackClose = void(void* pUserData);
using WindowCallbackResize = void(void* pUserData, int32_t topLeftX, int32_t topLeftY, int32_t width, int32_t height);
using WindowCallbackFocus = void(void* pUserData);
using WindowCallbackLostFocus = void(void* pUserData);
using WindowCallbackMinimize = void(void* pUserData);
using WindowCallbackMaximize = void(void* pUserData, int32_t width, int32_t height);
using WindowCallbackRestore = void(void* pUserData, int32_t width, int32_t height);

struct WindowStateInfo
{
	WindowState state{ WindowState::eNone };
	WindowPos pos{ 0, 0, 0, 0 };
};

struct UWindowCallback 
{ void* userData{ nullptr }; };
struct CloseCallback : public UWindowCallback 
{ };

struct SizingCallback : public UWindowCallback
{
	int32_t topLeftX{ 0 }, topLeftY{ 0 };
	int32_t width{ 0 }, height{ 0 };
};
	
struct ResizeCallback : public UWindowCallback
{ int32_t width{ 0 }, height{ 0 }; };
struct FocusCallback : public UWindowCallback
{ };
struct LostFocusCallback : public UWindowCallback
{ };
struct MinimizeCallback : public UWindowCallback
{ };
struct MaximizeCallback : public UWindowCallback
{ uint32_t width{ 0 }, height{ 0 }; };
struct RestoreCallback : public UWindowCallback
{ uint32_t width{ 0 }, height{ 0 }; };
struct MoveCallback : public UWindowCallback
{ };

template<typename T>
concept WindowCallback = std::is_base_of_v<UWindowCallback, T>;

template<WindowCallback T>
WindowEventType getWindowEventType();

template<>
inline WindowEventType getWindowEventType<CloseCallback>()
{ return WindowEventType::eClose; }
template<>
inline WindowEventType getWindowEventType<ResizeCallback>()
{ return WindowEventType::eResize; }
template<>
inline WindowEventType getWindowEventType<FocusCallback>()
{ return WindowEventType::eFocus; }
template<>
inline WindowEventType getWindowEventType<LostFocusCallback>()
{ return WindowEventType::eLostFocus; }
template<>
inline WindowEventType getWindowEventType<MinimizeCallback>()
{ return WindowEventType::eMinimize; }
template<>
inline WindowEventType getWindowEventType<MaximizeCallback>()
{ return WindowEventType::eMaximize; }
template<>
inline WindowEventType getWindowEventType<RestoreCallback>()
{ return WindowEventType::eRestore; }
template<>
inline WindowEventType getWindowEventType<MoveCallback>()
{ return WindowEventType::eMove; }
template<>
inline WindowEventType getWindowEventType<SizingCallback>()
{ return WindowEventType::eSizing; }
template<>
inline WindowEventType getWindowEventType<UWindowCallback>()
{ return WindowEventType::eNone; }

template<WindowCallback T>
using WindowCallbackFnc = void(*)(T*);

using genericCallback = void(*)();

template<class T>
struct enableBitmaskOperator : std::false_type {};

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T operator|(T lhs, T rhs)
{ return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) | static_cast<std::underlying_type_t<T>>(rhs)); }

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T operator&(T lhs, T rhs)
{ return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) & static_cast<std::underlying_type_t<T>>(rhs)); }

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T operator^(T lhs, T rhs)
{ return static_cast<T>(static_cast<std::underlying_type_t<T>>(lhs) ^ static_cast<std::underlying_type_t<T>>(rhs)); }

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T operator~(T lhs)
{ return static_cast<T>(~static_cast<std::underlying_type_t<T>>(lhs)); }

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T& operator|=(T& lhs, T rhs)
{ lhs = lhs | rhs; return lhs; }

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T& operator&=(T& lhs, T rhs)
{ lhs = lhs & rhs; return lhs; }

template<class T>
requires enableBitmaskOperator<T>::value
constexpr T& operator^=(T& lhs, T rhs)
{ lhs = lhs ^ rhs; return lhs; }

enum class WindowStateError : uint16_t
{
	eSuccess = 0,
	eSizeIgnored = 1,
	ePositionIgnored = 2,
	eStateIgnored = 4
};

enum class WindowTitleError : uint16_t
{
	eSuccess = 0,
	eInvalid = 1
};

template<> struct enableBitmaskOperator<WindowTitleError> : std::true_type {};
template<> struct enableBitmaskOperator<WindowStateError> : std::true_type {};