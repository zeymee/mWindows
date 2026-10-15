#include "mWindow.hpp"
#include <vector>

LRESULT Window::winEvent(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{

	switch (umsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
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
				userInput_.mouseInput.dX += static_cast<int32_t>(raw->data.mouse.lLastX);
				userInput_.mouseInput.dY += static_cast<int32_t>(raw->data.mouse.lLastY);

				if (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
					userInput_.mouseInput.wheelDelta += static_cast<int16_t>(raw->data.mouse.usButtonData);

				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_1_DOWN)
				{
					userInput_.mouseInput.key = MouseState::eLeft;
					userInput_.mouseInput.state = KeyState::ePressed;
				}
				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_1_UP)
				{
					userInput_.mouseInput.key = MouseState::eLeft;
					userInput_.mouseInput.state = KeyState::eReleased;
				}
				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_2_DOWN)
				{
					userInput_.mouseInput.key = MouseState::eRight;
					userInput_.mouseInput.state = KeyState::ePressed;
				}
				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_2_UP)
				{
					userInput_.mouseInput.key = MouseState::eRight;
					userInput_.mouseInput.state = KeyState::eReleased;
				}
				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_3_DOWN)
				{
					userInput_.mouseInput.key = MouseState::eMiddle;
					userInput_.mouseInput.state = KeyState::ePressed;
				}
				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_3_UP)
				{
					userInput_.mouseInput.key = MouseState::eMiddle;
					userInput_.mouseInput.state = KeyState::eReleased;
				}
				if (raw->data.mouse.usButtonData & RI_MOUSE_BUTTON_4_DOWN)
				{

				}
			}
		}

	} break;
	case WM_KEYUP:
	{
		userInput_.keyboardInput.key = static_cast<uint8_t>(wparam);
		userInput_.keyboardInput.state = KeyState::eReleased;
	} break;
	case WM_KEYDOWN:
	{
		userInput_.keyboardInput.key = static_cast<uint8_t>(wparam);
		userInput_.keyboardInput.state = KeyState::ePressed;
	} break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

LRESULT __stdcall Window::winProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (umsg == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
		Window* pWnd = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		return 0;
	}
	else
	{
		Window* pWnd = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if(pWnd)
			pWnd->winEvent(hwnd, umsg, wparam, lparam);
		else return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void Window::init()
{
	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Window::winProc;
	wcex.hInstance = info_.hInstance_;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.lpszClassName = info_.title_.c_str();
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.cbClsExtra = 0;

	if (!RegisterClassExW(&wcex))
	{
		MessageBox(nullptr, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
		throw std::runtime_error("Failed to register window class");
	}

	info_.hwnd_ = CreateWindowExW(
		WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED,
		info_.title_.c_str(),
		info_.title_.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		info_.width_, info_.height_,
		nullptr,
		nullptr,
		info_.hInstance_,
		this
	);

	rid_[0].usUsagePage = 0x01;
	rid_[0].usUsage = 0x02; // Mouse
	rid_[0].dwFlags = RIDEV_INPUTSINK;
	rid_[0].hwndTarget = info_.hwnd_;

	if (!RegisterRawInputDevices(rid_, 1, sizeof(RAWINPUTDEVICE)))
	{
		MessageBox(nullptr, L"Failed to register raw input devices", L"Error", MB_OK | MB_ICONERROR);
		throw std::runtime_error("Failed to register raw input devices");
	}

	ShowWindow(info_.hwnd_, SW_SHOW);
}

Window::Window(uint32_t width, uint32_t height, std::wstring title) : 
	info_({ width, height, title, nullptr, GetModuleHandle(nullptr) }) 
{ init(); }

bool Window::pullEvents(UserInput& userInput)
{
	MSG msg{};
	while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}
	userInput = userInput_;
	userInput_ = UserInput();

	return true;
}
