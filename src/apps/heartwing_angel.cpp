//
// Created by Mike Smith on 2021/6/25.
//

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

// Credit: https://www.shadertoy.com/view/4l23zc
int main(int argc, char *argv[]) {

    gui::ShaderToy toy{argc, argv};

    Callable JuliaFractal = [](Float2 c, Float2 c2, Float animparam, Float anim2) noexcept {
        Var z = c;
        Var mean = 0.0f;
        for (int i = 0; i < 64; i++) {
            Var a = make_float2(z.x, abs(z.y));
            Var b = atan2(a.y * (0.99f + animparam * 9.0f), a.x + .110765432f + animparam);
            b = ite(b > 0.0f, b - 6.303431307f + animparam * 3.1513f, b);
            z = make_float2(log(length(a * (0.98899f - (animparam * 2.70f * anim2)))), b) + c2;
            if (i > 0) { mean += length(z / a * b); }
            mean += a.x - (b * 77.0f / length(a * b));
            mean = clamp(mean, 111.0f, 99999.0f);
        }
        mean /= 131.21f;
        Var ci = 1.0f - fract(log2(.5f * log2(mean / (0.57891895f - abs(animparam * 141.0f)))));
        return make_float3(.5f + .5f * cos(6.f * ci + 0.0f),
                           .5f + .75f * cos(6.f * ci + 0.14f),
                           .5f + .5f * cos(6.f * ci + 0.7f));
    };

    toy.run([&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse) noexcept {
        static constexpr auto timeVal = 56.48f - 20.1601f;
        static constexpr auto rot = 3.141592654f * 0.5f;
        Var animWings = 0.004f * cos(iTime * 0.5f);
        Var animFlap = 0.011f * sin(iTime * 1.0f);
        Var uv = fragCoord - iResolution * .5f;
        uv /= iResolution.x * 1.5113f * abs(sin(timeVal));
        uv.y -= animWings * 5.0f;
        Var tuv = uv * 125.0f;
        uv.x = tuv.x * cos(rot) - tuv.y * sin(rot);
        uv.y = 1.05f * tuv.x * sin(rot) + tuv.y * cos(rot);
        Var juliax = tan(timeVal) * 0.011f + 0.02f / (fragCoord.y * 0.19531f * (1.0f - animFlap));
        Var juliay = cos(timeVal * 0.213f) * (0.022f + animFlap) + 5.66752f - (juliax * 1.5101f);
        Var tapU = 25.5f / iResolution.x;
        Var tapV = 25.5f / iResolution.y;
        Var color = JuliaFractal(uv + float2(0.0f), make_float2(juliax, juliay), animWings, animFlap);
        color += JuliaFractal(uv + make_float2(tapU, tapV), make_float2(juliax, juliay), animWings, animFlap);
        color += JuliaFractal(uv + make_float2(-tapU, -tapV), make_float2(juliax, juliay), animWings, animFlap);
        color *= 0.3333f;
        color = float3(1.0f) - color.zyx();
        return color;
    });
}
