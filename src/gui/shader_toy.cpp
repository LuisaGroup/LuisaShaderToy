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
    _stream << _event.signal();

    auto prev_key_up = false;
    auto show_console = true;
    auto cursor = float4(0.0f);
    auto dragging = false;
    Framerate framerate{0.8};
    window.run([&] {
        auto render_size = device_image.size();
        auto window_size = window.size();
        if (!all(render_size == window_size.x)) {
            device_image = _device.create_image<float>(PixelStorage::BYTE4, window_size);
            texture.resize(window_size);
        }

        if (window.mouse_down(MOUSE_LEFT)) {
            auto curr = window.cursor();
            curr = float2(curr.x, static_cast<float>(window_size.y) - curr.y);
            if (dragging) {
                cursor = make_float4(curr, cursor.zw());
            } else {
                cursor = make_float4(curr, curr * float2(1.0f, -1.0f));
                dragging = true;
            }
        } else if (window.mouse_up(MOUSE_LEFT)) {
            cursor = make_float4(cursor.xy(), -abs(cursor.zw()));
            dragging = false;
        }

        auto time = window.time();
        if (texture.present([&](void *pixels) noexcept {
                _event.synchronize();
                _stream << _shader(device_image, time, cursor).dispatch(window_size)
                        << device_image.copy_to(pixels)
                        << _event.signal();
            })) {
            ImVec2 background_size{static_cast<float>(render_size.x), static_cast<float>(render_size.y)};
            ImGui::GetBackgroundDrawList()->AddImage(reinterpret_cast<ImTextureID *>(texture.handle()), {}, background_size);
        }

        framerate.tick();
        auto fps = framerate.fps();
        auto spp = framerate.count();
        if (show_console) {
            with_panel("Console", [&] {
                ImGui::Text("Frame: %llu", static_cast<uint64_t>(spp));
                ImGui::Text("Time:  %.2lfs", time);
                ImGui::Text("FPS:   %.1lf", fps);
                ImGui::Text("Size:  %ux%u", window_size.x, window_size.y);
            });
        }
        if (window.key_down(KEY_ESCAPE)) {
            window.notify_close();
        }
        if (prev_key_up && (window.key_down(KEY_LEFT_CONTROL) || window.key_down(KEY_RIGHT_CONTROL))) {
            show_console = !show_console;
        }
        prev_key_up = window.key_up(KEY_LEFT_CONTROL) && window.key_up(KEY_RIGHT_CONTROL);
    });
}

ShaderToy::ShaderToy(Device &device, std::string_view title, const MainShader &shader) noexcept
    : _device{device},
      _stream{device.create_stream()},
      _event{device.create_event()},
      _title{title},
      _shader{device.compile(Kernel2D{[&shader](ImageFloat image, Float time, Float4 cursor) noexcept {
          using namespace compute;
          Var xy = dispatch_id().xy();
          Var prev_color = image.read(xy).xyz();
          Var resolution = dispatch_size().xy();
          Var col = shader(make_float2(make_uint2(xy.x, resolution.y - 1u - xy.y)) + 0.5f, make_float2(resolution), time, cursor, prev_color);
          image.write(xy, make_float4(col, 1.0f));
      }})} {}

void ShaderToy::run(const std::filesystem::path &program, const ShaderToy::MainShader &shader, uint2 size) noexcept {
    Context context{program};

    auto env = getenv("LUISA_COMPUTE_BACKEND");
    luisa::string backend{env ? env : ""};
    if (backend.empty()) {
#if defined(LUISA_BACKEND_CUDA_ENABLED)
        backend = "cuda";
#elif defined(LUISA_BACKEND_METAL_ENABLED)
        backend = "metal";
#elif defined(LUISA_BACKEND_DX_ENABLED)
        backend = "dx";
#else
        backend = "ispc";
#endif
    }

    auto device = context.create_device(backend);
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
