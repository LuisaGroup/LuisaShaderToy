//
// Created by Mike Smith on 2021/6/25.
//

#include <dsl/syntax.h>
#include <gui/framerate.h>
#include <gui/gl_texture.h>
#include <gui/window.h>
#include <runtime/context.h>
#include <runtime/device.h>
#include <runtime/event.h>
#include <runtime/stream.h>

using namespace luisa;
using namespace luisa::gui;
using namespace luisa::compute;

template<typename F>
void with_panel(const char *name, ImGuiWindowFlags flags, F &&f) {
    ImGui::Begin(name, nullptr, flags);
    f();
    ImGui::End();
}

int main(int argc, char *argv[]) {

    //    log_level_verbose();

    Context context{argv[0]};

#if defined(LUISA_BACKEND_METAL_ENABLED)
    auto device = context.create_device("metal");
#elif defined(LUISA_BACKEND_DX_ENABLED)
    auto device = context.create_device("dx");
#else
#error No backend available
#endif

    Callable palette = [](Float d) noexcept {
        return lerp(make_float3(0.2f, 0.7f, 0.9f), make_float3(1.0f, 0.0f, 1.0f), d);
    };

    Callable rotate = [](Float2 p, Float a) noexcept {
        Var c = cos(a);
        Var s = sin(a);
        return make_float2(dot(p, make_float2(c, s)), dot(p, make_float2(-s, c)));
    };

    Callable map = [&rotate](Float3 p, Float time) noexcept {
        for (auto i = 0u; i < 8u; i++) {
            Var t = time * 0.2f;
            p = make_float3(rotate(p.xz(), t), p.y).xzy();
            p = make_float3(rotate(p.xy(), t * 1.89f), p.z);
            p = make_float3(abs(p.x) - 0.5f, p.y, abs(p.z) - 0.5f);
        }
        return dot(sign(p), p) * 0.2f;
    };

    Callable rm = [&map, &palette](Float3 ro, Float3 rd, Float time) noexcept {
        Var t = 0.0f;
        Var col = make_float3(0.0f);
        Var d = 0.0f;
        for (auto i : range(64)) {
            Var p = ro + rd * t;
            d = map(p, time) * 0.5f;
            if_(d < 0.02f || d > 100.0f, [] { break_(); });
            col += palette(length(p) * 0.1f) / (400.0f * d);
            t += d;
        }
        return make_float4(col, 1.0f / (d * 100.0f));
    };

    Kernel2D clear = [](ImageVar<float> image) noexcept {
        Var coord = dispatch_id().xy();
        Var rg = make_float2(coord) / make_float2(launch_size().xy());
        image.write(coord, make_float4(make_float2(0.3f, 0.4f), 0.5f, 1.0f));
    };

    Kernel2D shader = [&rm, &rotate](ImageFloat image, Float time) noexcept {
        Var xy = dispatch_id().xy();
        Var resolution = launch_size().xy().cast<float2>();
        Var uv = (xy.cast<float2>() - resolution * 0.5f) / resolution.x;
        Var ro = make_float3(rotate(make_float2(0.0f, -50.0f), time), 0.0f).xzy();
        Var cf = normalize(-ro);
        Var cs = normalize(cross(cf, make_float3(0.0f, 1.0f, 0.0f)));
        Var cu = normalize(cross(cf, cs));
        Var uuv = ro + cf * 3.0f + uv.x * cs + uv.y * cu;
        Var rd = normalize(uuv - ro);
        Var col = rm(ro, rd, time);
        Var color = col.xyz();
        Var alpha = col.w;
        Var old = image.read(xy).xyz();
        Var accum = lerp(color, old, alpha);
        image.write(xy, make_float4(accum, 1.0f));
    };

    device.compile(clear, shader);

    static constexpr auto width = 1280u;
    static constexpr auto height = 720u;
    auto device_image = device.create_image<float>(PixelStorage::BYTE4, width, height);

    Window window{"Fractal Pyramid", width, height};
    GLTexture texture{PixelFormat::RGBA8UNorm, width, height};

    auto stream = device.create_stream();
    auto event = device.create_event();
    texture.with_pixels_uploading([&](void *pixels) noexcept {
        stream << clear(device_image).launch(width, height)
               << shader(device_image, 0.0f).launch(width, height)
               << device_image.copy_to(pixels)
               << event.signal();
    });

    Clock clock;
    Framerate framerate{0.8};
    window.run([&] {
        event.synchronize();
        auto render_size = device_image.size();
        ImVec2 background_size{static_cast<float>(render_size.x), static_cast<float>(render_size.y)};
        ImGui::GetBackgroundDrawList()->AddImage(reinterpret_cast<ImTextureID *>(texture.handle()), {}, background_size);

        auto window_size = window.size();
        if (!all(render_size == window_size)) {
            device_image = device.create_image<float>(PixelStorage::BYTE4, window_size);
            stream << clear(device_image).launch(window_size);
            texture.resize(window_size);
        }
        texture.with_pixels_uploading([&](void *pixels) noexcept {
            stream << shader(device_image, static_cast<float>(clock.toc()) * 1e-3f).launch(window_size)
                   << device_image.copy_to(pixels)
                   << event.signal();
        });

        framerate.tick();
        auto fps = framerate.fps();
        auto spp = framerate.count();
        with_panel("Console", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize, [&] {
            ImGui::Text("Frame: %llu", spp);
            ImGui::Text("Size: %ux%u", window_size.x, window_size.y);
            ImGui::Text("FPS: %lf", fps);
        });
    });
}
