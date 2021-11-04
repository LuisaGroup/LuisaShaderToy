//
// Created by Mike Smith on 2021/6/25.
//

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

// Credit: https://www.shadertoy.com/view/XlfGRj
int main(int argc, char *argv[]) {

    static constexpr auto iterations = 17u;
    static constexpr auto formuparam = 0.53f;
    static constexpr auto volsteps = 20u;
    static constexpr auto stepsize = 0.1f;
    static constexpr auto zoom = 0.800f;
    static constexpr auto tile = 0.850f;
    static constexpr auto speed = 0.010f;
    static constexpr auto brightness = 0.0015f;
    static constexpr auto darkmatter = 0.300f;
    static constexpr auto distfading = 0.730f;
    static constexpr auto saturation = 0.850f;

    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse, Float3) noexcept {
        //get coords and direction
        Var uv = fragCoord.xy() / iResolution.xy() - .5f;
        uv.y *= iResolution.y / iResolution.x;
        Var dir = make_float3(uv * zoom, 1.0f);
        Var time = iTime * speed + .25f;

        //mouse rotation
        Var a1 = .5f + iMouse.x / iResolution.x * 2.f;
        Var a2 = .8f + iMouse.y / iResolution.y * 2.f;
        Var rot1 = make_float2x2(cos(a1), -sin(a1), sin(a1), cos(a1));
        Var rot2 = make_float2x2(cos(a2), -sin(a2), sin(a2), cos(a2));
        dir = make_float3(rot1 * dir.xz(), dir.y).xzy();
        dir = make_float3(rot2 * dir.xy(), dir.z);
        Var from = make_float3(1.0f, 0.5f, 0.5f);
        from += make_float3(time * 2.f, time, -2.f);
        from = make_float3(rot1 * from.xz(), from.y).xzy();
        from = make_float3(rot2 * from.xy(), from.z);

        //volumetric rendering
        Var s = 0.1f;
        Var fade = 1.f;
        Var v = make_float3(0.f);
        for (auto r = 0u; r < volsteps; r++) {
            Var p = from + s * dir * .5f;
            p = abs(float3(tile) - mod(p, float3(tile * 2.f)));// tiling fold
            Var pa = 0.0f;
            Var a = 0.0f;
            for (auto i = 0u; i < iterations; i++) {
                p = abs(p) / dot(p, p) - formuparam;// the magic formula
                a += abs(length(p) - pa);           // absolute sum of average change
                pa = length(p);
            }
            Var dm = max(0.f, darkmatter - a * a * .001f);//dark matter
            a *= a * a;                                   // add contrast
            if (r > 6) fade *= 1.f - dm;                  // dark matter, don't render near
            v += fade;
            v += make_float3(s, s * s, s * s * s * s) * a * brightness * fade;// coloring based on distance
            fade *= distfading;                                               // distance fading
            s += stepsize;
        }
        v = lerp(make_float3(length(v)), v, saturation);//color adjust
        return v * .01f;
    };

    gui::ShaderToy::run(argv[0], mainImage, make_uint2(512u));
}
