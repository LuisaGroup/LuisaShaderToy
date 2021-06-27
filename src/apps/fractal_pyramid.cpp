//
// Created by Mike Smith on 2021/6/25.
//

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

// Credit: https://www.shadertoy.com/view/tsXBzS
int main(int argc, char *argv[]) {

    Callable palette = [](Float d) noexcept {
        return lerp(float3(0.2f, 0.7f, 0.9f), float3(1.0f, 0.0f, 1.0f), d);
    };

    Callable rotate = [](Float2 p, Float a) noexcept {
        Var c = cos(a);
        Var s = sin(a);
        Float2x2 m{c, -s, s, c};
        return m * p;
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

    Callable shader = [&](Float2 xy, Float2 resolution, Float time, Float4) noexcept {
        Var uv = (xy - resolution * 0.5f) / ite(resolution.x < resolution.y, resolution.x, resolution.y);
        Var ro = make_float3(rotate(make_float2(0.0f, -50.0f), time), 0.0f).xzy();
        Var cf = normalize(-ro);
        Var cs = normalize(cross(cf, make_float3(0.0f, 1.0f, 0.0f)));
        Var cu = normalize(cross(cf, cs));
        Var uuv = ro + cf * 3.0f + uv.x * cs + uv.y * cu;
        Var rd = normalize(uuv - ro);
        return rm(ro, rd, time).xyz();
    };

    gui::ShaderToy::run(argv[0], shader);
}
