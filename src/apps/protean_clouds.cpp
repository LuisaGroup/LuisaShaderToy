//
// Created by Mike Smith on 2021/6/25.
//

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

// Credit: https://www.shadertoy.com/view/3l23Rh
int main(int argc, char *argv[]) {

    Callable rot = [](Float a) noexcept {
        Var c = cos(a);
        Var s = sin(a);
        return make_float2x2(c, -s, s, c);
    };

    static constexpr auto m3 = transpose(make_float3x3(
                                   0.33338f, 0.56034f, -0.71817f,
                                   -0.87887f, 0.32651f, -0.15323f,
                                   0.15162f, 0.69596f, 0.61339f))
                               * 1.93f;

    Callable mag2 = [](Float2 p) noexcept { return dot(p, p); };
    Callable linstep = [](Float mn, Float mx, Float x) noexcept { return clamp((x - mn) / (mx - mn), 0.f, 1.f); };

    Callable disp = [](Float t) noexcept {
        return make_float2(sin(t * 0.22f) * 1.f, cos(t * 0.175f) * 1.f) * 2.f;
    };

    Callable map = [&](Float3 p, Float iTime, Float prm1, Float2 bsMo) noexcept {
        Var p2 = p;
        p2 = make_float3(p2.xy() - disp(p.z).xy(), p2.z);
        p = make_float3(rot(sin(p.z + iTime) * (0.1f + prm1 * 0.05f) + iTime * 0.09f) * p.xy(), p.z);
        Var cl = mag2(p2.xy());
        Var d = 0.0f;
        p *= .61f;
        Var z = 1.f;
        Var trk = 1.f;
        Var dspAmp = 0.1f + prm1 * 0.2f;
        for (int i = 0; i < 5; i++) {
            p += sin(p.zxy() * 0.75f * trk + iTime * trk * .8f) * dspAmp;
            d -= abs(dot(cos(p), sin(p.yzx())) * z);
            z *= 0.57f;
            trk *= 1.4f;
            p = m3 * p;
        }
        d = abs(d + prm1 * 3.f) + prm1 * .3f - 2.5f + bsMo.y;
        return make_float2(d + cl * .2f + 0.25f, cl);
    };

    Callable render = [&](Float3 ro, Float3 rd, Float time, Float iTime, Float prm1, Float2 bsMo) noexcept {
        static constexpr auto ldst = 8.f;
        Var rez = make_float4(0.0f);
        Var lpos = make_float3(disp(time + ldst) * 0.5f, time + ldst);
        Var t = 1.5f;
        Var fogT = 0.f;
        for (auto i : range(130)) {

            if_(rez.w > 0.99f, [] { break_(); });

            Var pos = ro + t * rd;
            Var mpv = map(pos, iTime, prm1, bsMo);
            Var den = clamp(mpv.x - 0.3f, 0.f, 1.f) * 1.12f;
            Var dn = clamp((mpv.x + 2.f), 0.f, 3.f);

            Var col = make_float4(0.0f);
            if_(mpv.x > 0.6f, [&] {
                col = make_float4(sin(float3(5.f, 0.4f, 0.2f) + mpv.y * 0.1f + sin(pos.z * 0.4f) * 0.5f + 1.8f) * 0.5f + 0.5f, 0.08f);
                col *= den * den * den;
                col = make_float4(col.xyz() * linstep(4.f, -2.5f, mpv.x) * 2.3f, col.w);
                Var dif = clamp((den - map(pos + .8f, iTime, prm1, bsMo).x) / 9.f, 0.001f, 1.f);
                dif += clamp((den - map(pos + .35f, iTime, prm1, bsMo).x) / 2.5f, 0.001f, 1.f);
                col = make_float4(col.xyz() * den * (float3(0.005f, .045f, .075f) + 1.5f * float3(0.033f, 0.07f, 0.03f) * dif), col.w);
            });

            Var fogC = exp(t * 0.2f - 2.2f);
            col += float4(0.06f, 0.11f, 0.11f, 0.1) * clamp(fogC - fogT, 0.f, 1.f);
            fogT = fogC;
            rez = rez + col * (1.f - rez.w);
            t += clamp(0.5f - dn * dn * .05f, 0.09f, 0.3f);
        }
        return clamp(rez, 0.0f, 1.0f);
    };

    Callable getsat = [](Float3 c) noexcept {
        Var mi = min(min(c.x, c.y), c.z);
        Var ma = max(max(c.x, c.y), c.z);
        return (ma - mi) / (ma + 1e-7f);
    };

    //from my "Will it blend" shader (https://www.shadertoy.com/view/lsdGzN)
    Callable iLerp = [&](Float3 a, Float3 b, Float x) noexcept {
        Var ic = lerp(a, b, x) + float3(1e-6f, 0.f, 0.f);
        Var sd = abs(getsat(ic) - lerp(getsat(a), getsat(b), x));
        Var dir = normalize(make_float3(2.f * ic.x - ic.y - ic.z, 2.f * ic.y - ic.x - ic.z, 2.f * ic.z - ic.y - ic.x));
        Var lgt = dot(float3(1.0f), ic);
        Var ff = dot(dir, normalize(ic));
        ic += 1.5f * dir * sd * ff * lgt;
        return clamp(ic, 0.f, 1.f);
    };

    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse) noexcept {
        Var q = fragCoord.xy() / iResolution.xy();
        Var p = (fragCoord.xy() - 0.5f * iResolution.xy()) / iResolution.y;
        Var bsMo = (iMouse.xy() - 0.5f * iResolution.xy()) / iResolution.y;

        static constexpr auto dspAmp = .85f;
        static constexpr auto tgtDst = 3.5f;

        Var time = iTime * 3.f;
        Var ro = make_float3(0.0f, 0.0f, time);

        ro += make_float3(sin(iTime) * 0.5f, sin(iTime * 1.f) * 0.f, 0.0f);
        ro = make_float3(ro.xy() + disp(ro.z) * dspAmp, ro.z);

        Var target = normalize(ro - make_float3(disp(time + tgtDst) * dspAmp, time + tgtDst));
        ro.x -= bsMo.x * 2.f;
        Var rightdir = normalize(cross(target, float3(0.0f, 1.0f, 0.0f)));
        Var updir = normalize(cross(rightdir, target));
        rightdir = normalize(cross(updir, target));
        Var rd = normalize((p.x * rightdir + p.y * updir) * 1.f - target);
        rd = make_float3(rot(-disp(time + 3.5f).x * 0.2f + bsMo.x) * rd.xy(), rd.z);
        Var prm1 = smoothstep(-0.4f, 0.4f, sin(iTime * 0.3f));
        Var scn = render(ro, rd, time, iTime, prm1, bsMo);
        Var col = scn.xyz();
        col = iLerp(col.zyx(), col.xyz(), clamp(1.f - prm1, 0.05f, 1.f));
        col = pow(col, float3(.55f, 0.65f, 0.6)) * float3(1.f, .97f, .9f);
        return col * pow(16.0f * q.x * q.y * (1.0f - q.x) * (1.0f - q.y), 0.12f) * 0.7f + 0.3f;//Vign
    };

    gui::ShaderToy::run(argv[0], mainImage, make_uint2(512u));
}
