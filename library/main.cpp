#include <node_api.h>
#include <napi.h>

#if defined(_WIN32) || defined(_WIN64)

#include "platforms/windows.cpp"

#elif defined(__linux__)

#include "platforms/linux.cpp"

#elif defined(__APPLE__)

#include "platforms/macos.cpp"

#endif

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "attach"), Napi::Function::New(env, attach));
    exports.Set(Napi::String::New(env, "detach"), Napi::Function::New(env, detach));
    exports.Set(Napi::String::New(env, "refresh"), Napi::Function::New(env, refresh));
    return exports;
}

NODE_API_MODULE(addon, Init);
