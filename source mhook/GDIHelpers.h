#pragma once
#include <Windows.h>
template <typename T, typename Deleter>
class RAIIWrapper {
    T handle;
    Deleter deleter;
public:
    RAIIWrapper(T h = {}) : handle(h) {}
    ~RAIIWrapper() { if (handle) deleter(handle); }
    RAIIWrapper(const RAIIWrapper&) = delete;
    RAIIWrapper& operator=(const RAIIWrapper&) = delete;
    RAIIWrapper(RAIIWrapper&& other) noexcept : handle(other.handle) { other.handle = {}; }
    RAIIWrapper& operator=(RAIIWrapper&& other) noexcept {
        if (this != &other) { if (handle) deleter(handle); handle = other.handle; other.handle = {}; }
        return *this;
    }
    T get() const { return handle; }
    explicit operator bool() const { return handle != nullptr; }
    T* operator&() { return &handle; }
};
struct BrushDeleter { void operator()(HBRUSH h) const { DeleteObject(h); } };
using GdiBrush = RAIIWrapper<HBRUSH, BrushDeleter>;
struct PenDeleter { void operator()(HPEN h) const { DeleteObject(h); } };
using GdiPen = RAIIWrapper<HPEN, PenDeleter>;
struct FontDeleter { void operator()(HFONT h) const { DeleteObject(h); } };
using GdiFont = RAIIWrapper<HFONT, FontDeleter>;
struct BitmapDeleter { void operator()(HBITMAP h) const { DeleteObject(h); } };
using GdiBitmap = RAIIWrapper<HBITMAP, BitmapDeleter>;
struct DcDeleter { void operator()(HDC h) const { DeleteDC(h); } };
using GdiDC = RAIIWrapper<HDC, DcDeleter>;