//
// Created by Mike Smith on 2021/6/25.
//

#include <gui/shader_toy.h>

using namespace luisa;
using namespace luisa::compute;

// Credit: https://www.shadertoy.com/view/WdjBWc
int main(int argc, char *argv[]) {

    Callable rot = [](Float a) noexcept {
        Var s = sin(a);
        Var c = cos(a);
        return make_float2x2(c, -s, s, c);
    };

    Callable lpNorm = [](Float3 p, Float n) noexcept {
        p = pow(abs(p), make_float3(n));
        return pow(p.x + p.y + p.z, 1.0f / n);
    };

    Callable pSFold = [](Float2 p, Float n) noexcept {
        Var h = floor(log2(n));
        Var a = 6.2831f * exp2(h) / n;
        for (auto i : range(h.cast<uint>() + 2u)) {
            Var v = make_float2(-cos(a), sin(a));
            Var g = dot(p, v);
            p -= (g - sqrt(g * g + 5e-3f)) * v;
            a *= 0.5f;
        }
        return p;
    };

    Callable sFold45 = [](Float2 p, Float k) noexcept {
        Var v = float2(-1.0f, 1.0f) * 0.7071f;
        Var g = dot(p, v);
        return p - (g - sqrt(g * g + k)) * v;
    };

    Callable frameBox = [&](Float3 p, Float3 s, Float r) noexcept {
        p = abs(p) - s;
        p = make_float3(p.x, sFold45(p.yz(), 1e-3f));
        p = make_float3(sFold45(p.xy(), 1e-3f), p.z);
        p.x = max(0.0f, p.x);
        return lpNorm(p, 5.0f) - r;
    };

    Callable sdRoundBox = [](Float3 p, Float3 b, Float r) noexcept {
        Var q = abs(p) - b;
        return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f) - r;
    };

    Callable deObj = [&](Float3 p) noexcept {
        return min(min(sdRoundBox(p, float3(0.3f), 0.1f),
                       frameBox(p, float3(0.7f), 0.05f)),
                   frameBox(p, float3(0.5f), 0.01f));
    };

    Callable map = [&](Float3 p, Float g, Float time) noexcept {
        Var de = 1e9f;
        p.z -= time * 1.5f;
        p.z = mod(p.z, 12.0f) - 6.0f;
        Var q = p;
        p = make_float3(pSFold(p.xy(), 6.0f), p.z);
        p.y -= 5.0f;
        Var s = 1.0f;
        for (auto i = 0; i < 6; i++) {
            p = make_float3(abs(p.xy()) - 0.5f, p.z);
            p.z = abs(p.z) - 0.3f;
            p = make_float3(rot(-0.05f) * p.xy(), p.z);
            p = make_float3(p.x, rot(0.1f) * p.zy()).xzy();
            s *= 0.7f;
            p *= s;
            p = make_float3(rot(0.05f) * p.xy(), p.z);
            p.y -= 0.3f;
            Var sp = p / s;
            de = min(de, min(sdRoundBox(sp, float3(0.3f), 0.1f),
                             frameBox(sp, float3(0.7f), 0.05f)));
        }
        q.z -= clamp(q.z, -1.f, 1.f);
        Var d = length(q) - 0.5f;
        g += 0.1f / (0.2f + d * d * 5.0f);// Distance glow by balkhan
        de = min(de, d + 0.2f);
        return multiple(de, g);
    };

    Callable calcNormal = [&](Float3 pos, Float g0, Float time) noexcept {
        auto e = float2(1.0f, -1.0f) * 0.002f;
        auto [m1, g1] = map(pos + e.xyy(), g0, time);
        auto [m2, g2] = map(pos + e.yyx(), g1, time);
        auto [m3, g3] = map(pos + e.yxy(), g2, time);
        auto [m4, g4] = map(pos + e.xxx(), g3, time);
        return multiple(normalize(e.xyy() * m1 + e.yyx() * m2 + e.yxy() * m3 + e.xxx() * m4), g4);
    };

    Callable march = [&](Float3 ro, Float3 rd, Float t_near, Float t_far, Float g, Float time) noexcept {
        Var t = t_near;
        Var ret = t_far;
        for (auto i : range(100)) {
            auto [d, new_g] = map(ro + rd * t, g, time);
            g = new_g;
            t += d;
            if_(d < 0.001f, [&] {
                ret = t;
                break_();
            });
            if_(t >= t_far, [&] { break_(); });
        }
        return multiple(ret, g);
    };

    Callable calcShadow = [&](Float3 light, Float3 ld, Float len, Float g, Float time) noexcept {
        auto [depth, new_g] = march(light, ld, 0.0f, len, g, time);
        return multiple(step(len - depth, 0.01f), new_g);
    };

    Callable doColor = [](Float3 p) noexcept {
        return float3(0.3, 0.5, 0.8) + cos(p * 0.2f) * .5f + .5f;
    };

    Callable reflect = [](Float3 I, Float3 N) noexcept {
        return I - 2.0f * dot(N, I) * N;
    };

    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float time, Float4 cursor) noexcept {
        Var uv = (fragCoord * 2.0f - iResolution.xy()) / iResolution.y;
        auto ro = float3(2.5, 3.5, 8);
        auto ta = float3(-1, 0, 0);
        auto w = normalize(ta - ro);
        auto u = normalize(cross(w, float3(0.0f, 1.0f, 0.0f)));
        Var rd = float3x3(u, cross(u, w), w) * normalize(make_float3(uv, 2.0f));
        Var col = make_float3(0.05f, 0.05f, 0.1f);
        static constexpr auto maxd = 80.0f;
        auto [t0, g0] = march(ro, rd, 0.0f, maxd, 0.0f, time);
        Var g = g0;
        Var t = t0;
        if_(t < maxd, [&] {
            Var p = ro + rd * t;
            col = doColor(p);
            auto [n1, g1] = calcNormal(p, g, time);
            Var n = n1;
            auto lightPos = float3(5.0f, 5.0f, 1.0f);
            Var li = lightPos - p;
            Var len = length(li);
            li /= len;
            Var dif = clamp(dot(n, li), 0.0f, 1.0f);
            auto [s, g2] = calcShadow(lightPos, -li, len, g1, time);
            Var sha = s;
            col *= max(sha * dif, 0.2f);
            Var rimd = pow(clamp(1.0f - dot(reflect(-li, n), -rd), 0.0f, 1.0f), 2.5f);
            Var frn = rimd + 2.2f * (1.0f - rimd);
            col *= frn * 0.8f;
            col *= max(0.5f + 0.5f * n.y, 0.0f);
            auto [m, g3] = map(p + n * 0.3f, g2, time);
            col *= exp2(-2.f * pow(max(0.0f, 1.0f - m / 0.3f), 2.0f));
            col += float3(0.8f, 0.6f, 0.2f) * pow(clamp(dot(reflect(rd, n), li), 0.0f, 1.0f), 20.0f);
            col = lerp(float3(0.1f, 0.1f, 0.2f), col, exp(-0.001f * t * t));
            col += float3(0.7f, 0.3f, 0.1f) * g3 * (1.5f + 0.8f * sin(time * 3.5f));
            col = clamp(col, 0.0f, 1.0f);
        });
        return pow(col, float3(1.5f));
    };

    gui::ShaderToy::run(argv[0], mainImage, 512u);
}
