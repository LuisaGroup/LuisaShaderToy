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

using compute::Device;
using compute::Event;
using compute::Image;
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
    Shader _shader;
    Stream _stream;
    Event _event;
    std::string_view _title;
    Kernel2D<void(Image<float>)> _clear;

public:
    template<typename Def>
    ShaderToy(Device &device, std::string_view title, Def &&shader) noexcept
        : _device{device},
          _shader{std::forward<Def>(shader)},
          _stream{device.create_stream()},
          _event{device.create_event()},
          _title{title},
          _clear{[](ImageVar<float> image) noexcept {
              using namespace compute;
              Var coord = dispatch_id().xy();
              Var rg = make_float2(coord) / make_float2(launch_size().xy());
              image.write(coord, make_float4(make_float2(0.3f, 0.4f), 0.5f, 1.0f));
          }} {
        device.compile(_shader, _clear);
    }

    void run(uint2 size) noexcept;
    void run(uint w, uint h) noexcept { run({w, h}); }
};

}// namespace luisa::gui
