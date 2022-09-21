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
#include <runtime/shader.h>

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
using compute::Shader;

class ShaderToy {

public:
    using MainShader = Callable<float3(
        float2 /* xy */,
        float2 /* resolution */,
        float /* time */,
        float4 /* cursor */)>;

private:
    Context _context;
    luisa::unique_ptr<Device> _device;
    Stream _stream;
    std::string _title{};
    uint2 _size{1280u, 720u};
    double _step{0.};
    luisa::string _dump_file{};
    uint _dump_frames{1u};
    double _dump_fps{24.};

private:
    void _run_display(const compute::Shader2D<Image<float>, float, float4> &shader) noexcept;
    void _run_dump(const compute::Shader2D<Image<float>, float, float4> &shader) noexcept;

public:
    ShaderToy(int argc, const char *const *argv) noexcept;
    void run(const MainShader &shader) noexcept;
    [[nodiscard]] auto &device() noexcept { return *_device; }
    [[nodiscard]] auto &stream() noexcept { return _stream; }
    [[nodiscard]] auto size() const noexcept { return _size; }
};

}// namespace luisa::gui
