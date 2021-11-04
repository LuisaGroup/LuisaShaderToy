//
// Created by Mike Smith on 2021/6/25.
//

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

// Credit: https://www.shadertoy.com/view/MdX3zr
int main(int argc, char *argv[]) {

    Callable noise = [](Float3 p) noexcept {
        Var i = floor(p);
        Var a = dot(i, float3(1.f, 57.f, 21.f)) + float4(0.f, 57.f, 21.f, 78.f);
        Var f = cos((p - i) * acos(-1.f)) * (-.5f) + .5f;
        a = lerp(sin(cos(a) * a), sin(cos(1.f + a) * (1.f + a)), f.x);
        Var axy = lerp(a.xz(), a.yw(), f.y);
        return lerp(axy.x, axy.y, f.z);
    };

    Callable sphere = [](Float3 p, Float4 spr) noexcept {
        return length(spr.xyz() - p) - spr.w;
    };

    Callable flame = [&](Float3 p, Float iTime) noexcept {
        Var d = sphere(p * float3(1.f, .5f, 1.f), float4(.0f, -1.f, .0f, 1.f));
        return d + (noise(p + make_float3(.0f, iTime * 2.f, .0f)) + noise(p * 3.f) * .5f) * .25f * p.y;
    };

    Callable scene = [&](Float3 p, Float iTime) noexcept {
        return min(100.f - length(p), abs(flame(p, iTime)));
    };

    Callable raymarch = [&](Float3 org, Float3 dir, Float iTime) noexcept {
        static constexpr auto eps = 0.02f;
        Var d = 0.0f;
        Var glow = 0.0f;
        Var p = org;
        Var glowed = false;

        for (int i = 0; i < 64; i++) {
            d = scene(p, iTime) + eps;
            p += d * dir;
            if_(d > eps, [&] {
                glowed = glowed || flame(p, iTime) < .0f;
                glow = ite(glowed, static_cast<float>(i) / 64.0f, glow);
            });
        }
        return make_float4(p, glow);
    };

    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse, Float3) noexcept {
        Var v = 2.0f * fragCoord / iResolution - 1.0f;
        v.x *= iResolution.x / iResolution.y;

        static constexpr auto org = float3(0.f, -2.f, 4.f);
        Var dir = normalize(make_float3(v.x * 1.6f, -v.y, -1.5f));

        Var p = raymarch(org, dir, iTime);
        Var glow = p.w;
        Var col = lerp(float3(1.f, .5f, .1f), float3(0.1f, .5f, 1.f), p.y * .02f + .4f);
        return lerp(float3(0.f), col, pow(glow * 2.f, 4.f));
    };

    gui::ShaderToy::run(argv[0], mainImage, make_uint2(540, 360));
}
