//
// Created by Yawanari.fst on 2021/6/28.
// Credit: https://www.shadertoy.com/view/lsjGRV

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

int main(int argc, char *argv[]) {

    static constexpr auto i3 = 0.5773502691896258f;
    static constexpr auto r = 0.40824829046386302f;

    static constexpr auto i = 0.3333333333333333f;
    static constexpr auto j = 0.6666666666666666f;

    static constexpr auto lrad = 0.015f;
    static constexpr auto trad = 0.06f;
    static constexpr auto fogv = 0.025f;

    static constexpr auto dmax = 20.0f;
    static constexpr auto rayiter = 60;

    static constexpr auto wrap = 64.0f;

    auto L = normalize(float3(0.1f, 1.0f, 0.5f));

    static constexpr auto axis = float3(1.0f, 1.0f, 0.0f);
    static constexpr auto tgt = float3(1.0f, 1.7f, 1.1f);
    static constexpr auto cpos = tgt + axis;
    static constexpr auto vel = 0.2f * axis;
    static constexpr auto key_G = 71.5f / 256.0f;

    Callable hash = [](Float3 x) {
        return fract(87.3f * dot(x, float3(0.1f, 0.9f, 0.7f)));
    };

    Callable line = [](Float3 p0, Float3 p1, Float3 p) {
        Var dp0 = p - p0;
        Var dp10 = p1 - p0;
        Var u = clamp(dot(dp0, dp10) / dot(dp10, dp10), -5.0f, 5.0f);
        return distance(lerp(p0, p1, u), p) - 0.5f * lrad;
    };

    Callable opU = [](Float2 a, Float2 b) {
        return ite(a.x < b.x, a, b);
    };

    Callable hueOf = [](Float3 pos) {
        return cos(2.0f * dot(2.0f * pos, make_float3(0.3f, 0.7f, 0.4f))) * 0.49f + 0.5f;
    };

    Callable round2 = [](Float3 x, Float3 a) {
        return 2.0f * floor(0.5f * (x + 1.0f - a)) + a;
    };

    Callable pdist = [](Float3 p, Float3 q) {
        Var pq = p - q;
        return make_float4(q, dot(pq, pq));
    };

    Callable pselect = [](Float4 a, Float4 b) {
        return ite(a.w < b.w, a, b);
    };

    Callable torus = [](Float3 a, Float3 b, Float3 pos) {
        pos -= 0.5f * (a + b);
        Var n = normalize(b - a);
        return distance(pos, r * normalize(pos - n * dot(n, pos))) - trad;
    };

    Callable permute = [](Float3 e, Float3 f, Float3 g, Float3 h, Float p) {
        return ite(p < i,
                   make_float4x4(make_float4(e, 1.0f), make_float4(f, 1.0f), make_float4(g, 1.0f), make_float4(h, 1.0f)),
                   ite(p < j, make_float4x4(make_float4(e, 1.0f), make_float4(g, 1.0f), make_float4(f, 1.0f), make_float4(h, 1.0f)),
                       make_float4x4(make_float4(e, 1.0f), make_float4(h, 1.0f), make_float4(f, 1.0f), make_float4(g, 1.0f))));
    };

    Callable randomBasis = [](Float p) {
        return ite(p < i, make_float3(1.0f, 0.0f, 0.0f),
                   ite(p < j, make_float3(0.0f, 1.0f, 0.0f), make_float3(0.0f, 0.0f, 1.0f)));
    };

    Callable randomPerp = [](Float3 v, Float p) {
        return ite(v.x > 0.0f, ite(p < 0.5f, make_float3(0.0f, 1.0f, 0.0f), make_float3(0.0f, 0.0f, 1.0f)),
                   ite(v.y > 0.0f, ite(p < 0.5f, make_float3(1.0f, 0.0f, 0.0f), make_float3(0.0f, 0.0f, 1.0f)),
                       ite(p < 0.5f, make_float3(1.0f, 0.0f, 0.0f), make_float3(0.0f, 1.0f, 0.0f))));
    };

    Callable map = [&](Float3 pos, Float iTime) {
        Var orig = pos;
        pos = mod(pos + mod(iTime * vel, wrap), wrap);
        Var a = round2(pos, float3(1.0f));
        Var h = round2(pos, float3(0.0f));

        Var b = make_float3(a.x, h.y, h.z);
        Var c = make_float3(h.x, a.y, h.z);
        Var d = make_float3(h.x, h.y, a.z);
        Var e = make_float3(h.x, a.y, a.z);
        Var f = make_float3(a.x, h.y, a.z);
        Var g = make_float3(a.x, a.y, h.z);

        // o is the closest octahedron center
        Var o = pselect(pselect(pdist(pos, a), pdist(pos, b)),
                        pselect(pdist(pos, c), pdist(pos, d)))
                    .xyz();

        // t is the closest tetrahedron center
        Var t = floor(pos) + 0.5f;

        // normal points towards o
        // so bd is positive inside octahedron, negative inside tetrahedron
        Var omt = o.xyz() - t.xyz();
        Var bd = dot(pos - o.xyz(), omt * 2.0f * i3) + i3;

        Var m = permute(e, f, g, h, hash(mod(t, wrap)));

        Var t1 = torus(m[0].xyz(), m[1].xyz(), pos);
        Var t2 = torus(m[2].xyz(), m[3].xyz(), pos);

        Var p = hash(mod(o, wrap));
        Var b1 = randomBasis(fract(85.17f * p));
        Var b2 = randomPerp(b1, fract(63.61f * p + 4.2f));
        Var b3 = randomPerp(b1, fract(43.79f * p + 8.3f));

        Var po = pos - o;

        Var o1 = torus(b1, b2, po);
        Var o2 = torus(b1, -b2, po);
        Var o3 = torus(-b1, b3, po);
        Var o4 = torus(-b1, -b3, po);

        Var noodle = make_float2(min(max(bd, min(t1, t2)),
                                     max(-bd, min(min(o1, o2), min(o3, o4)))),
                                 hueOf(orig + 0.5f * vel * iTime));

#define SHOW_GRIDS 1
#if SHOW_GRIDS
        Var dline = line(e, f, pos);
        dline = min(dline, line(e, g, pos));
        dline = min(dline, line(e, h, pos));
        dline = min(dline, line(f, g, pos));
        dline = min(dline, line(f, h, pos));
        dline = min(dline, line(g, h, pos));
        Var grid = make_float2(dline, 2.0f);
        noodle.x += 0.1f * trad;
        noodle.y = hash(mod(ite(bd < 0.0f, t, o), wrap));
        noodle = opU(grid, noodle);
#endif
        return noodle;
    };

    Callable hue = [](Float h) {
        Var c = mod(h * 6.0f + float3(2.0f, 0.0f, 4.0f), 6.0f);
        return ite(h > 1.0f, float3(0.5f), clamp(min(c, -c + 4.0f), 0.0f, 1.0f));
    };

    Callable castRay = [&map](Float3 ro, Float3 rd, Float maxd, Float iTime) {
        static constexpr auto precis = 0.0001f;
        Var h = precis * 2.0f;
        Var t = 0.0f;
        Var m = -1.0f;
        for (auto i = 0; i < rayiter; i++) {
            if_(!(abs(h) < precis || t > maxd), [&] {
                t += h;
                Var res = map(ro + rd * t, iTime);
                h = res.x;
                m = res.y;
            });
        }
        return make_float2(t, m);
    };

    Callable calcNormal = [&map](Float3 pos, Float iTime) {
        static constexpr auto eps = float3(0.0001f, 0.0f, 0.0f);
        Var nor = make_float3(
            map(pos + eps.xyy(), iTime).x - map(pos - eps.xyy(), iTime).x,
            map(pos + eps.yxy(), iTime).x - map(pos - eps.yxy(), iTime).x,
            map(pos + eps.yyx(), iTime).x - map(pos - eps.yyx(), iTime).x);
        return normalize(nor);
    };

    Callable shade = [&hue, &calcNormal, &castRay, &L](Float3 ro, Float3 rd, Float iTime) {
        Var tm = castRay(ro, rd, dmax, iTime);
        Var ret = float3(1.0f);
        if_(tm.y >= 0.0f, [&] {
            Var n = calcNormal(ro + tm.x * rd, iTime);
            Var fog = exp(-tm.x * tm.x * fogv);
            Var color = hue(tm.y) * 0.55f + 0.45f;
            Var diffamb = (0.5f * dot(n, L) + 0.5f) * color;
            Var R = 2.0f * n * dot(n, L) - L;
            Var spec = 0.2f * pow(clamp(-dot(R, rd), 0.0f, 1.0f), 6.0f);
            ret = fog * (diffamb + spec);
        });
        return ret;
    };

    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse) {
        static constexpr auto yscl = 720.0f;
        static constexpr auto f = 900.0f;
        static constexpr auto up = float3(0.0f, 1.0f, 0.0f);

        Var uv = (fragCoord - 0.5f * iResolution) * yscl / iResolution.y;

        Var rz = normalize(tgt - cpos);
        Var rx = normalize(cross(rz, up));
        Var ry = cross(rx, rz);

        Var thetax = 0.0f;
        Var thetay = 0.0f;

        if_(max(iMouse.x, iMouse.y) > 20.0f, [&] {
            thetax = (iMouse.y - 0.5f * iResolution.y) * 3.14f / iResolution.y;
            thetay = (iMouse.x - 0.5f * iResolution.x) * -6.28f / iResolution.x;
        });

        Var cx = cos(thetax);
        Var sx = sin(thetax);
        Var cy = cos(thetay);
        Var sy = sin(thetay);

        Var Rx = make_float3x3(1.0f, 0.0f, 0.0f,
                               0.0f, cx, sx,
                               0.0f, -sx, cx);
        Var Ry = make_float3x3(cy, 0.0f, -sy,
                               0.0f, 1.0f, 0.0f,
                               sy, 0.0f, cy);
        Var R = make_float3x3(rx, ry, rz);
        Var Rt = make_float3x3(rx.x, ry.x, rz.x,
                               rx.y, ry.y, rz.y,
                               rx.z, ry.z, rz.z);

        Var rd = R * Rx * Ry * normalize(make_float3(uv, f));
        Var ro = tgt + R * Rx * Ry * Rt * (cpos - tgt);

        return shade(ro, rd, iTime);
    };

    gui::ShaderToy::run(argv[0], mainImage, 512u);
}
