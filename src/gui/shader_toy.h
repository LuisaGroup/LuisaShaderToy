//
// Created by Mike Smith on 2021/6/27.
//

#pragma once

#include <dsl/syntax.h>
#include <gui/framerate.h>
#include <gui/gl_texture.h>
#include <gui/window.h>
#include <runtime/context.h>
#include <runtime/device.h>
#include <runtime/event.h>
#include <runtime/stream.h>

namespace luisa::gui {

using compute::Callable;
using compute::Context;
using compute::Device;
using compute::Event;
using compute::Float;
using compute::Float2;
using compute::Float4;
using compute::Image;
using compute::ImageFloat;
using compute::ImageVar;
using compute::Kernel2D;
using compute::Stream;

class ShaderToy {

public:
    using Shader = Kernel2D<void(
        Image<float>,// image
        float,       // time
        float2       // mouse
        )>;

private:
    Device &_device;
    Stream _stream;
    Event _event;
    std::string_view _title;
    Kernel2D<void(Image<float>, float, float2)> _shader;
    Kernel2D<void(Image<float>)> _clear;

public:
    ShaderToy(Device &device, std::string_view title,
              const Callable<float4(float2 /* xy */,
                                    float2 /* resolution */,
                                    float /* time */,
                                    float2 /* cursor */)> &shader) noexcept;
    void run(uint2 size) noexcept;
    void run(uint w, uint h) noexcept { run({w, h}); }
};

template<typename Shader>
inline void run_toy(const std::filesystem::path &program, uint width, uint height, Shader &&shader) noexcept {
    Context context{program};

#if defined(LUISA_BACKEND_METAL_ENABLED)
    auto device = context.create_device("metal");
#elif defined(LUISA_BACKEND_DX_ENABLED)
    auto device = context.create_device("dx");
#else
#error No backend available
#endif

    auto title = program.filename().replace_extension("").string();
    for (auto &c : title) { c = c == '_' ? ' ' : c; }
    auto is_first = true;
    for (auto &c : title) {
        if (is_first) { c = static_cast<char>(std::toupper(c)); }
        is_first = c == ' ';
    }
    ShaderToy{device, title, std::forward<Shader>(shader)}.run(width, height);
}

}// namespace luisa::gui
