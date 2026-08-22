// Environment backgrounds — see native/backgrounds.h.
// Port of engine/web/js/backgrounds/*.js. Point clouds surrounding the lattice,
// deterministic per index (PCG-hash PRNG) + animated by wall-clock time.

#include "native/backgrounds.h"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace ftd::native {
namespace {

constexpr double kPi = 3.14159265358979323846;

// Deterministic per-point PRNG (PCG-hash). Seeded from the point index + a theme
// stream so the cloud is stable across frames (only `time` animates it).
struct Rng {
    std::uint32_t s;
    explicit Rng(std::uint32_t seed) : s(seed * 747796405u + 2891336453u) {}
    float f() {
        s = s * 747796405u + 2891336453u;
        std::uint32_t x = ((s >> ((s >> 28) + 4)) ^ s) * 277803737u;
        x = (x >> 22) ^ x;
        return static_cast<float>(x >> 8) * (1.0f / 16777216.0f);
    }
    float range(float a, float b) { return a + (b - a) * f(); }
};

struct V3 { double x, y, z; };

// Uniform point on a shell of `radius` with slight radial jitter (matches the
// web randSphere()).
V3 rand_sphere(Rng& r, double radius) {
    const double u = r.f(), v = r.f();
    const double theta = 2.0 * kPi * u, phi = std::acos(2.0 * v - 1.0);
    const double rr = radius * (0.85 + 0.15 * r.f());
    return {rr * std::sin(phi) * std::cos(theta), rr * std::sin(phi) * std::sin(theta),
            rr * std::cos(phi)};
}

struct Rgb { float r, g, b; };
Rgb hsl(double h, double s, double l) {
    auto hue = [](double p, double q, double t) {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.0 / 6) return p + (q - p) * 6 * t;
        if (t < 1.0 / 2) return q;
        if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
        return p;
    };
    if (s == 0) return {float(l), float(l), float(l)};
    const double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    const double p = 2 * l - q;
    return {float(hue(p, q, h + 1.0 / 3)), float(hue(p, q, h)), float(hue(p, q, h - 1.0 / 3))};
}

// Push one background point at lattice-centre + offset, colour scaled by `bright`.
struct Emitter {
    std::vector<NativeParticle>& out;
    double c;   // lattice centre (N/2)
    void pt(const V3& off, const Rgb& col, float size, float bright) {
        NativeParticle p;
        p.x = float(c + off.x); p.y = float(c + off.y); p.z = float(c + off.z);
        p.r = col.r * bright; p.g = col.g * bright; p.b = col.b * bright;
        p.size = size;
        out.push_back(p);
    }
};

// Twinkle/flicker factor in [lo,1] from a per-point phase + time.
inline float twinkle(float phase, double t, double speed, float lo) {
    const float s = 0.5f + 0.5f * std::sin(float(t * speed) + phase);
    return lo + (1.0f - lo) * s;
}

void build_stars(Emitter& e, double br, double sz, double t, int count, std::uint32_t stream) {
    for (int i = 0; i < count; ++i) {
        Rng r(static_cast<std::uint32_t>(i) * 9781u + stream);
        const V3 off = rand_sphere(r, br);
        const float temp = r.f();
        Rgb col;
        if (temp < 0.6f)      col = hsl(0.0, 0.0, 0.7 + 0.3 * r.f());   // white
        else if (temp < 0.8f) col = hsl(0.6, 0.4, 0.6 + 0.3 * r.f());   // blue-white
        else if (temp < 0.9f) col = hsl(0.08, 0.5, 0.6 + 0.3 * r.f());  // warm
        else                  col = hsl(0.55, 0.6, 0.5 + 0.4 * r.f());  // cyan
        const float size = float(sz * (0.5 + 1.2 * r.f()));
        e.pt(off, col, size, twinkle(r.f() * 6.283f, t, 1.6, 0.45f));
    }
}

}  // namespace

