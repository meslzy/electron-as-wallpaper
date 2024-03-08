#include <node_api.h>
#include <napi.h>
#include <windows.h>
#include <hidusage.h>

struct Window {
    HWND handle;
    bool transparent;
    bool forwardMouseInput;
    bool forwardKeyboardInput;
};

std::vector<Window> windows;

auto makeWindowTransparent(HWND windowHandle, bool active) {
    if (active) {
        SetWindowLong(windowHandle, GWL_EXSTYLE, GetWindowLong(windowHandle, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(windowHandle, 0, 255, LWA_ALPHA);
    } else {
        SetWindowLong(windowHandle, GWL_EXSTYLE, GetWindowLong(windowHandle, GWL_EXSTYLE) & ~WS_EX_LAYERED);
    }
}

bool isDesktopActive() {
    HWND foreground = GetForegroundWindow();
    HWND desktop = GetDesktopWindow();
    HWND shell = GetShellWindow();

    HWND desktopWorkerW = FindWindowEx(nullptr, nullptr, "WorkerW", nullptr);

    while (desktopWorkerW != nullptr) {
        HWND shellDllDefView = FindWindowEx(desktopWorkerW, nullptr, "SHELLDLL_DefView", nullptr);

        if (shellDllDefView != nullptr) {
            break;
        }

        desktopWorkerW = FindWindowEx(nullptr, desktopWorkerW, "WorkerW", nullptr);
    }

    HWND wallpaperWorkerW = FindWindowEx(nullptr, desktopWorkerW, "WorkerW", nullptr);

    bool isForeground = (
            foreground == desktop ||
            foreground == shell ||
            foreground == desktopWorkerW ||
            foreground == wallpaperWorkerW
    );

    return isForeground;
}

void postMouseMessage(UINT uMsg, WPARAM wParam) {
    for (auto &window: windows) {
        if (window.forwardMouseInput) {
            PostMessageA(window.handle, uMsg, wParam, 0);
        }
    }
}

void postMouseMessage(UINT uMsg, WPARAM wParam, POINT point) {
    for (auto &window: windows) {
        if (window.forwardMouseInput) {
            ScreenToClient(window.handle, &point);

            auto lParam = static_cast<std::uint32_t>(point.y);
            lParam <<= 16;
            lParam |= static_cast<std::uint32_t>(point.x);

            PostMessageA(window.handle, uMsg, wParam, lParam);
        }
    }
}

void postKeyboardMessage(UINT uMsg, WPARAM wParam, UINT makeCode, bool isPressed) {
    std::uint32_t lParam = 1u;

    lParam |= static_cast<std::uint32_t>(makeCode) << 16;
    lParam |= 1u << 24;
    lParam |= 0u << 29;

    if (!isPressed) {
        lParam |= 1u << 30;
        lParam |= 1u << 31;
    }

    for (auto &window: windows) {
        if (window.forwardKeyboardInput) {
            PostMessageA(window.handle, uMsg, wParam, lParam);
        }
    }
}

LRESULT CALLBACK handleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (isDesktopActive() && uMsg == WM_INPUT) {
        UINT dwSize = sizeof(RAWINPUT);
        static BYTE lpb[sizeof(RAWINPUT)];

        GetRawInputData((HRAWINPUT) lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

        auto *raw = (RAWINPUT *) lpb;

        switch (raw->header.dwType) {
            case RIM_TYPEMOUSE: {
                POINT point;

                GetCursorPos(&point);

                RAWMOUSE rawMouse = raw->data.mouse;

                if ((rawMouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL || (rawMouse.usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL) {
                    bool isHorizontalScroll = (rawMouse.usButtonFlags & RI_MOUSE_HWHEEL) == RI_MOUSE_HWHEEL;

                    auto wheelDelta = (float) (short) rawMouse.usButtonData;

                    if (isHorizontalScroll) {
                        postMouseMessage(WM_HSCROLL, wheelDelta > 0 ? SB_LINELEFT : SB_LINERIGHT);
                    } else {
                        postMouseMessage(WM_VSCROLL, wheelDelta > 0 ? SB_LINEUP : SB_LINEDOWN);
                    }

                    break;
                }

                switch (rawMouse.ulButtons) {
                    case RI_MOUSE_LEFT_BUTTON_DOWN: {
                        postMouseMessage(WM_LBUTTONDOWN, MK_LBUTTON, point);
                        break;
                    }
                    case RI_MOUSE_LEFT_BUTTON_UP: {
                        postMouseMessage(WM_LBUTTONUP, MK_LBUTTON, point);
                        break;
                    }
                    case RI_MOUSE_RIGHT_BUTTON_DOWN: {
                        postMouseMessage(WM_RBUTTONDOWN, MK_RBUTTON, point);
                        break;
                    }
                    case RI_MOUSE_RIGHT_BUTTON_UP: {
                        postMouseMessage(WM_RBUTTONUP, MK_RBUTTON, point);
                        break;
                    }
                    case RI_MOUSE_MIDDLE_BUTTON_DOWN: {
                        postMouseMessage(WM_MBUTTONDOWN, MK_MBUTTON, point);
                        break;
                    }
                    case RI_MOUSE_MIDDLE_BUTTON_UP: {
                        postMouseMessage(WM_MBUTTONUP, MK_MBUTTON, point);
                        break;
                    }
                    case RI_MOUSE_BUTTON_4_DOWN:
                        postMouseMessage(WM_XBUTTONDOWN, MAKEWPARAM(0, XBUTTON1), point);
                        break;
                    case RI_MOUSE_BUTTON_4_UP:
                        postMouseMessage(WM_XBUTTONUP, MAKEWPARAM(0, XBUTTON1), point);
                        break;
                    case RI_MOUSE_BUTTON_5_DOWN:
                        postMouseMessage(WM_XBUTTONDOWN, MAKEWPARAM(0, XBUTTON2), point);
                        break;
                    case RI_MOUSE_BUTTON_5_UP:
                        postMouseMessage(WM_XBUTTONUP, MAKEWPARAM(0, XBUTTON2), point);
                        break;
                    default: {
                        postMouseMessage(WM_MOUSEMOVE, 0x0020, point);
                        break;
                    }
                }

                break;
            }
            case RIM_TYPEKEYBOARD: {
                RAWKEYBOARD rawKeyboard = raw->data.keyboard;

                auto message = rawKeyboard.Message;
                auto vKey = rawKeyboard.VKey;
                auto makeCode = rawKeyboard.MakeCode;
                auto flags = rawKeyboard.Flags;

                postKeyboardMessage(message, vKey, makeCode, flags & RI_KEY_BREAK);

                break;
            }
        }
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND rawInputWindowHandle = nullptr;
LPCSTR rawInputWindowClassName = "RawInputWindow";

void startForwardingRawInput(Napi::Env env) {
    if (rawInputWindowHandle != nullptr) {
        return;
    }

    HINSTANCE hInstance = GetModuleHandleA(nullptr);

    WNDCLASS wc = {};
    wc.lpfnWndProc = handleWindowMessage;
    wc.hInstance = hInstance;
    wc.lpszClassName = rawInputWindowClassName;

    if (!RegisterClass(&wc)) {
        Napi::TypeError::New(env, "Could not register raw input window class").ThrowAsJavaScriptException();
        return;
    }

    rawInputWindowHandle = CreateWindowEx(0, wc.lpszClassName, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInstance, nullptr);

    if (!rawInputWindowHandle) {
        Napi::TypeError::New(env, "Could not create raw input window").ThrowAsJavaScriptException();
    }

    RAWINPUTDEVICE rid[2];

    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = rawInputWindowHandle;

    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = rawInputWindowHandle;

    if (RegisterRawInputDevices(rid, 2, sizeof(rid[0])) == FALSE) {
        Napi::TypeError::New(env, "Could not register raw input devices").ThrowAsJavaScriptException();
    }
}

void stopForwardingRawInput() {
    if (rawInputWindowHandle == nullptr) {
        return;
    }

    HINSTANCE hInstance = GetModuleHandleA(nullptr);

    DestroyWindow(rawInputWindowHandle);

    UnregisterClass(rawInputWindowClassName, hInstance);

    rawInputWindowHandle = nullptr;
}

void attach(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto buffer = info[0].As<Napi::Buffer<void *> >();
    HWND windowHandle = static_cast<HWND>(*buffer.Data());

    auto options = info[1].As<Napi::Object>();

    auto transparent = options.Get("transparent").As<Napi::Boolean>();
    auto forwardMouseInput = options.Get("forwardMouseInput").As<Napi::Boolean>();
    auto forwardKeyboardInput = options.Get("forwardKeyboardInput").As<Napi::Boolean>();

    auto shellHandle = GetShellWindow();

    SendMessage(shellHandle, 0x052C, 0x0000000D, 0);
    SendMessage(shellHandle, 0x052C, 0x0000000D, 1);

    static HWND workerW = nullptr;

    EnumWindows([](HWND topHandle, LPARAM topParamHandle) {
        HWND shellDllDefView = FindWindowEx(topHandle, nullptr, "SHELLDLL_DefView", nullptr);

        if (shellDllDefView != nullptr) {
            workerW = FindWindowEx(nullptr, topHandle, "WorkerW", nullptr);
        }

        return TRUE;
    }, NULL);

    if (workerW == nullptr) {
        Napi::TypeError::New(env, "couldn't locate WorkerW").ThrowAsJavaScriptException();
        return;
    }

    Window window = {
            windowHandle,
            transparent,
            forwardMouseInput,
            forwardKeyboardInput
    };

    SetParent(windowHandle, workerW);

    if (transparent) {
        makeWindowTransparent(windowHandle, true);
    }

    windows.push_back(window);

    if (!windows.empty()) {
        startForwardingRawInput(env);
    }
}

void detach(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);

    auto buffer = info[0].As<Napi::Buffer<void *> >();
    HWND windowHandle = static_cast<HWND>(*reinterpret_cast<void **>(buffer.Data()));

    SetParent(windowHandle, nullptr);

    auto window = std::find_if(windows.begin(), windows.end(), [windowHandle](const Window &window) {
        return window.handle == windowHandle;
    });

    if (window != windows.end()) {
        if (window->transparent) {
            makeWindowTransparent(window->handle, false);
        }

        windows.erase(window);
    }

    if (windows.empty()) {
        stopForwardingRawInput();
    }
}

void refresh(const Napi::CallbackInfo &info) {
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_SENDCHANGE);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "attach"), Napi::Function::New(env, attach));
    exports.Set(Napi::String::New(env, "detach"), Napi::Function::New(env, detach));
    exports.Set(Napi::String::New(env, "refresh"), Napi::Function::New(env, refresh));
    return exports;
}

NODE_API_MODULE(addon, Init);
