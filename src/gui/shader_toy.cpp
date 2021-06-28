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

void ShaderToy::_run(uint2 size) noexcept {

    Window window{_title, size};
    GLTexture texture{PixelFormat::RGBA8UNorm, size};
    auto device_image = _device.create_image<float>(PixelStorage::BYTE4, size);

    texture.with_pixels_uploading([&](void *pixels) noexcept {
        _stream << _clear(device_image).launch(size)
                << device_image.copy_to(pixels)
                << _event.signal();
    });
    
    auto cursor = float4(0.0f);
    auto dragging = false;
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
        
        if (window.mouse_down(MOUSE_LEFT)) {
            auto curr = window.cursor();
            curr = float2(curr.x, static_cast<float>(window_size.y) - curr.y);
            if (dragging) {
                cursor = float4(curr, cursor.zw());
            } else {
                cursor = float4(curr, curr * float2(1.0f, -1.0f));
                dragging = true;
            }
        } else if (window.mouse_up(MOUSE_LEFT)) {
            cursor = float4(cursor.xy(), -abs(cursor.zw()));
            dragging = false;
        }

        auto time = window.time();
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
            ImGui::Text("Mouse: (%.1f, %.1f, %.1f, %.1f)", cursor.x, cursor.y, cursor.z, cursor.w);
        });

        if (window.key_down(KEY_ESCAPE)) { window.notify_close(); }
    });
}

ShaderToy::ShaderToy(Device &device, std::string_view title, const Shader &shader) noexcept
    : _device{device},
      _stream{device.create_stream()},
      _event{device.create_event()},
      _title{title},
      _shader{[&shader](ImageFloat image, Float time, Float4 cursor) noexcept {
          using namespace compute;
          Var xy = dispatch_id().xy();
          Var resolution = launch_size().xy();
          Var col = shader(make_uint2(xy.x, resolution.y - 1u - xy.y).cast<float2>() + 0.5f, resolution.cast<float2>(), time, cursor);
          image.write(xy, make_float4(col, 1.0f));
      }},
      _clear{[](ImageFloat image) noexcept {
          using namespace compute;
          Var coord = dispatch_id().xy();
          Var rg = make_float2(coord) / make_float2(launch_size().xy());
          image.write(coord, float4(0.0f));
      }} {
    device.compile(_shader, _clear);
}

void ShaderToy::run(const std::filesystem::path &program, const ShaderToy::Shader &shader, uint2 size) noexcept {
    Context context{program};

#if defined(LUISA_BACKEND_METAL_ENABLED)
    auto device = context.create_device("metal");
#elif defined(LUISA_BACKEND_DX_ENABLED)
    auto device = context.create_device("dx");
#else
    auto &&device = *static_cast<Device *>(nullptr);
#endif
    
    auto title = program.filename().replace_extension("").string();
    for (auto &c : title) { c = c == '_' ? ' ' : c; }
    auto is_first = true;
    for (auto &c : title) {
        if (is_first) { c = static_cast<char>(std::toupper(c)); }
        is_first = c == ' ';
    }
    ShaderToy{device, title, shader}._run(size);
}

}// namespace luisa::gui
