
//
// Created by Yawanari.fst on 2021/6/28.
// Credit: https://www.shadertoy.com/view/wlKXWc

#include <gui/shader_toy.h>
#include <math.h>
using namespace luisa;
using namespace luisa::compute;

int main(int argc, char *argv[]) {

    static constexpr auto PI = 3.1415926f;
    static constexpr auto TAU = PI * 2.0f;
    static constexpr auto E = 0.01f;

    struct Ray{
        Float3 pos;
        Float3 dir;
    };

    Callable rotate2D =[](Float rad) {
        Var c = cos(rad);
        Var s = sin(sin);
        return make_float2X2(c, s,
                            -s, c);

    };

    Callable de = [&rotate2D](Float3 p, Float iTime) {
        Var d = 100.0f;
        Var a = 0.0f;
        p = make_float3(p.x, rotate2D(PI/5.0f)*p.yz());
        p.y -= 0.5f;

        //reaction
        Var reaction = make_float3(cos(iTime), 0.0f, sin(iTime)) * 3.0f;
        p += exp(-length(reaction - p) * 1.0) * normalize(reaction - p);

        //cales
        Var r = atan(p.z, p.x) * 3.0f;
        static constexpr auto ite_c = 50;
        static constexpr auto ite_f = 50.0f;
        for (auto i : range(ite_c)) {
            Var ite_r = 0.0f;
            r += 0.5f / ite_f * TAU;
            Var s = 0.5f + sin(ite_r * 1.618f * TAU) * 0.25f;
            s += sin(iTime + ite_r) * 0.1f;
            Var q = make_float2(length(p.xz())+cos(r)*s-3.0f, p.y+sin(r)*s);
            Var dd = length(q) - 0.035f;
            a = ite(dd < d, ite_c, a);
            d = min(d, dd);
            ite_r += 1.0f;
        }

        // sphere
        Var dd = length(p - reaction) - 0.1f;
        a = ite(dd < d, 0.0f, a);
        d = min(d, dd);

    return make_float2(d, a);
    };

    Callable trace = [&](Ray ray, Float3 color, Float md, Float iTime) {
        Var ad = 0.0f;
        for ( auto i : range(128)) {
            Var o = de(ray.pos, iTime);
            if_(o.x<E, [&]{
                color = lerp(make_float3(0.1f, 0.1f, 0.5f), make_float3(0.0f, 0.0f, 1.0f), fract(o.y * 1.618f));
                color = lerp(make_float3(1.0f, 1.0f, 1.0f), color, make_float3(0.05f, fract(o.y * 1.618f)));
                color = lerp(make_float3(0.175f, 0.1f, 0.1f), color, make_float3(0.35f, fract(o.y * 1.618f + 0.9f)));
                color *= exp(-(i/128.0f) * 15.0f);
            }).else_([&]{
                o.x *= 0.6f;
                ray.pos += ray.dir * o.x;
                ad += o.x;
                if_(ad > md, [&]{
                    i = 129;
                    });
            });
        }
        color = lerp(float3(0.0f), float3(1.0f), ray.dir.y * ray.dir.y);
    };


    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse){
        Var p = (fragCoord.xy() * 2.0f - iResolution.xy()) / min(iResolution.x, iResolution.y);
        Var color = float3(0.0f);

        Var view = make_float3(0.0f, 0.0f, 10.0f);
        Var at = normalize(make_float3(0.0f, 0.0f, 0.0f) - view);
        Var right = normalize(cross(at, make_float3(0.0f, 1.0f, 0.0f)));
        Var up = cross(right, at);
        Var focallength = 3.0f;

        Ray ray;
        ray.pos = view;
        ray.dir = normalize(right * p.x + up * p.y + at * focallength);
    
        trace(ray, color, 20.0f);

    return color = pow(color, float3(0.454545f));   
    };

    gui::ShaderToy::run(argv[0], mainImage, 512u);
}