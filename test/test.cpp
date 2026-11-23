#include "../include/mWindow.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

struct TestCallbackState
{
    HWND hWnd{ nullptr };
    bool allowClose{ false };

    int resizeCount{ 0 };
    int32_t lastResizeW{ 0 };
    int32_t lastResizeH{ 0 };

    int minimizeCount{ 0 };

    int maximizeCount{ 0 };
    uint32_t lastMaxW{ 0 };
    uint32_t lastMaxH{ 0 };

    int restoreCount{ 0 };
    uint32_t lastRestoreW{ 0 };
    uint32_t lastRestoreH{ 0 };

    int moveCount{ 0 };
    int closeCount{ 0 };
};

static void onResize(ResizeCallback* cb)
{
    auto* state = static_cast<TestCallbackState*>(cb->userData);
    ++state->resizeCount;
    state->lastResizeW = cb->width;
    state->lastResizeH = cb->height;
}

static void onMinimize(MinimizeCallback* cb)
{
    auto* state = static_cast<TestCallbackState*>(cb->userData);
    ++state->minimizeCount;
}

static void onMaximize(MaximizeCallback* cb)
{
    auto* state = static_cast<TestCallbackState*>(cb->userData);
    ++state->maximizeCount;
    state->lastMaxW = cb->width;
    state->lastMaxH = cb->height;
}

static void onRestore(RestoreCallback* cb)
{
    auto* state = static_cast<TestCallbackState*>(cb->userData);
    ++state->restoreCount;
    state->lastRestoreW = cb->width;
    state->lastRestoreH = cb->height;
}

static void onMove(MoveCallback* cb)
{
    auto* state = static_cast<TestCallbackState*>(cb->userData);
    ++state->moveCount;
}

static void onClose(CloseCallback* cb)
{
    auto* state = static_cast<TestCallbackState*>(cb->userData);
    ++state->closeCount;
    if (state->allowClose && state->hWnd)
    {
        DestroyWindow(state->hWnd);
    }
}

static std::wstring keyToName(uint8_t vk)
{
    switch (vk)
    {
        case VK_LWIN: return L"LWIN";
        case VK_RWIN: return L"RWIN";
        case VK_SHIFT: return L"SHIFT";
        case VK_LSHIFT: return L"LSHIFT";
        case VK_RSHIFT: return L"RSHIFT";
        case VK_CONTROL: return L"CTRL";
        case VK_LCONTROL: return L"LCTRL";
        case VK_RCONTROL: return L"RCTRL";
        case VK_MENU: return L"ALT";
        case VK_LMENU: return L"LALT";
        case VK_RMENU: return L"RALT";
        case VK_SPACE: return L"SPACE";
        case VK_RETURN: return L"ENTER";
        case VK_ESCAPE: return L"ESC";
        case VK_BACK: return L"BACK";
        case VK_TAB: return L"TAB";
        default: break;
    }

    UINT scanCode = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    wchar_t nameBuf[64]{};
    if (scanCode != 0 && GetKeyNameTextW(static_cast<LONG>(scanCode << 16), nameBuf, 64) > 0)
    {
        return nameBuf;
    }
    return L"VK_" + std::to_wstring(vk);
}

static std::wstring windowStateToString(WindowState state)
{
    switch (state)
    {
        case WindowState::eMinimized: return L"Minimized";
        case WindowState::eWindowed: return L"Windowed";
        case WindowState::eWindowedFullscreen: return L"Maximized";
        case WindowState::eFullscreen: return L"Fullscreen";
        case WindowState::eHidden: return L"Hidden";
        default: return L"None";
    }
}

static void check(bool condition, const char* expr)
{
    if (!condition)
    {
        std::cerr << "[FAIL] Check failed: " << expr << "\n";
        std::exit(1);
    }
}

#define TEST_CHECK(cond) check((cond), #cond)