void build_background(int theme, double time_sec, int lattice_size,
                      std::vector<NativeParticle>& points, std::vector<NativeLine>& lines) {
    points.clear();
    lines.clear();
    const double N = std::max(1, lattice_size);
    const double c = N * 0.5;
    const double br = N * 10.0;        // background radius (camera at ~N*1.8 sits inside)
    const double sz = N * 0.05;        // base sprite size (world units)
    Emitter e{points, c};
    const double t = time_sec;

    switch (static_cast<BackgroundTheme>(theme)) {
        case BackgroundTheme::Stars:
            build_stars(e, br, sz, t, 2500, 1u);
            break;

        case BackgroundTheme::Foam: {
            // Dense flickering micro-points (vacuum foam), muted blue-violet.
            for (int i = 0; i < 7000; ++i) {
                Rng r(static_cast<std::uint32_t>(i) * 6151u + 4u);
                const V3 off = rand_sphere(r, br * (0.5 + 0.5 * r.f()));
                const Rgb col = hsl(0.6 + 0.2 * r.f(), 0.3 + 0.3 * r.f(), 0.15 + 0.15 * r.f());
                e.pt(off, col, float(sz * (0.3 + 0.7 * r.f())),
                     twinkle(r.f() * 6.283f, t, 3.5, 0.25f));
            }
            break;
        }

        case BackgroundTheme::Nebula: {
            // A composed starfield backdrop + a few drifting gaussian gas clouds.
            build_stars(e, br, sz, t, 1500, 2u);
            const double drift = t * 0.03;
            const double palettes[5] = {0.62, 0.78, 0.95, 0.15, 0.50};   // cloud hues
            for (int cl = 0; cl < 5; ++cl) {
                Rng cr(static_cast<std::uint32_t>(cl) * 2749u + 20u);
                const double cx = cr.range(-0.55f, 0.55f) * br,
                             cy = cr.range(-0.4f, 0.4f) * br,
                             cz = cr.range(-0.55f, 0.55f) * br;
                const double hue = palettes[cl];
                for (int i = 0; i < 900; ++i) {
                    Rng r(static_cast<std::uint32_t>(cl * 1000 + i) * 3299u + 30u);
                    // gaussian blob (Box-Muller) around the cloud centre
                    const double a = std::sqrt(-2.0 * std::log(std::max(1e-6f, r.f())));
                    const double g1 = a * std::cos(2 * kPi * r.f());
                    const double g2 = a * std::sin(2 * kPi * r.f());
                    const double g3 = a * std::cos(2 * kPi * r.f());
                    const double spread = br * 0.18;
                    // slow drift = rotate the blob offset about Y
                    double ox = g1 * spread, oz = g3 * spread;
                    const double cs = std::cos(drift), sn = std::sin(drift);
                    const double rx = ox * cs - oz * sn, rz = ox * sn + oz * cs;
                    const Rgb col = hsl(hue + 0.05 * r.f(), 0.6, 0.18 + 0.1 * r.f());
                    e.pt({cx + rx, cy + g2 * spread, cz + rz}, col,
                         float(sz * (1.5 + 2.0 * r.f())), 0.7f);
                }
            }
            break;
        }

        case BackgroundTheme::Storm: {
            // Swirling tilted bands of coloured particles + a starfield backdrop.
            build_stars(e, br, sz, t, 1200, 5u);
            const double bandHue[4] = {0.55, 0.80, 0.10, 0.45};
            const double bandSat[4] = {0.7, 0.6, 0.7, 0.5};
            const double swirl = t * 0.15;
            for (int b = 0; b < 4; ++b) {
                const double tilt = (double(b) / 4.0) * kPi;
                Rng br0(static_cast<std::uint32_t>(b) * 5387u + 40u);
                const double radius = br * (0.55 + 0.3 * br0.f());
                for (int i = 0; i < 2000; ++i) {
                    Rng r(static_cast<std::uint32_t>(b * 4000 + i) * 1733u + 50u);
                    const double angle = (double(i) / 2000.0) * 2 * kPi + b * 0.5 + swirl;
                    const double rr = radius + (r.f() - 0.5) * br * 0.16;
                    const double spread = (r.f() - 0.5) * br * 0.10;
                    double x = rr * std::cos(angle), y = spread, z = rr * std::sin(angle);
                    const double cs = std::cos(tilt), sn = std::sin(tilt);
                    const double ny = y * cs - z * sn, nz = y * sn + z * cs;
                    const Rgb col = hsl(bandHue[b] + (r.f() - 0.5) * 0.08, bandSat[b],
                                        0.25 + 0.1 * r.f());
                    e.pt({x, ny, nz}, col, float(sz * (0.6 + 2.0 * r.f())), 0.85f);
                }
            }
            break;
        }

        case BackgroundTheme::Beyond: {
            // Fading 3D grid ("a lattice with no defined boundary") + sparse
            // flickering void points. Grid extent/step scale with the lattice.
            const double GE = N * 5.0, GS = N * 0.7;
            const double fadeStart = N * 1.0, fadeEnd = GE;
            constexpr int steps = 4;
            auto alpha = [&](double x, double y, double z) {
                const double d = std::sqrt(x * x + y * y + z * z);
                return std::max(0.0, 1.0 - (d - fadeStart) / (fadeEnd - fadeStart));
            };
            for (int axis = 0; axis < 3; ++axis) {
                const int da = (axis + 1) % 3, db = (axis + 2) % 3;
                for (double a = -GE; a <= GE + 1e-6; a += GS)
                for (double b = -GE; b <= GE + 1e-6; b += GS) {
                    for (int s = 0; s < steps; ++s) {
                        const double t0 = -GE + (2 * GE * s / steps);
                        const double t1 = -GE + (2 * GE * (s + 1) / steps);
                        double p0[3] = {0, 0, 0}, p1[3] = {0, 0, 0};
                        p0[da] = a; p0[db] = b; p0[axis] = t0;
                        p1[da] = a; p1[db] = b; p1[axis] = t1;
                        const double a0 = alpha(p0[0], p0[1], p0[2]);
                        const double a1 = alpha(p1[0], p1[1], p1[2]);
                        if (a0 < 0.02 && a1 < 0.02) continue;
                        NativeLine l;
                        l.x0 = float(c + p0[0]); l.y0 = float(c + p0[1]); l.z0 = float(c + p0[2]);
                        l.x1 = float(c + p1[0]); l.y1 = float(c + p1[1]); l.z1 = float(c + p1[2]);
                        l.r0 = float(0.15 * a0); l.g0 = float(0.30 * a0); l.b0 = float(0.50 * a0);
                        l.r1 = float(0.15 * a1); l.g1 = float(0.30 * a1); l.b1 = float(0.50 * a1);
                        lines.push_back(l);
                    }
                }
            }
            for (int i = 0; i < 1200; ++i) {       // flickering void points
                Rng r(static_cast<std::uint32_t>(i) * 7919u + 60u);
                const V3 off = rand_sphere(r, br * 0.6);
                const Rgb col = hsl(0.58, 0.3, 0.1 + 0.1 * r.f());
                e.pt(off, col, float(sz * (0.3 + 0.6 * r.f())),
                     twinkle(r.f() * 6.283f, t, 4.0, 0.3f));
            }
            break;
        }

        case BackgroundTheme::None:
        default:
            break;
    }
}

namespace {
constexpr const char* kNames[] = {"none", "stars", "nebula", "foam", "beyond", "storm"};
}

const char* background_theme_name(int theme) {
    if (theme < 0 || theme >= static_cast<int>(BackgroundTheme::Count)) return "";
    return kNames[theme];
}

int background_theme_from_name(const char* name) {
    if (!name) return -1;
    for (int i = 0; i < static_cast<int>(BackgroundTheme::Count); ++i)
        if (std::strcmp(name, kNames[i]) == 0) return i;
    return -1;
}

}  // namespace ftd::native
