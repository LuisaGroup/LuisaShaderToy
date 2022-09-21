
//
// Created by Yawanari.fst on 2021/6/28.
// Credit: https://www.shadertoy.com/view/wlKXWc

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

struct Ray {
    float3 pos;
    float3 dir;
};

LUISA_STRUCT(Ray, pos, dir) {};

int main(int argc, char *argv[]) {

    static constexpr auto PI = 3.1415926f;
    static constexpr auto TAU = PI * 2.0f;
    static constexpr auto E = 0.01f;

    Callable rotate2D = [](Float rad) {
        Var c = cos(rad);
        Var s = sin(rad);
        return make_float2x2(c, -s, s, c);
    };

    Callable de = [&rotate2D](Float3 p, Float iTime) {
        Var d = 100.0f;
        Var a = 0.0f;
        p = make_float3(p.x, rotate2D(PI / 5.0f) * p.yz());
        p.y -= 0.5f;

        //reaction
        Var reaction = make_float3(cos(iTime), 0.0f, sin(iTime)) * 3.0f;
        p += exp(-length(reaction - p) * 1.0f) * normalize(reaction - p);

        //cables
        Var r = atan2(p.z, p.x) * 3.0f;
        static constexpr auto iter_count = 50;
        for (auto i = 0; i < iter_count; i++) {
            r += 0.5f / static_cast<float>(iter_count) * TAU;
            Var s = 0.5f + sin(static_cast<float>(i) * 1.618f * TAU) * 0.25f;
            s += sin(iTime + static_cast<float>(i)) * 0.1f;
            Var q = make_float2(length(p.xz()) + cos(r) * s - 3.0f, p.y + sin(r) * s);
            Var dd = length(q) - 0.035f;
            a = ite(dd < d, static_cast<float>(i), a);
            d = min(d, dd);
        }

        // sphere
        Var dd = length(p - reaction) - 0.1f;
        a = ite(dd < d, 0.0f, a);
        d = min(d, dd);

        return make_float2(d, a);
    };

    Callable trace = [&](Var<Ray> ray, Float3 color, Float md, Float iTime) {
        Var ad = 0.0f;
        Var early_return = false;
        for (auto i : range(128)) {
            Var o = de(ray.pos, iTime);
            if_(o.x < E, [&] {
                color = lerp(make_float3(0.1f, 0.1f, 0.5f), make_float3(0.0f, 0.0f, 1.0f), fract(o.y * 1.618f));
                color = lerp(make_float3(1.0f, 1.0f, 1.0f), color, step(0.05f, fract(o.y * 1.618f)));
                color = lerp(make_float3(0.175f, 0.1f, 0.1f), color, step(0.35f, fract(o.y * 1.618f + 0.9f)));
                color *= exp(-i / 128.0f * 15.0f);
                early_return = true;
                break_();
            });
            o.x *= 0.6f;
            ray.pos += ray.dir * o.x;
            ad += o.x;
            if_(ad > md, [&] { break_(); });
        }
        return ite(early_return, color, lerp(float3(0.0f), float3(1.0f), ray.dir.y * ray.dir.y));
    };

    gui::ShaderToy toy{argc, argv};
    toy.run([&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse) {
        Var p = (fragCoord * 2.0f - iResolution) / min(iResolution.x, iResolution.y);
        Var color = float3(0.0f);

        Var view = float3(0.0f, 0.0f, 10.0f);
        Var at = normalize(float3(0.0f, 0.0f, 0.0f) - view);
        Var right = normalize(cross(at, float3(0.0f, 1.0f, 0.0f)));
        Var up = cross(right, at);
        Var focallength = 3.0f;

        Var<Ray> ray;
        ray.pos = view;
        ray.dir = normalize(right * p.x + up * p.y + at * focallength);

        color = trace(ray, color, 20.0f, iTime);
        return pow(color, float3(0.454545f));
    });
}
