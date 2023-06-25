#include <node_api.h>
#include <napi.h>

#include <windows.h>
#include <winuser.h>
#include <hidusage.h>

using namespace Napi;

struct Window {
    HWND handle;
    bool transparent;
    bool forwardMouseInput;
    bool forwardKeyboardInput;
};

std::vector<Window> windows;

void makeWindowTransparent(HWND windowHandle, bool toggle) {
  auto windowLong = GetWindowLong(windowHandle, GWL_EXSTYLE);

  if (toggle) {
    windowLong |= WS_EX_LAYERED;
    SetWindowLong(windowHandle, GWL_EXSTYLE, windowLong);
    SetLayeredWindowAttributes(windowHandle, 0, 255, LWA_ALPHA);
  } else {
    windowLong &= ~WS_EX_LAYERED;
    SetWindowLong(windowHandle, GWL_EXSTYLE, windowLong);
  }
}

//

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

  if (foreground == desktop) {
    return true;
  } else if (foreground == shell) {
    return true;
  } else if (foreground == desktopWorkerW) {
    return true;
  } else if (foreground == wallpaperWorkerW) {
    return true;
  } else {
    return false;
  }
}

void postMouseMessage(UINT uMsg, WPARAM wParam, POINT point) {
  if (!isDesktopActive()) {
    return;
  }

  auto lParam = static_cast<std::uint32_t>(point.y);
  lParam <<= 16;
  lParam |= static_cast<std::uint32_t>(point.x);

  for (auto &window: windows) {
    if (window.forwardMouseInput) {
      PostMessage(window.handle, uMsg, wParam, lParam);
    }
  }
}

void postKeyboardMessage(UINT uMsg, WPARAM wParam, UINT makeCode, bool isPressed) {
  if (!isDesktopActive()) {
    return;
  }

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
      PostMessage(window.handle, uMsg, wParam, lParam);
    }
  }
}

LRESULT CALLBACK handleWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_INPUT) {
    UINT dwSize = sizeof(RAWINPUT);
    static BYTE lpb[sizeof(RAWINPUT)];

    GetRawInputData((HRAWINPUT) lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

    auto *raw = (RAWINPUT *) lpb;

    switch (raw->header.dwType) {
      case RIM_TYPEMOUSE: {
        POINT point;

        GetCursorPos(&point);

        switch (raw->data.mouse.ulButtons) {
          case RI_MOUSE_LEFT_BUTTON_DOWN: {
            postMouseMessage(WM_LBUTTONDOWN, 0x0001, point);
            break;
          }
          case RI_MOUSE_LEFT_BUTTON_UP: {
            postMouseMessage(WM_LBUTTONUP, 0x0001, point);
            break;
          }
          case RI_MOUSE_MIDDLE_BUTTON_DOWN: {
            postMouseMessage(WM_MBUTTONDOWN, 0x0010, point);
            break;
          }
          case RI_MOUSE_MIDDLE_BUTTON_UP: {
            postMouseMessage(WM_MBUTTONUP, 0x0010, point);
            break;
          }
          case RI_MOUSE_RIGHT_BUTTON_DOWN: {
            postMouseMessage(WM_RBUTTONDOWN, 0x0002, point);
            break;
          }
          case RI_MOUSE_RIGHT_BUTTON_UP: {
            postMouseMessage(WM_RBUTTONUP, 0x0002, point);
            break;
          }
          case RI_MOUSE_BUTTON_4_DOWN: {
            postMouseMessage(WM_XBUTTONDOWN, 0x0020, point);
            break;
          }
          case RI_MOUSE_BUTTON_4_UP: {
            postMouseMessage(WM_XBUTTONUP, 0x0020, point);
            break;
          }
          case RI_MOUSE_BUTTON_5_DOWN: {
            postMouseMessage(WM_XBUTTONDOWN, 0x0040, point);
            break;
          }
          case RI_MOUSE_BUTTON_5_UP: {
            postMouseMessage(WM_XBUTTONUP, 0x0040, point);
            break;
          }
          default: {
            postMouseMessage(WM_MOUSEMOVE, 0x0020, point);
            break;
          }
        }

        break;
      }
      case RIM_TYPEKEYBOARD: {
        auto message = raw->data.keyboard.Message;
        auto vKey = raw->data.keyboard.VKey;
        auto makeCode = raw->data.keyboard.MakeCode;
        auto flags = raw->data.keyboard.Flags;

        postKeyboardMessage(message, vKey, makeCode, flags & RI_KEY_BREAK);

        break;
      }
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND rawInputWindowHandle = nullptr;

void startForwardingRawInput(Napi::Env env) {
  if (rawInputWindowHandle != nullptr) {
    return;
  }

  HINSTANCE hInstance = GetModuleHandle(nullptr);

  WNDCLASS wc = {};
  wc.lpfnWndProc = handleWindowMessage;
  wc.hInstance = hInstance;
  wc.lpszClassName = "RawInputWindow";

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

  DestroyWindow(rawInputWindowHandle);
  rawInputWindowHandle = nullptr;
}

//

void attach(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  auto buffer = info[0].As<Napi::Buffer<void *>>();
  HWND windowHandle = static_cast<HWND>(*reinterpret_cast<void **>(buffer.Data()));

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

  SetParent(windowHandle, workerW);

  if (transparent) {
    makeWindowTransparent(windowHandle, true);
  }

  windows.push_back({windowHandle, transparent, forwardMouseInput, forwardKeyboardInput});

  if (!windows.empty()) {
    startForwardingRawInput(env);
  }
}

void detach(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  auto buffer = info[0].As<Napi::Buffer<void *>>();
  HWND windowHandle = static_cast<HWND>(*reinterpret_cast<void **>(buffer.Data()));

  auto window = std::find_if(windows.begin(), windows.end(), [windowHandle](const Window &window) {
      return window.handle == windowHandle;
  });

  if (window != windows.end()) {
    if (window->transparent) {
      makeWindowTransparent(windowHandle, false);
    }

    windows.erase(window);
  }

  if (windows.empty()) {
    stopForwardingRawInput();
  }
}

void refresh(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, nullptr, SPIF_SENDCHANGE);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "attach"), Napi::Function::New(env, attach));
  exports.Set(Napi::String::New(env, "detach"), Napi::Function::New(env, detach));
  exports.Set(Napi::String::New(env, "refresh"), Napi::Function::New(env, refresh));
  return exports;
}

NODE_API_MODULE(addon, Init);