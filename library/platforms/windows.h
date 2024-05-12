#ifndef WINDOWS_H
#define WINDOWS_H

#include <napi.h>

void attach(const Napi::CallbackInfo &info);

void detach(const Napi::CallbackInfo &info);

void refresh(const Napi::CallbackInfo &info);

#endif //WINDOWS_H
