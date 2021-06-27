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
    using Shader = Callable<float4(
        float2 /* xy */,
        float2 /* resolution */,
        float /* time */,
        float2 /* cursor */)>;

private:
    Device &_device;
    Stream _stream;
    Event _event;
    std::string_view _title;
    Kernel2D<void(Image<float>, float, float2)> _shader;
    Kernel2D<void(Image<float>)> _clear;

private:
    ShaderToy(Device &device, std::string_view title,
              const Shader &shader) noexcept;
    void _run(uint2 size) noexcept;

public:
    static void run(const std::filesystem::path &program, const ShaderToy::Shader &shader, uint2 size = {1280u, 720u}) noexcept;
};

}// namespace luisa::gui
