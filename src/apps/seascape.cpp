//
// Created by AirGuanZ on 2021/11/13.
// 

#include <dsl/sugar.h>
#include <gui/shader_toy.h>

using namespace luisa;
using namespace compute;

// Credit: https://www.shadertoy.com/view/Ms2SD1
int main(int, char *argv[]) {

    constexpr int   NUM_STEPS = 8;
    constexpr float PI        = 3.14159265f;

    constexpr int ITER_GEOMETRY   = 3;
    constexpr int ITER_FRAGMENT   = 5;

    constexpr float  SEA_HEIGHT      = 0.6f;
    constexpr float  SEA_CHOPPY      = 4.0f;
    constexpr float  SEA_SPEED       = 0.8f;
    constexpr float  SEA_FREQ        = 0.16f;
    constexpr float3 SEA_BASE        = float3(0.0f, 0.09f, 0.18f);
    constexpr float3 SEA_WATER_COLOR = float3(0.8f, 0.9f, 0.6f) * 0.6f;

    Callable fromEuler = [](Float3 ang) {
        Float2 a1 = make_float2(sin(ang.x), cos(ang.x));
        Float2 a2 = make_float2(sin(ang.y), cos(ang.y));
        Float2 a3 = make_float2(sin(ang.z), cos(ang.z));
        Float3x3 m;
        m[0] = make_float3(a1.y * a3.y + a1.x * a2.x * a3.x, a1.y * a2.x * a3.x + a3.y * a1.x, -a2.y * a3.x);
        m[1] = make_float3(-a2.y * a1.x, a1.y * a2.y, a2.x);
        m[2] = make_float3(a3.y * a1.x * a2.x + a1.y * a3.x, a1.x * a3.x - a1.y * a3.y * a2.x, a2.y * a3.y);
        return m;
    };

    Callable hash = [](Float2 p) {
        Float h = dot(p, make_float2(127.1f, 311.7f));
        Float f = fract(sin(h) * 43758.5453123f);
        return f - floor(f);
    };

    Callable noise = [&](Float2 p) {
        Float2 i = floor(p);
        Float2 f = p - i;
        Float2 u = f * f * (3.0f - 2.0f * f);
        return -1.0f + 2.0f * lerp(
            lerp(hash(i + make_float2(0.0f, 0.0f)), hash(i + make_float2(1.0, 0.0)), u.x),
            lerp(hash(i + make_float2(0.0f, 1.0f)), hash(i + make_float2(1.0f, 1.0f)), u.x),
            u.y);
    };

    Callable reflect = [](Float3 I, Float3 N) noexcept {
        return I - 2.0f * dot(N, I) * N;
    };

    Callable diffuse = [](Float3 n, Float3 l, Float p) {
        return pow(dot(n, l) * 0.4f + 0.6f, p);
    };

    Callable specular = [&](Float3 n, Float3 l, Float3 e, Float s) {
        Float nrm = (s + 8.0f) / (PI * 8.0f);
        return pow(max(dot(reflect(e, n), l), 0.0f), s) * nrm;
    };

    Callable getSkyColor = [](Float3 e) {
        e.y = (max(e.y, 0.0f) * 0.8f + 0.2f) * 0.8f;
        return make_float3(pow(1.0f - e.y, 2.0f), 1.0f - e.y, 0.6f + (1.0f - e.y) * 0.4f) * 1.1f;
    };
    
    Callable seaOctave = [&](Float2 uv, Float choppy) {
        uv = uv + noise(uv);
        Float2 wv = 1.0f - abs(sin(uv));
        Float2 swv = abs(cos(uv));
        wv = lerp(wv, swv, wv);
        return pow(1.0f - pow(wv.x * wv.y, 0.65f), choppy);
    };

    Callable map = [&](Float3 p, Float time) {
        Float2x2 octave_m = make_float2x2(1.6f, -1.2f, 1.2f, 1.6f);
        Float freq = SEA_FREQ;
        Float amp = SEA_HEIGHT;
        Float choppy = SEA_CHOPPY;
        Float2 uv = p.xz();
        uv.x *= 0.75f;
        Float h = 0.0f;
        $for(_) : $range(ITER_GEOMETRY) {
            Float d = seaOctave((uv + (1 + time * SEA_SPEED)) * freq, choppy);
            d += seaOctave((uv - (1 + time * SEA_SPEED)) * freq, choppy);
            h += d * amp;
            uv = octave_m * uv;
            freq *= 1.9f;
            amp *= 0.22f;
            choppy = lerp(choppy, 1.0f, 0.2f);
        };
        return p.y - h;
    };

    Callable mapDetailed = [&](Float3 p, Float time) {
        Float2x2 octave_m = make_float2x2(1.6f, -1.2f, 1.2f, 1.6f);
        Float freq = SEA_FREQ;
        Float amp = SEA_HEIGHT;
        Float choppy = SEA_CHOPPY;
        Float2 uv = p.xz();
        uv.x *= 0.75f;
        Float h = 0.0f;
        $for(_) : $range(ITER_FRAGMENT) {
            Float d = seaOctave((uv + (1 + time * SEA_SPEED)) * freq, choppy);
            d += seaOctave((uv - (1 + time * SEA_SPEED)) * freq, choppy);
            h += d * amp;
            uv = octave_m * uv;
            freq *= 1.9f;
            amp *= 0.22f;
            choppy = lerp(choppy, 1.0f, 0.2f);
        };
        return p.y - h;
    };

    Callable getSeaColor = [&](Float3 p, Float3 n, Float3 l, Float3 eye, Float3 dist) {
        Float fresnel = clamp(1.0f - dot(n, -eye), 0.0f, 1.0f);
        fresnel = pow(fresnel, 3.0f) * 0.5f;
        Float3 reflected = getSkyColor(reflect(eye, n));
        Float3 refracted = make_float3(SEA_BASE.x, SEA_BASE.y, SEA_BASE.z)
                           + diffuse(n, l, 80.0f) * 0.12f * make_float3(SEA_WATER_COLOR.x, SEA_WATER_COLOR.y, SEA_WATER_COLOR.z);
        Float3 color = lerp(refracted, reflected, fresnel);
        Float atten = max(1.0f - dot(dist, dist) * 0.001f, 0.0f);
        color += make_float3(SEA_WATER_COLOR.x, SEA_WATER_COLOR.y, SEA_WATER_COLOR.z)
                 * (p.y - SEA_HEIGHT) * 0.18f * atten;
        color += make_float3(0.2f * specular(n, l, eye, 60.0f));
        return color;
    };
    
    Callable getNormal = [&](Float3 p, Float eps, Float time) {
        Float3 n;
        n.y = mapDetailed(p, time);
        n.x = mapDetailed(make_float3(p.x + eps, p.y, p.z), time) - n.y;
        n.z = mapDetailed(make_float3(p.x, p.y, p.z + eps), time) - n.y;
        n.y = eps;
        return normalize(n);
    };

    Callable heightMapTracing = [&](Float3 ori, Float3 dir, Float3 &p, Float time) {
        Float tm = 0.0f;
        Float tx = 1000.0f;
        Float hx = map(ori + dir * tx, time);
        $if(hx > 0.0f) {
            p = ori + dir * tx;
            return tx;
        };
        Float hm = map(ori + dir * tm, time);
        Float tmid = 0.0f;
        $for(_) : $range(NUM_STEPS) {
            tmid = lerp(tm, tx, hm / (hm - hx));
            p = ori + dir * tmid;
            Float hmid = map(p, time);
            $if(hmid < 0.0f) {
                tx = tmid;
                hx = hmid;
            }
            $else {
                tm = tmid;
                hm = hmid;
            };
        };
    };

    Callable getPixel = [&](Float2 coord, Float time, Float2 iResolution) {
        Float2 uv = coord / iResolution.xy();
        uv = uv * 2.0f - 1.0f;
        uv.x *= iResolution.x / iResolution.y;
        // ray
        Float3 ang = make_float3(sin(time * 3.0f) * 0.1f, sin(time) * 0.2f + 0.3f, time);
        Float3 ori = make_float3(0.0f, 3.5f, time * 5.0f);
        Float3 dir = normalize(make_float3(uv.xy(), -2.0f));
        dir.z += length(uv) * 0.14f;
        dir = transpose(fromEuler(ang)) * normalize(dir);
        // tracing
        Float3 p;
        heightMapTracing(ori, dir, p, time);
        Float3 dist = p - ori;
        Float3 n = getNormal(p, dot(dist, dist) * (0.1f / iResolution.x), time);
        Float3 light = normalize(make_float3(0.0f, 1.0f, 0.8f));
        // color
        return lerp(
            getSkyColor(dir),
            getSeaColor(p, n, light, dir, dist),
            pow(smoothstep(0.0f, -0.02f, dir.y), 0.2f));
    };

    Callable mainImage = [&](Float2 fragCoord, Float2 iResolution, Float iTime, Float4 iMouse, Float3) {
        Float time = iTime * 0.3f + iMouse.x * 0.01f;
        Float3 color = getPixel(fragCoord, time, iResolution);
        return pow(color, 0.65f);
    };

    gui::ShaderToy::run(argv[0], mainImage, make_uint2(1280, 720));
}
