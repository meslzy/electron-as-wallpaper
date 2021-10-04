#include <napi.h>
#include <windows.h>

using namespace Napi;

Napi::Value Attach(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
        Napi::TypeError::New(env, "err msg 1").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info[0].IsBuffer())
    {
        Napi::Buffer<void *> wndHandle = info[0].As<Napi::Buffer<void*>>();
        HWND win = static_cast<HWND>(*reinterpret_cast<void **>(wndHandle.Data()));

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

        return Napi::Boolean::New(env, true);
    }

    Napi::TypeError::New(env, "err msg 2").ThrowAsJavaScriptException();
    return env.Null();
}

Napi::Value Detach(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() != 1)
    {
        Napi::TypeError::New(env, "err msg 1").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (info[0].IsBuffer())
    {
        Napi::Buffer<void *> wndHandle = info[0].As<Napi::Buffer<void*>>();
        HWND win = static_cast<HWND>(*reinterpret_cast<void **>(wndHandle.Data()));

        SetParent(win, 0);

        return Napi::Boolean::New(env, true);
    }

    Napi::TypeError::New(env, "err msg 2").ThrowAsJavaScriptException();
    return env.Null();
}

Napi::Value Refresh(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    SystemParametersInfo(20, 0, nullptr, 0x1);

    return Napi::Boolean::New(env, true);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "Attach"), Napi::Function::New(env, Attach));
    exports.Set(Napi::String::New(env, "Detach"), Napi::Function::New(env, Detach));
    exports.Set(Napi::String::New(env, "Refresh"), Napi::Function::New(env, Refresh));
    return exports;
}

NODE_API_MODULE(addon, Init);
