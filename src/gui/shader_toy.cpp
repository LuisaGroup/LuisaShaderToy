//
// Created by Mike Smith on 2021/6/27.
//

#include <core/basic_types.h>
#include <gui/shader_toy.h>

#if LUISA_SHADERTOY_HAS_OPENCV
#include <opencv2/opencv.hpp>
#endif

namespace luisa::gui {

using namespace compute;

template<typename F>
static void with_panel(const char *name, F &&f) {
    ImGui::Begin(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    f();
    ImGui::End();
}

void ShaderToy::run(const MainShader &main_shader) noexcept {

    auto shader = _device->compile(Kernel2D{[&main_shader](ImageFloat image, Float time, Float4 cursor) noexcept {
        using namespace compute;
        Var xy = dispatch_id().xy();
        Var resolution = dispatch_size().xy();
        Var col = main_shader(make_float2(make_uint2(xy.x, resolution.y - 1u - xy.y)) + 0.5f,
                              make_float2(resolution), time, cursor);
        image.write(xy, make_float4(col, 1.0f));
    }});

    Window window{_title, _size};
    GLTexture texture{PixelFormat::RGBA8UNorm, _size};
    auto device_image = _device->create_image<float>(PixelStorage::BYTE4, _size);
    _stream << _event.signal();

    auto prev_key_up = false;
    auto show_console = true;
    auto cursor = float4(0.0f);
    auto dragging = false;
    Framerate framerate{0.8};
    window.run([&] {
        auto render_size = device_image.size();
        auto window_size = window.size();
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

        auto time = _step == 0. ? window.time() : static_cast<double>(framerate.count()) * _step;
        if (texture.present([&](void *pixels) noexcept {
                _event.synchronize();
                _stream << shader(device_image, static_cast<float>(time), cursor).dispatch(window_size)
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

ShaderToy::ShaderToy(int argc, const char *const *argv) noexcept
    : _context{argv[0]} {
    Context context{argv[0]};
    luisa::string backend{"unknown"};
    auto device_id = 0u;
    for (auto i = 1u; i < argc; i++) {
        using namespace std::string_view_literals;
        auto next_arg = [&] {
            if (i + 1u >= argc) {
                LUISA_ERROR_WITH_LOCATION(
                    "Missing argument for option: ",
                    argv[i]);
            }
            return argv[++i];
        };
        if (argv[i] == "-b"sv || argv[i] == "--backend"sv) {
            backend = next_arg();
        } else if (argv[i] == "-s"sv || argv[i] == "--size"sv) {
            auto s = next_arg();
            auto n = std::sscanf(s, "%ux%u", &_size.x, &_size.y);
            LUISA_ASSERT(n != 0, "Invalid size: {}", s);
            if (n == 1) { _size.y = _size.x; }
            _size = luisa::clamp(_size, 1u, 4096u);
        } else if (argv[i] == "-d"sv || argv[i] == "--device"sv) {
            device_id = std::atoi(next_arg());
        } else if (argv[i] == "--step"sv) {
            _step = std::clamp(std::atof(next_arg()), 0., 1000.);
        } else if (argv[i] == "-o"sv || argv[i] == "--output"sv) {
            _dump_file = next_arg();
        } else if (argv[i] == "-n"sv || argv[i] == "--frames"sv) {
            _dump_frames = std::atoi(next_arg());
        } else if (argv[i] == "--fps"sv) {
            _dump_fps = std::clamp(std::atof(next_arg()), 1., 200.);
        } else {
            LUISA_ERROR_WITH_LOCATION("Unknown option: ", argv[i]);
        }
    }
    _title = std::filesystem::canonical(argv[0]).filename().replace_extension("").string();
    for (auto &c : _title) { c = c == '_' ? ' ' : c; }
    auto is_first = true;
    for (auto &c : _title) {
        if (is_first) { c = static_cast<char>(std::toupper(c)); }
        is_first = c == ' ';
    }
    _device = luisa::make_unique<Device>(context.create_device(
        backend, luisa::format("{{\"index\": {}}}", device_id)));
    _stream = _device->create_stream();
    _event = _device->create_event();
}

}// namespace luisa::gui
