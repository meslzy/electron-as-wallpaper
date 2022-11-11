#include <napi.h>
#include <windows.h>

using namespace Napi;

DWORD MakeWindowTransparent(HWND hWnd, unsigned char factor)
{
  auto windowLong = SetWindowLong(hWnd,GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

  if ((windowLong == 0) && (GetLastError() != 0)) {
    return FALSE;
  }

  if (!SetLayeredWindowAttributes(hWnd, 0, factor, LWA_ALPHA))
  {
    return FALSE;
  }

  return TRUE;
}

Napi::Value Attach(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    Napi::Buffer<void *> wndHandle = info[0].As<Napi::Buffer<void*>>();
    HWND win = static_cast<HWND>(*reinterpret_cast<void **>(wndHandle.Data()));

    Napi::Object options = info[1].As<Napi::Object>();
    Napi::Boolean transparent = options.Get("transparent").As<Napi::Boolean>();

    HWND progmanHandle = FindWindow("Progman", nullptr);

    SendMessage(progmanHandle, 0x052C, 0x0000000D, 0);
    SendMessage(progmanHandle, 0x052C, 0x0000000D, 1);

    static HWND workerW = nullptr;
    EnumWindows([](HWND topHandle, LPARAM topParamHandle) {
        HWND shellDllDefView = FindWindowEx(topHandle, nullptr, "SHELLDLL_DefView", nullptr);

        if (shellDllDefView != nullptr) {
            workerW = FindWindowEx(nullptr, topHandle, "WorkerW", nullptr);
        }

        return TRUE;
    }, NULL);

    if (workerW == NULL)
    {
        Napi::TypeError::New(env, "couldn't locate WorkerW").ThrowAsJavaScriptException();
        return env.Null();
    }

    SetParent(win, workerW);

    if (transparent) {
        MakeWindowTransparent(win, 200);
    }

    return Napi::Boolean::New(env, true);
}

Napi::Value Detach(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    Napi::Buffer<void *> wndHandle = info[0].As<Napi::Buffer<void*>>();
    HWND win = static_cast<HWND>(*reinterpret_cast<void **>(wndHandle.Data()));

    SetParent(win, 0);

    return Napi::Boolean::New(env, true);
}

Napi::Value Refresh(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    SystemParametersInfo(20, 0, nullptr, 0x1);

    return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "attach"), Napi::Function::New(env, Attach));
    exports.Set(Napi::String::New(env, "detach"), Napi::Function::New(env, Detach));
    exports.Set(Napi::String::New(env, "refresh"), Napi::Function::New(env, Refresh));
    return exports;
}

NODE_API_MODULE(addon, Init);