static void runAutomatedTests(mWindow& window, TestCallbackState& cbState)
{
    HWND hwnd = window.getWindowHandle().hWnd;

    // 1. Test de dimensionnement (setWindowStyle -> WM_SIZE / WM_MOVE -> callbacks)
    int prevResizeCount = cbState.resizeCount;
    window.setWindowStyle({ WindowState::eWindowed, { 1024, 720, 120, 90 } });
    TEST_CHECK(window.pullEvents());
    TEST_CHECK(cbState.resizeCount > prevResizeCount);
    TEST_CHECK(cbState.lastResizeW > 0 && cbState.lastResizeH > 0);
    int32_t largeW = cbState.lastResizeW;
    int32_t largeH = cbState.lastResizeH;

    prevResizeCount = cbState.resizeCount;
    window.setWindowStyle({ WindowState::eWindowed, { 800, 600, 150, 110 } });
    TEST_CHECK(window.pullEvents());
    TEST_CHECK(cbState.resizeCount > prevResizeCount);
    TEST_CHECK(cbState.lastResizeW < largeW && cbState.lastResizeH < largeH);
    TEST_CHECK(cbState.moveCount > 0);

    // 2. Test des triggers boutons : Minimize -> Maximize -> Restore -> Close
    int prevMin = cbState.minimizeCount;
    PostMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
    TEST_CHECK(window.pullEvents());
    TEST_CHECK(cbState.minimizeCount > prevMin);
    TEST_CHECK(window.getWindowState().state == WindowState::eMinimized);

    int prevMax = cbState.maximizeCount;
    PostMessageW(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    TEST_CHECK(window.pullEvents());
    TEST_CHECK(cbState.maximizeCount > prevMax);
    TEST_CHECK(cbState.lastMaxW > 0 && cbState.lastMaxH > 0);
    TEST_CHECK(window.getWindowState().state == WindowState::eWindowedFullscreen);

    int prevRestore = cbState.restoreCount;
    PostMessageW(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    TEST_CHECK(window.pullEvents());
    TEST_CHECK(cbState.restoreCount > prevRestore);
    TEST_CHECK(window.getWindowState().state == WindowState::eWindowed);

    int prevClose = cbState.closeCount;
    PostMessageW(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
    TEST_CHECK(window.pullEvents());
    TEST_CHECK(cbState.closeCount > prevClose);

    cbState.allowClose = true;
    std::cout << "[OK] Automated resize & button trigger tests passed.\n";
}

int main()
{
    mWindow window(800, 600, L"Test Window");

    TestCallbackState cbState{};
    cbState.hWnd = window.getWindowHandle().hWnd;

    window.AddEventWindowCallback<ResizeCallback>(onResize, &cbState);
    window.AddEventWindowCallback<MinimizeCallback>(onMinimize, &cbState);
    window.AddEventWindowCallback<MaximizeCallback>(onMaximize, &cbState);
    window.AddEventWindowCallback<RestoreCallback>(onRestore, &cbState);
    window.AddEventWindowCallback<MoveCallback>(onMove, &cbState);
    window.AddEventWindowCallback<CloseCallback>(onClose, &cbState);

    runAutomatedTests(window, cbState);

    std::cout << "Interactive controls: [F1] Minimize | [F2] Maximize | [F3] Restore | [F4] Toggle Size | [ESC] Close\n";

    std::bitset<256> prevKeys{};
    int32_t lastNonZeroDX{ 0 }, lastNonZeroDY{ 0 };
    bool largeSize{ false };
    std::wstring lastTitle;

    std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
    while (window.pullEvents())
    {
        const auto& input = window.getUserInput();
        const auto keys = input.keyboardInput.getKeyState();

        auto justPressed = [&](uint8_t vk) {
            return keys[vk] && !prevKeys[vk];
        };

        if (justPressed(VK_F1))
        {
            window.setWindowStyle({ WindowState::eMinimized, {} });
        }
        if (justPressed(VK_F2))
        {
           
        }
        if (justPressed(VK_F3))
        {
            window.setWindowStyle({ WindowState::eWindowed, {} });
        }
        if (justPressed(VK_F4))
        {
            largeSize = !largeSize;
            WindowPos newPos = largeSize
                ? WindowPos{ 1024, 720, 100, 100 }
                : WindowPos{ 800, 600, 150, 150 };
            window.setWindowStyle({ WindowState::eWindowed, newPos });
        }
        if (justPressed(VK_ESCAPE))
        {
            PostMessageW(cbState.hWnd, WM_CLOSE, 0, 0);
        }

        int32_t dX{ 0 }, dY{ 0 };
        input.mouseInput.getDelta(dX, dY);
        if (dX != 0 || dY != 0)
        {
            lastNonZeroDX = dX;
            lastNonZeroDY = dY;
        }

        std::wostringstream keysStream;
        bool firstKey = true;
        for (uint16_t vk = 0; vk < 256; ++vk)
        {
            // Skip generic VK_SHIFT/CONTROL/MENU when left/right variants are tracked
            if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU)
                continue;
            if (keys[vk])
            {
                if (!firstKey) keysStream << L", ";
                keysStream << keyToName(static_cast<uint8_t>(vk));
                firstKey = false;
            }
        }
        if (firstKey)
            keysStream << L"none";

        const auto& winState = window.getWindowState();
        std::wostringstream titleStream;
        titleStream << L"Keys: [" << keysStream.str() << L"]"
                    << L" | Mouse dX:" << lastNonZeroDX << L" dY:" << lastNonZeroDY
                    << L" | " << windowStateToString(winState.state)
                    << L" (" << winState.pos.width << L"x" << winState.pos.height << L")";

        std::wstring newTitle = titleStream.str();
        if (newTitle != lastTitle)
        {
            window.setWindowTitle(newTitle);
            lastTitle = std::move(newTitle);
        }

        prevKeys = keys;
    }
    return 0;
}