#include "mWindow.hpp"
#include <vector>
#include <string>

LRESULT mWindow::winEvent(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_CLOSE:
			if (!windowCallbacks_[WindowEventType::eClose].isEmpty())
			{
				CloseCallback callback{ windowCallbacks_[WindowEventType::eClose].userData };
				reinterpret_cast<WindowCallbackFnc<CloseCallback>>(windowCallbacks_[WindowEventType::eClose].callback)(&callback);
				return 0;
			}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_SIZING:
		{
			RECT* pRect = reinterpret_cast<RECT*>(lparam);
			if(!windowCallbacks_[WindowEventType::eSizing].isEmpty())
			{
				SizingCallback callback;
				callback.userData = windowCallbacks_[WindowEventType::eSizing].userData;
				callback.topLeftX = pRect->left;
				callback.topLeftY = pRect->top;
				callback.width = pRect->right - pRect->left;
				callback.height = pRect->bottom - pRect->top;
				reinterpret_cast<WindowCallbackFnc<SizingCallback>>(windowCallbacks_[WindowEventType::eSizing].callback)(&callback);
			}
		} break;

		case WM_SIZE:
		{
			uint32_t width = LOWORD(lparam);
			uint32_t height = HIWORD(lparam);
			windowState_.pos.width = static_cast<int32_t>(width);
			windowState_.pos.height = static_cast<int32_t>(height);

			switch (wparam)
			{
				case SIZE_MINIMIZED:
					windowState_.state = WindowState::eMinimized;
					if (!windowCallbacks_[WindowEventType::eMinimize].isEmpty())
					{
						MinimizeCallback callback{ windowCallbacks_[WindowEventType::eMinimize].userData };
						reinterpret_cast<WindowCallbackFnc<MinimizeCallback>>(windowCallbacks_[WindowEventType::eMinimize].callback)(&callback);
					}
				break;
				case SIZE_MAXIMIZED:
					windowState_.state = WindowState::eWindowedFullscreen;
					if (!windowCallbacks_[WindowEventType::eMaximize].isEmpty())
					{
						MaximizeCallback callback{ windowCallbacks_[WindowEventType::eMaximize].userData, width, height };
						reinterpret_cast<WindowCallbackFnc<MaximizeCallback>>(windowCallbacks_[WindowEventType::eMaximize].callback)(&callback);
					}
				break;
				case SIZE_RESTORED:
					windowState_.state = WindowState::eWindowed;
					if (!windowCallbacks_[WindowEventType::eRestore].isEmpty())
					{
						RestoreCallback callback{ windowCallbacks_[WindowEventType::eRestore].userData, width, height };
						reinterpret_cast<WindowCallbackFnc<RestoreCallback>>(windowCallbacks_[WindowEventType::eRestore].callback)(&callback);
					}
				break;
			}

			if (wparam != SIZE_MINIMIZED && !windowCallbacks_[WindowEventType::eResize].isEmpty())
			{
				ResizeCallback callback{ windowCallbacks_[WindowEventType::eResize].userData, static_cast<int32_t>(width), static_cast<int32_t>(height) };
				reinterpret_cast<WindowCallbackFnc<ResizeCallback>>(windowCallbacks_[WindowEventType::eResize].callback)(&callback);
			}
		} break;

		case WM_MOVE:
		{
			windowState_.pos.posX = static_cast<int16_t>(LOWORD(lparam));
			windowState_.pos.posY = static_cast<int16_t>(HIWORD(lparam));
			monitorInfo_.initializeMonitorInfo(windowHandle_);
			if (!windowCallbacks_[WindowEventType::eMove].isEmpty())
			{
				MoveCallback callback{ windowCallbacks_[WindowEventType::eMove].userData };
				reinterpret_cast<WindowCallbackFnc<MoveCallback>>(windowCallbacks_[WindowEventType::eMove].callback)(&callback);
			}
		} break;
		
		case WM_KILLFOCUS:
		{
			if (!windowCallbacks_[WindowEventType::eLostFocus].isEmpty())
			{
				LostFocusCallback callback{ windowCallbacks_[WindowEventType::eLostFocus].userData };
				reinterpret_cast<WindowCallbackFnc<LostFocusCallback>>(windowCallbacks_[WindowEventType::eLostFocus].callback)(&callback);
			}
		} break;

		case WM_SETFOCUS:
		{
			if (!windowCallbacks_[WindowEventType::eFocus].isEmpty())
			{
				FocusCallback callback{ windowCallbacks_[WindowEventType::eFocus].userData };
				reinterpret_cast<WindowCallbackFnc<FocusCallback>>(windowCallbacks_[WindowEventType::eFocus].callback)(&callback);
			}
		} break;

		case WM_INPUT:
		{
		uint32_t dwSize{ 0u };
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
		std::unique_ptr<byte[]> rawData{ std::make_unique<byte[]>(dwSize) };
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, rawData.get(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
		{

			RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.get());
			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				MouseButtonState mouseButtonState = userInput_.mouseInput.getMouseState();
				
				userInput_.mouseInput.addDelta(
					static_cast<int32_t>(raw->data.mouse.lLastX),
					static_cast<int32_t>(raw->data.mouse.lLastY)
				);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					userInput_.mouseInput.addWheelDelta(static_cast<int16_t>(raw->data.mouse.usButtonData));

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN)
					mouseButtonState.add(MouseState::eLeft);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP)
					mouseButtonState.remove(MouseState::eLeft);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN)
					mouseButtonState.add(MouseState::eRight);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP)
					mouseButtonState.remove(MouseState::eRight);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN)
					mouseButtonState.add(MouseState::eMiddle);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP)
					mouseButtonState.remove(MouseState::eMiddle);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
					mouseButtonState.add(MouseState::eX1);
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
					mouseButtonState.remove(MouseState::eX1);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
					mouseButtonState.add(MouseState::eX2);
				if (raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
					mouseButtonState.remove(MouseState::eX2);

				userInput_.mouseInput.setMouseState(mouseButtonState);
			}
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				KeyboardInput& keyboardInput = userInput_.keyboardInput;
				if (raw->data.keyboard.Message == WM_KEYDOWN || raw->data.keyboard.Message == WM_SYSKEYDOWN)
					keyboardInput.setKeyState(static_cast<uint8_t>(raw->data.keyboard.VKey), KeyState::ePressed);
				if (raw->data.keyboard.Message == WM_KEYUP || raw->data.keyboard.Message == WM_SYSKEYUP)
					keyboardInput.setKeyState(static_cast<uint8_t>(raw->data.keyboard.VKey), KeyState::eReleased);
			}
		}
		} break;
	}

    return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT mWindow::winProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if(umsg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
		mWindow* pWnd = reinterpret_cast<mWindow*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		return 0;
	}
	else
	{
		mWindow* pWnd = reinterpret_cast<mWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if(pWnd) return pWnd->winEvent(hwnd, umsg, wparam, lparam);
		else return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void mWindow::setWindowState(const WindowState &state)
{
	if(windowState_.state == state)
		return;
	
	windowState_.state = state;

	LONG_PTR dStyle = GetWindowLongPtrW(windowHandle_.hWnd, GWL_STYLE);
	switch(state)
	{
		case WindowState::eMinimized:
			ShowWindow(windowHandle_.hWnd, SW_MINIMIZE);
			return;
		case WindowState::eWindowed:
			dStyle = (dStyle & ~WS_POPUP) | WS_OVERLAPPEDWINDOW | WS_VISIBLE;
			SetWindowLongPtrW(windowHandle_.hWnd, GWL_STYLE, dStyle);
			ShowWindow(windowHandle_.hWnd, SW_RESTORE);
			SetWindowPos(windowHandle_.hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			return;
		case WindowState::eWindowedFullscreen:
			dStyle = (dStyle & ~WS_OVERLAPPEDWINDOW) | WS_POPUP | WS_VISIBLE;
			SetWindowLongPtrW(windowHandle_.hWnd, GWL_STYLE, dStyle);
			ShowWindow(windowHandle_.hWnd, SW_MAXIMIZE);
			SetWindowPos(windowHandle_.hWnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			return;
		case WindowState::eFullscreen:
			// Implement fullscreen logic here
			return;
		case WindowState::eHidden:
			dStyle &= ~WS_VISIBLE;
			SetWindowLongPtrW(windowHandle_.hWnd, GWL_STYLE, dStyle);
			ShowWindow(windowHandle_.hWnd, SW_HIDE);
			return;
		default: return;
	}
}

void mWindow::setWindowPos(const WindowPos &pos)
{
	if(pos.isEmpty())
		return;

	if((
		pos.width != windowState_.pos.width ||
		pos.height != windowState_.pos.height ||
		pos.posX != windowState_.pos.posX ||
		pos.posY != windowState_.pos.posY
	) && windowState_.state == WindowState::eWindowed)
	{
		windowState_.pos = pos;
		SetWindowPos(windowHandle_.hWnd, nullptr, windowState_.pos.posX, windowState_.pos.posY, windowState_.pos.width, windowState_.pos.height, SWP_NOZORDER);
	}
}

void mWindow::setWindowStyle(WindowStateInfo style)
{
	if (style.state != WindowState::eNone)
		setWindowState(style.state);
	setWindowPos(style.pos);
}

mWindow::mWindow(uint32_t width, uint32_t height, std::wstring title)
	: title_(std::move(title))
	, className_(L"mWindow_" + std::to_wstring(reinterpret_cast<uintptr_t>(this)))
{
	auto& handle = windowHandle_;
	auto& winState = windowState_;

	auto& winPos = windowState_.pos;
	winPos.width = width, winPos.height = height;

	handle.hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = mWindow::winProc;
	wcex.hInstance = handle.hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.lpszClassName = className_.c_str();
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.cbClsExtra = 0;
	if(!RegisterClassExW(&wcex))
	{
		MessageBoxW(nullptr, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
		throw std::runtime_error("Failed to register window class");
	}
	
	handle.hWnd = CreateWindowExW(
		WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED,
		wcex.lpszClassName,
		title_.c_str(),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		winPos.width, winPos.height,
		nullptr,
		nullptr,
		handle.hInstance,
		this
	);
	if(!handle.hWnd)
	{
		MessageBoxW(nullptr, L"Failed to create window", L"Error", MB_OK | MB_ICONERROR);
		throw std::runtime_error("Failed to create window");
	}

	SetLayeredWindowAttributes(handle.hWnd, 0, 255, LWA_ALPHA);

	auto& mouse = rid_[0];
	mouse.usUsagePage = 0x01;
	mouse.usUsage = 0x02; // Mouse
	mouse.dwFlags = RIDEV_INPUTSINK;
	mouse.hwndTarget = handle.hWnd;

	auto& keyboard = rid_[1];
	keyboard.usUsagePage = 0x01;
	keyboard.usUsage = 0x06; // Keyboard
	keyboard.dwFlags = RIDEV_INPUTSINK;
	keyboard.hwndTarget = handle.hWnd;

	if (!RegisterRawInputDevices(rid_, 2, sizeof(RAWINPUTDEVICE)))
	{
		MessageBoxW(nullptr, L"Failed to register raw input devices", L"Error", MB_OK | MB_ICONERROR);
		throw std::runtime_error("Failed to register raw input devices");
	}

	ShowWindow(handle.hWnd, SW_SHOWNORMAL);
	UpdateWindow(handle.hWnd);
	SetForegroundWindow(handle.hWnd);
	BringWindowToTop(handle.hWnd);
	
	winState.state = WindowState::eWindowed;
	monitorInfo_.initializeMonitorInfo(windowHandle_);
}

mWindow::~mWindow()
{
	rid_[0].dwFlags = RIDEV_REMOVE;
	rid_[0].hwndTarget = nullptr;
	rid_[1].dwFlags = RIDEV_REMOVE;
	rid_[1].hwndTarget = nullptr;
	RegisterRawInputDevices(rid_, 2, sizeof(RAWINPUTDEVICE));

	if (windowHandle_.hWnd)
	{
		DestroyWindow(windowHandle_.hWnd);
		windowHandle_.hWnd = nullptr;
	}
	if (!className_.empty() && windowHandle_.hInstance)
		UnregisterClassW(className_.c_str(), windowHandle_.hInstance);

}

bool mWindow::pullEvents()
{
	userInput_.mouseInput.resetDelta();
	userInput_.mouseInput.resetWheelDelta();
	
	MSG msg{};
	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}
	{
		POINT cursorPos{};
		GetCursorPos(&cursorPos);
		userInput_.mouseInput.setPos(cursorPos.x, cursorPos.y);
	}
	return true;
}

WindowTitleError mWindow::setWindowTitle(const std::wstring& title)
{
	if (title.empty() || !windowHandle_.hWnd)
		return WindowTitleError::eInvalid;

	if (!SetWindowTextW(windowHandle_.hWnd, title.c_str()))
		return WindowTitleError::eInvalid;

	title_ = title;
	return WindowTitleError::eSuccess;
}
