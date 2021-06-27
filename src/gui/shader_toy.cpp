//
// Created by Mike Smith on 2021/6/27.
//

#include <core/basic_types.h>
#include <gui/shader_toy.h>

namespace luisa::gui {

using namespace compute;

template<typename F>
static void with_panel(const char *name, F &&f) {
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    f();
    ImGui::End();
}

void ShaderToy::run(uint2 size) noexcept {

    Window window{_title, size};
    GLTexture texture{PixelFormat::RGBA8UNorm, size};
    auto device_image = _device.create_image<float>(PixelStorage::BYTE4, size);

    texture.with_pixels_uploading([&](void *pixels) noexcept {
        _stream << _clear(device_image).launch(size)
                << _shader(device_image, window.time(), window.cursor()).launch(size)
                << device_image.copy_to(pixels)
                << _event.signal();
    });

    Framerate framerate{0.8};
    window.run([&] {
        _event.synchronize();
        auto render_size = device_image.size();
        ImVec2 background_size{static_cast<float>(render_size.x), static_cast<float>(render_size.y)};
        ImGui::GetBackgroundDrawList()->AddImage(reinterpret_cast<ImTextureID *>(texture.handle()), {}, background_size);

        auto window_size = window.size();
        if (render_size.x != window_size.x || render_size.y != window_size.y) {
            device_image = _device.create_image<float>(PixelStorage::BYTE4, window_size);
            _stream << _clear(device_image).launch(window_size);
            texture.resize(window_size);
        }

        auto time = window.time();
        auto cursor = window.cursor();
        texture.with_pixels_uploading([&](void *pixels) noexcept {
            _stream << _shader(device_image, time, cursor).launch(window_size)
                    << device_image.copy_to(pixels)
                    << _event.signal();
        });

        framerate.tick();
        auto fps = framerate.fps();
        auto spp = framerate.count();
        with_panel("Console", [&] {
            ImGui::Text("Frame: %llu", spp);
            ImGui::Text("Time:  %.2lfs", time);
            ImGui::Text("FPS:   %.1lf", fps);
            ImGui::Text("Size:  %ux%u", window_size.x, window_size.y);
            ImGui::Text("Mouse: (%.1f, %.1f)", cursor.x, cursor.y);
        });

        if (window.key_down(KEY_ESCAPE)) { window.notify_close(); }
    });
}

ShaderToy::ShaderToy(Device &device, std::string_view title, const Callable<float4(float2, float2, float, float2)> &shader) noexcept
    : _device{device},
      _stream{device.create_stream()},
      _event{device.create_event()},
      _title{title},
      _shader{[&shader](ImageFloat image, Float time, Float2 cursor) noexcept {
          using namespace compute;
          Var xy = dispatch_id().xy();
          Var resolution = launch_size().xy();
          Var col = shader(xy.cast<float2>(), resolution.cast<float2>(), time, cursor);
          Var color = col.xyz();
          Var alpha = col.w;
          Var old = image.read(xy).xyz();
          Var accum = lerp(color, old, alpha);
          image.write(xy, make_float4(accum, 1.0f));
      }},
      _clear{[](ImageFloat image) noexcept {
          using namespace compute;
          Var coord = dispatch_id().xy();
          Var rg = make_float2(coord) / make_float2(launch_size().xy());
          image.write(coord, make_float4(make_float2(0.3f, 0.4f), 0.5f, 1.0f));
      }} {
    device.compile(_shader, _clear);
}

}// namespace luisa::gui
