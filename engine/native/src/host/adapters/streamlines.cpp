// host/adapters/streamlines.cpp — CPU RK4 field-line integrator for the Scale-0
// STREAMLINE overlays. Faithful port of engine/web/js/fieldlines.js
// (computeStreamlines + the seed generators) and the per-overlay colour ramps.
//
// Pipeline per overlay:
//   1. scatter the stride-1 VisualFieldSample into a dense L³ voxel grid;
//   2. generate seeds (importance ∝|field|^1.5 / particle-anchored / rings);
//   3. RK4-integrate each seed BIDIRECTIONALLY over the unit-normalised field
//      (dir-folded backward pass, exactly as the web);
//   4. emit each traced polyline as consecutive NativeLine segments, coloured
//      per-vertex (flux colormap by local |J| / cyan fade / green fade).

#include "native/host/adapters/streamlines.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace ftd::native::streamlines {
namespace {

// ── Web computeStreamlines constants (fieldlines.js) ──────────────────────────
constexpr float  kStepSize     = 0.5f;    // RK4 step in voxels
constexpr int    kMaxSteps     = 100;     // per-direction step budget (×1.5 for B)
constexpr int    kMaxSeeds     = 150;     // seed cap (buildElectromagneticOverlayData)
constexpr int    kMaxLines     = 200;     // global line cap (computeStreamlines)
constexpr float  kMinMag       = 1e-10f;  // stop when |field| drops below this
constexpr float  kImportanceExp = 1.5f;   // |field|^exponent importance weight
constexpr int    kEOffset      = 2;       // E particle-anchored seed offset (voxels)
constexpr int    kBRadius      = 4;       // B ring / perpendicular-importance radius

// ── Dense stride-1 field grid ─────────────────────────────────────────────────
// data is 3*L³ floats laid out at index (x + y*L + z*L²)*3. Voxels not present in
// the (compacted) sample stay zero → the trace stops there, mirroring the web.
struct DenseField {
    int L = 0;
    std::vector<float> data;  // 3 * L³

    void build(const ftd::VisualFieldSample& s, int lattice) {
        L = lattice;
        data.assign(static_cast<std::size_t>(L) * L * L * 3u, 0.0f);
        if (s.components != 3u) return;
        const std::size_t n = s.count();
        for (std::size_t i = 0; i < n; ++i) {
            const int ix = clampi(static_cast<int>(std::floor(s.positions[i * 3u])));
            const int iy = clampi(static_cast<int>(std::floor(s.positions[i * 3u + 1u])));
            const int iz = clampi(static_cast<int>(std::floor(s.positions[i * 3u + 2u])));
            const std::size_t idx =
                (static_cast<std::size_t>(ix) + static_cast<std::size_t>(iy) * L
                 + static_cast<std::size_t>(iz) * L * L) * 3u;
            data[idx]      = s.data[i * 3u];
            data[idx + 1u] = s.data[i * 3u + 1u];
            data[idx + 2u] = s.data[i * 3u + 2u];
        }
    }

    int clampi(int v) const { return v < 0 ? 0 : (v >= L ? L - 1 : v); }

    // Nearest-voxel field lookup (floor + clamp): the dense-grid analogue of the
    // web lookupFieldInto nearest-sample scan.
    void lookup(float px, float py, float pz, float& fx, float& fy, float& fz) const {
        const int ix = clampi(static_cast<int>(std::floor(px)));
        const int iy = clampi(static_cast<int>(std::floor(py)));
        const int iz = clampi(static_cast<int>(std::floor(pz)));
        const std::size_t idx =
            (static_cast<std::size_t>(ix) + static_cast<std::size_t>(iy) * L
             + static_cast<std::size_t>(iz) * L * L) * 3u;
        fx = data[idx];
        fy = data[idx + 1u];
        fz = data[idx + 2u];
    }
};

// ── RK4 integration over the unit-normalised field ────────────────────────────
// Mirrors integrateGridInto (fieldlines.js): dir ∈ {+1,-1} folds the backward
// pass into the normalised field, the step-local fallback is the dir-signed raw
// field, and the boundary test is a half-open [0,L) box on the new point.
struct Vertex { float x, y, z; };

// Normalised (dir-folded) field at (px,py,pz); falls back to (fbx,fby,fbz).
inline void norm_into(const DenseField& g, float px, float py, float pz, float dir,
                      float fbx, float fby, float fbz,
                      float& nx, float& ny, float& nz) {
    float vx, vy, vz;
    g.lookup(px, py, pz, vx, vy, vz);
    const float m = std::sqrt(vx * vx + vy * vy + vz * vz);
    if (m < kMinMag) { nx = fbx; ny = fby; nz = fbz; return; }
    nx = dir * (vx / m);
    ny = dir * (vy / m);
    nz = dir * (vz / m);
}

// Integrate one streamline from (x0,y0,z0) in direction `dir`, appending
// vertices (including the seed) to `out`. Returns the number of vertices added.
int integrate(const DenseField& g, float x0, float y0, float z0, float h, int max_steps,
              float dir, std::vector<Vertex>& out) {
    const float bound = static_cast<float>(g.L);
    out.push_back({x0, y0, z0});
    int added = 1;
    float x = x0, y = y0, z = z0;

    for (int step = 0; step < max_steps; ++step) {
        float vx, vy, vz;
        g.lookup(x, y, z, vx, vy, vz);
        const float mag = std::sqrt(vx * vx + vy * vy + vz * vz);
        if (mag < kMinMag) break;

        // Step-local fallback = dir-signed normalised raw field at (x,y,z).
        const float fbx = dir * (vx / mag);
        const float fby = dir * (vy / mag);
        const float fbz = dir * (vz / mag);

        float k1x, k1y, k1z, k2x, k2y, k2z, k3x, k3y, k3z, k4x, k4y, k4z;
        norm_into(g, x, y, z, dir, fbx, fby, fbz, k1x, k1y, k1z);
        norm_into(g, x + 0.5f * h * k1x, y + 0.5f * h * k1y, z + 0.5f * h * k1z, dir,
                  fbx, fby, fbz, k2x, k2y, k2z);
        norm_into(g, x + 0.5f * h * k2x, y + 0.5f * h * k2y, z + 0.5f * h * k2z, dir,
                  fbx, fby, fbz, k3x, k3y, k3z);
        norm_into(g, x + h * k3x, y + h * k3y, z + h * k3z, dir,
                  fbx, fby, fbz, k4x, k4y, k4z);

        const float x1 = x + (h / 6.0f) * (k1x + 2.0f * k2x + 2.0f * k3x + k4x);
        const float y1 = y + (h / 6.0f) * (k1y + 2.0f * k2y + 2.0f * k3y + k4y);
        const float z1 = z + (h / 6.0f) * (k1z + 2.0f * k2z + 2.0f * k3z + k4z);

        if (x1 < 0.0f || x1 >= bound || y1 < 0.0f || y1 >= bound || z1 < 0.0f
            || z1 >= bound)
            break;

        x = x1; y = y1; z = z1;
        out.push_back({x, y, z});
        ++added;
    }
    return added;
}

// One traced field line: [reversed backward, forward] (matches the web layout).
using Polyline = std::vector<Vertex>;

// Trace `seeds` bidirectionally, appending each non-trivial line to `lines`.
void trace_all(const DenseField& g, const std::vector<Vertex>& seeds, int max_steps,
               std::vector<Polyline>& lines) {
    std::vector<Vertex> fwd, bwd;
    for (const Vertex& s : seeds) {
        if (static_cast<int>(lines.size()) >= kMaxLines) break;
        fwd.clear();
        bwd.clear();
        const int fwd_n = integrate(g, s.x, s.y, s.z, kStepSize, max_steps, +1.0f, fwd);
        const int bwd_n = integrate(g, s.x, s.y, s.z, kStepSize, max_steps, -1.0f, bwd);
        // Both passes re-emit the seed; a line of only the two seed copies is
        // empty (web MIN_VERTS_FLOATS gate: keep when either pass advanced).
        if (fwd_n <= 1 && bwd_n <= 1) continue;
        Polyline line;
        line.reserve(static_cast<std::size_t>(fwd_n + bwd_n));
        for (int i = bwd_n - 1; i >= 0; --i) line.push_back(bwd[static_cast<std::size_t>(i)]);
        for (int i = 0; i < fwd_n; ++i) line.push_back(fwd[static_cast<std::size_t>(i)]);
        if (line.size() >= 2) lines.push_back(std::move(line));
    }
}

// ── Seed generators (ported from fieldlines.js) ───────────────────────────────

// Deterministic RNG so captures are reproducible (the web uses Math.random();
// only the algorithm — not the exact seed positions — is load-bearing).
struct Rng {
    std::mt19937 gen{0x5eedu};
    std::uniform_real_distribution<float> uni{0.0f, 1.0f};
    float next() { return uni(gen); }
};

// Importance sampling ∝|field|^exponent via a stratified inverse-CDF, excluding
// the 1-voxel boundary shell (sampleByFieldMagnitude). `jitter` spreads seeds
// off the exact voxel centre so co-located seeds don't overlap.
std::vector<Vertex> importance_seeds(const ftd::VisualFieldSample& s, int count,
                                     float jitter, Rng& rng) {
    std::vector<Vertex> seeds;
    const std::size_t n = s.count();
    if (s.components != 3u || n == 0 || count <= 0) return seeds;

    float minC = 1e30f, maxC = -1e30f;
    for (std::size_t i = 0; i < n * 3u; ++i) {
        const float v = s.positions[i];
        minC = std::min(minC, v);
        maxC = std::max(maxC, v);
    }
    const float border_min = minC + 0.5f;
    const float border_max = maxC - 0.5f;

    std::vector<float> weights(n, 0.0f);
    double total = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const float px = s.positions[i * 3u];
        const float py = s.positions[i * 3u + 1u];
        const float pz = s.positions[i * 3u + 2u];
        if (px < border_min || px > border_max || py < border_min || py > border_max
            || pz < border_min || pz > border_max)
            continue;
        const float x = s.data[i * 3u];
        const float y = s.data[i * 3u + 1u];
        const float z = s.data[i * 3u + 2u];
        const float m = std::sqrt(x * x + y * y + z * z);
        const float w = std::pow(m, kImportanceExp);
        weights[i] = w;
        total += w;
    }
    if (total <= 0.0) return seeds;

    const double stratum = total / count;
    double cum = 0.0;
    double target = stratum * static_cast<double>(rng.next());
    for (std::size_t i = 0; i < n && static_cast<int>(seeds.size()) < count; ++i) {
        cum += weights[i];
        while (cum >= target && static_cast<int>(seeds.size()) < count) {
            const float jx = (rng.next() - 0.5f) * jitter;
            const float jy = (rng.next() - 0.5f) * jitter;
            const float jz = (rng.next() - 0.5f) * jitter;
            seeds.push_back({s.positions[i * 3u] + jx, s.positions[i * 3u + 1u] + jy,
                             s.positions[i * 3u + 2u] + jz});
            target += stratum;
        }
    }
    return seeds;
}

// E-field seeds: 6 per particle (±x,±y,±z at `offset`), capped at maxSeeds.
std::vector<Vertex> e_field_seeds(const std::vector<std::array<float, 3>>& particles,
                                  int offset, int max_seeds) {
    std::vector<Vertex> seeds;
    static const int dirs[6][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
                                   {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    for (const auto& p : particles) {
        if (static_cast<int>(seeds.size()) >= max_seeds) break;
        for (const auto& d : dirs) {
            if (static_cast<int>(seeds.size()) >= max_seeds) break;
            seeds.push_back({p[0] + d[0] * offset, p[1] + d[1] * offset,
                             p[2] + d[2] * offset});
        }
    }
    return seeds;
}

// B-field ring seeds: 8 per particle on a radius-`radius` ring perpendicular to
// the flux direction. Faithful to generateBFieldSeeds + fillFieldParticleBuf:
// the web's per-particle buffer carries NO flux components, so the flux
// direction defaults to +z (p.fz || 1) and the ring lies in the xy-plane. The
// Gram-Schmidt basis is built exactly as the web builds it.
std::vector<Vertex> b_field_seeds(const std::vector<std::array<float, 3>>& particles,
                                  int radius, int max_seeds) {
    std::vector<Vertex> seeds;
    constexpr int n_ring = 8;
    for (const auto& p : particles) {
        if (static_cast<int>(seeds.size()) >= max_seeds) break;
        // Flux direction defaults to +z (fieldParticleBuf carries no flux).
        float fx = 0.0f, fy = 0.0f, fz = 1.0f;
        // Perpendicular vectors (Gram-Schmidt): a = world-X (or world-Y if f≈X).
        float ax, ay, az;
        if (std::abs(fx) < 0.9f) { ax = 1.0f; ay = 0.0f; az = 0.0f; }
        else { ax = 0.0f; ay = 1.0f; az = 0.0f; }
        const float dot = ax * fx + ay * fy + az * fz;
        float ux = ax - dot * fx, uy = ay - dot * fy, uz = az - dot * fz;
        const float umag = std::sqrt(ux * ux + uy * uy + uz * uz);
        ux /= umag; uy /= umag; uz /= umag;
        // v = f × u
        const float vx = fy * uz - fz * uy;
        const float vy = fz * ux - fx * uz;
        const float vz = fx * uy - fy * ux;
        for (int k = 0; k < n_ring && static_cast<int>(seeds.size()) < max_seeds; ++k) {
            const float theta = (2.0f * 3.14159265358979323846f * k) / n_ring;
            const float c = std::cos(theta), sn = std::sin(theta);
            seeds.push_back({p[0] + radius * (c * ux + sn * vx),
                             p[1] + radius * (c * uy + sn * vy),
                             p[2] + radius * (c * uz + sn * vz)});
        }
    }
    return seeds;
}

// B-field importance seeds (no particles): importance-sample by |B|, then offset
// each seed perpendicular to the LOCAL B direction so it lands on the loop
// circumference (generateBImportanceSeeds).
std::vector<Vertex> b_importance_seeds(const ftd::VisualFieldSample& s, int count,
                                       int offset, Rng& rng) {
    std::vector<Vertex> base = importance_seeds(s, count, 0.0f, rng);
    std::vector<Vertex> out;
    if (base.empty()) return out;
    const std::size_t n = s.count();
    for (const Vertex& seed : base) {
        // Nearest sample to read the local field direction (linear scan; count and
        // nSamples are both small/bounded, as the web notes).
        float best = 1e30f, bx = 0.0f, by = 0.0f, bz = 0.0f;
        for (std::size_t i = 0; i < n; ++i) {
            const float dx = s.positions[i * 3u] - seed.x;
            const float dy = s.positions[i * 3u + 1u] - seed.y;
            const float dz = s.positions[i * 3u + 2u] - seed.z;
            const float d = dx * dx + dy * dy + dz * dz;
            if (d < best) {
                best = d;
                bx = s.data[i * 3u];
                by = s.data[i * 3u + 1u];
                bz = s.data[i * 3u + 2u];
            }
        }
        const float m = std::sqrt(bx * bx + by * by + bz * bz);
        if (m < 1e-10f) { out.push_back(seed); continue; }
        const float fx = bx / m, fy = by / m, fz = bz / m;
        float ax, ay, az;
        if (std::abs(fx) < 0.9f) { ax = 1.0f; ay = 0.0f; az = 0.0f; }
        else { ax = 0.0f; ay = 1.0f; az = 0.0f; }
        const float dot = ax * fx + ay * fy + az * fz;
        float ux = ax - dot * fx, uy = ay - dot * fy, uz = az - dot * fz;
        float umag = std::sqrt(ux * ux + uy * uy + uz * uz);
        if (umag < 1e-20f) umag = 1.0f;
        ux /= umag; uy /= umag; uz /= umag;
        const float theta = rng.next() * 2.0f * 3.14159265358979323846f;
        const float c = std::cos(theta), sn = std::sin(theta);
        const float vx = fy * uz - fz * uy;
        const float vy = fz * ux - fx * uz;
        const float vz = fx * uy - fy * ux;
        out.push_back({seed.x + offset * (c * ux + sn * vx),
                       seed.y + offset * (c * uy + sn * vy),
                       seed.z + offset * (c * uz + sn * vz)});
    }
    return out;
}

// ── Colour ramps (ported from fields.js / field-em-renderer.js) ───────────────

// Flux colormap by local |J| (fluxToColor): dark-blue → blue → cyan → white →
// yellow → red across t = mag/maxFlux ∈ [0,1].
void flux_color(float mag, float max_flux, float& r, float& g, float& b) {
    if (max_flux < 1e-20f) { r = 0.02f; g = 0.03f; b = 0.08f; return; }
    float t = mag / max_flux;
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    if (t < 0.25f) {
        const float s = t / 0.25f;
        r = 0.02f + 0.03f * s; g = 0.03f + 0.12f * s; b = 0.08f + 0.52f * s;
    } else if (t < 0.5f) {
        const float s = (t - 0.25f) / 0.25f;
        r = 0.05f + 0.05f * s; g = 0.15f + 0.65f * s; b = 0.60f + 0.30f * s;
    } else if (t < 0.75f) {
        const float s = (t - 0.5f) / 0.25f;
        r = 0.10f + 0.85f * s; g = 0.80f + 0.15f * s; b = 0.90f - 0.30f * s;
    } else {
        const float s = (t - 0.75f) / 0.25f;
        r = 0.95f + 0.05f * s; g = 0.95f - 0.65f * s; b = 0.60f - 0.55f * s;
    }
}

// Per-vertex colour for one polyline vertex `i` of `n_pts`, by overlay.
//   Flux — flux colormap by local |J| (needs the dense field + max_flux);
//   E    — cyan fade  (0.30,0.82,0.88)·α, α = 1 − 0.7·(i/(n−1));
//   B    — green fade (0.40,0.73,0.42)·α, α = 1 − 0.5·(i/(n−1)).
void vertex_color(Overlay overlay, const DenseField& g, const Vertex& v, int i, int n_pts,
                  float max_flux, float& r, float& g_out, float& b) {
    const float denom = n_pts > 1 ? static_cast<float>(n_pts - 1) : 1.0f;
    if (overlay == Overlay::Flux) {
        float fx, fy, fz;
        g.lookup(v.x, v.y, v.z, fx, fy, fz);
        const float mag = std::sqrt(fx * fx + fy * fy + fz * fz);
        flux_color(mag, max_flux, r, g_out, b);
    } else if (overlay == Overlay::Electric) {
        const float alpha = 1.0f - (static_cast<float>(i) / denom) * 0.7f;
        r = 0.30f * alpha; g_out = 0.82f * alpha; b = 0.88f * alpha;
    } else {  // Magnetic
        const float alpha = 1.0f - (static_cast<float>(i) / denom) * 0.5f;
        r = 0.40f * alpha; g_out = 0.73f * alpha; b = 0.42f * alpha;
    }
}

}  // namespace

void append(const ftd::VisualFieldSample& field,
            const std::vector<std::array<float, 3>>& particles, int L, Overlay overlay,
            std::vector<NativeLine>& out_lines) {
    if (field.components != 3u || field.count() == 0 || L <= 0) return;

    // Global max |field| — the flux colormap normaliser (fluxToColor's maxFlux).
    float max_flux = 0.0f;
    for (std::size_t i = 0; i < field.count(); ++i) {
        const float x = field.data[i * 3u];
        const float y = field.data[i * 3u + 1u];
        const float z = field.data[i * 3u + 2u];
        max_flux = std::max(max_flux, std::sqrt(x * x + y * y + z * z));
    }

    DenseField grid;
    grid.build(field, L);

    Rng rng;
    std::vector<Vertex> seeds;
    int max_steps = kMaxSteps;
    switch (overlay) {
        case Overlay::Flux:
            seeds = importance_seeds(field, kMaxSeeds, 0.5f, rng);
            break;
        case Overlay::Electric:
            seeds = !particles.empty()
                        ? e_field_seeds(particles, kEOffset, kMaxSeeds)
                        : importance_seeds(field, kMaxSeeds, 0.5f, rng);
            break;
        case Overlay::Magnetic:
            seeds = !particles.empty()
                        ? b_field_seeds(particles, kBRadius, kMaxSeeds)
                        : b_importance_seeds(field, kMaxSeeds, kBRadius, rng);
            // Loops need ~2·π·radius steps to close — give B 1.5× the budget.
            max_steps = static_cast<int>(std::ceil(kMaxSteps * 1.5f));
            break;
    }
    if (seeds.empty()) return;

    std::vector<Polyline> lines;
    trace_all(grid, seeds, max_steps, lines);
    if (lines.empty()) return;

    std::size_t seg_total = 0;
    for (const Polyline& line : lines) seg_total += line.size() - 1;
    out_lines.reserve(out_lines.size() + seg_total);

    for (const Polyline& line : lines) {
        const int n_pts = static_cast<int>(line.size());
        for (int j = 0; j + 1 < n_pts; ++j) {
            float r0, g0, b0, r1, g1, b1;
            vertex_color(overlay, grid, line[static_cast<std::size_t>(j)], j, n_pts,
                         max_flux, r0, g0, b0);
            vertex_color(overlay, grid, line[static_cast<std::size_t>(j + 1)], j + 1, n_pts,
                         max_flux, r1, g1, b1);
            NativeLine seg;
            seg.x0 = line[static_cast<std::size_t>(j)].x;
            seg.y0 = line[static_cast<std::size_t>(j)].y;
            seg.z0 = line[static_cast<std::size_t>(j)].z;
            seg.r0 = r0; seg.g0 = g0; seg.b0 = b0;
            seg.x1 = line[static_cast<std::size_t>(j + 1)].x;
            seg.y1 = line[static_cast<std::size_t>(j + 1)].y;
            seg.z1 = line[static_cast<std::size_t>(j + 1)].z;
            seg.r1 = r1; seg.g1 = g1; seg.b1 = b1;
            out_lines.push_back(seg);
        }
    }
}

// ── Force-Flow: dashed, animated streamlines (updateForceStreamlines port) ─────
void append_force_flow(const ftd::VisualFieldSample& field, int L, float phase,
                       const float low[3], const float mid[3], const float high[3],
                       std::vector<NativeLine>& out_lines) {
    if (field.components != 3u || field.count() == 0 || L <= 0) return;

    float max_flux = 0.0f;
    for (std::size_t i = 0; i < field.count(); ++i) {
        const float x = field.data[i * 3u];
        const float y = field.data[i * 3u + 1u];
        const float z = field.data[i * 3u + 2u];
        max_flux = std::max(max_flux, std::sqrt(x * x + y * y + z * z));
    }
    if (max_flux < 1e-20f) return;

    DenseField grid;
    grid.build(field, L);

    Rng rng;
    std::vector<Vertex> seeds = importance_seeds(field, kMaxSeeds, 0.5f, rng);
    if (seeds.empty()) return;

    // 40% of the field-line step budget (the web traces force flow shorter).
    const int max_steps = std::max(8, static_cast<int>(kMaxSteps * 0.4f));
    std::vector<Polyline> lines;
    trace_all(grid, seeds, max_steps, lines);
    if (lines.empty()) return;

    // 3-stop palette colour of local |field| (port of lerpPalette).
    auto palette = [&](float t, float& r, float& g, float& b) {
        t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        if (t < 0.5f) {
            const float u = t * 2.0f;
            r = low[0] + (mid[0] - low[0]) * u;
            g = low[1] + (mid[1] - low[1]) * u;
            b = low[2] + (mid[2] - low[2]) * u;
        } else {
            const float u = (t - 0.5f) * 2.0f;
            r = mid[0] + (high[0] - mid[0]) * u;
            g = mid[1] + (high[1] - mid[1]) * u;
            b = mid[2] + (high[2] - mid[2]) * u;
        }
    };

    // Dash pattern (voxels): the LINELIST has no dash shader, so the dashes are
    // emitted CPU-side — a segment is drawn only when its arc-length midpoint
    // falls in the "on" part of the (dash+gap) period, offset by `phase` so the
    // pattern marches along the line as `phase` advances with the sim tick.
    constexpr float kDash = 1.5f, kGap = 0.8f;
    const float period = kDash + kGap;

    for (const Polyline& line : lines) {
        const int n_pts = static_cast<int>(line.size());
        float arclen = 0.0f;
        for (int j = 0; j + 1 < n_pts; ++j) {
            const Vertex& a = line[static_cast<std::size_t>(j)];
            const Vertex& b = line[static_cast<std::size_t>(j + 1)];
            const float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
            const float seg_len = std::sqrt(dx * dx + dy * dy + dz * dz);
            const float mid_s = arclen + 0.5f * seg_len;
            arclen += seg_len;
            float ph = std::fmod(mid_s + phase, period);
            if (ph < 0.0f) ph += period;
            if (ph >= kDash) continue;  // in the gap → this dash is "off"

            float fx0, fy0, fz0, fx1, fy1, fz1;
            grid.lookup(a.x, a.y, a.z, fx0, fy0, fz0);
            grid.lookup(b.x, b.y, b.z, fx1, fy1, fz1);
            const float t0 = std::sqrt(fx0 * fx0 + fy0 * fy0 + fz0 * fz0) / max_flux;
            const float t1 = std::sqrt(fx1 * fx1 + fy1 * fy1 + fz1 * fz1) / max_flux;
            NativeLine seg;
            seg.x0 = a.x; seg.y0 = a.y; seg.z0 = a.z;
            seg.x1 = b.x; seg.y1 = b.y; seg.z1 = b.z;
            palette(t0, seg.r0, seg.g0, seg.b0);
            palette(t1, seg.r1, seg.g1, seg.b1);
            out_lines.push_back(seg);
        }
    }
}

// ── Knot Zones: cluster field-line segments into wireframe boxes ───────────────
std::vector<KnotBox> detect_knots(const std::vector<NativeLine>& segments, int L,
                                  float sensitivity, int max_knots) {
    std::vector<KnotBox> out;
    if (segments.empty() || L <= 0) return out;

    // Coarse cell grid over the box. cs=2 like the web, but grown so G stays
    // bounded (≤ 64) for large lattices — G³ arrays are allocated below.
    const int cs = std::max(2, (L + 63) / 64);
    const int G = std::max(1, (L + cs - 1) / cs);
    const std::size_t total = static_cast<std::size_t>(G) * G * G;
    std::vector<float> density(total, 0.0f);

    auto cell_of = [&](float x, float y, float z) -> std::size_t {
        int cx = static_cast<int>(x) / cs, cy = static_cast<int>(y) / cs,
            cz = static_cast<int>(z) / cs;
        cx = cx < 0 ? 0 : (cx >= G ? G - 1 : cx);
        cy = cy < 0 ? 0 : (cy >= G ? G - 1 : cy);
        cz = cz < 0 ? 0 : (cz >= G ? G - 1 : cz);
        return (static_cast<std::size_t>(cz) * G + cy) * G + cx;
    };

    // Pass 1: segment midpoints → density.
    for (const NativeLine& s : segments) {
        const float mx = (s.x0 + s.x1) * 0.5f;
        const float my = (s.y0 + s.y1) * 0.5f;
        const float mz = (s.z0 + s.z1) * 0.5f;
        density[cell_of(mx, my, mz)] += 1.0f;
    }

    // Adaptive density threshold (sensitivity-scaled), exactly the web formula.
    double sum = 0.0;
    std::size_t nz = 0;
    for (std::size_t c = 0; c < total; ++c)
        if (density[c] > 0.0f) { sum += density[c]; ++nz; }
    if (nz == 0) return out;
    const float mean = static_cast<float>(sum / static_cast<double>(nz));
    const float sens = sensitivity < 0.0f ? 0.0f : (sensitivity > 1.0f ? 1.0f : sensitivity);
    const float mult = 3.0f - 2.6f * sens;
    const float d_thr = std::max(3.0f, std::ceil(mult * mean));

    // Hot cells (density ≥ threshold). The web's crossing gate is OFF by default.
    std::vector<std::uint8_t> hot(total, 0);
    for (std::size_t c = 0; c < total; ++c)
        if (density[c] >= d_thr) hot[c] = 1;

    // 26-neighbour flood fill over hot cells → connected components.
    std::vector<std::uint8_t> seen(total, 0);
    std::vector<int> stack;
    stack.reserve(total);
    struct Comp { std::vector<std::size_t> cells; };
    std::vector<Comp> comps;
    for (std::size_t c0 = 0; c0 < total; ++c0) {
        if (!hot[c0] || seen[c0]) continue;
        Comp comp;
        stack.clear();
        stack.push_back(static_cast<int>(c0));
        seen[c0] = 1;
        while (!stack.empty()) {
            const int cur = stack.back();
            stack.pop_back();
            comp.cells.push_back(static_cast<std::size_t>(cur));
            const int gx = cur % G, gy = (cur / G) % G, gz = cur / (G * G);
            for (int dz = -1; dz <= 1; ++dz) {
                const int nz2 = gz + dz;
                if (nz2 < 0 || nz2 >= G) continue;
                for (int dy = -1; dy <= 1; ++dy) {
                    const int ny = gy + dy;
                    if (ny < 0 || ny >= G) continue;
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0 && dz == 0) continue;
                        const int nx = gx + dx;
                        if (nx < 0 || nx >= G) continue;
                        const std::size_t ni =
                            (static_cast<std::size_t>(nz2) * G + ny) * G + nx;
                        if (hot[ni] && !seen[ni]) {
                            seen[ni] = 1;
                            stack.push_back(static_cast<int>(ni));
                        }
                    }
                }
            }
        }
        if (comp.cells.size() >= 2) comps.push_back(std::move(comp));  // minCellsPerKnot
    }
    if (comps.empty()) return out;

    // Largest knots first, capped.
    std::sort(comps.begin(), comps.end(),
              [](const Comp& a, const Comp& b) { return a.cells.size() > b.cells.size(); });
    const int K = std::min(static_cast<int>(comps.size()), std::max(0, max_knots));
    const float half = cs * 0.5f;
    out.reserve(static_cast<std::size_t>(K));
    for (int k = 0; k < K; ++k) {
        const Comp& comp = comps[static_cast<std::size_t>(k)];
        float wsum = 0.0f, cxA = 0.0f, cyA = 0.0f, czA = 0.0f;
        float minx = 1e30f, miny = 1e30f, minz = 1e30f;
        float maxx = -1e30f, maxy = -1e30f, maxz = -1e30f;
        for (std::size_t ci : comp.cells) {
            const int gx = static_cast<int>(ci % G);
            const int gy = static_cast<int>((ci / G) % G);
            const int gz = static_cast<int>(ci / (static_cast<std::size_t>(G) * G));
            const float wcx = (gx + 0.5f) * cs, wcy = (gy + 0.5f) * cs, wcz = (gz + 0.5f) * cs;
            const float w = density[ci];
            wsum += w; cxA += wcx * w; cyA += wcy * w; czA += wcz * w;
            minx = std::min(minx, wcx); maxx = std::max(maxx, wcx);
            miny = std::min(miny, wcy); maxy = std::max(maxy, wcy);
            minz = std::min(minz, wcz); maxz = std::max(maxz, wcz);
        }
        KnotBox box;
        box.cx = wsum > 0.0f ? cxA / wsum : 0.0f;
        box.cy = wsum > 0.0f ? cyA / wsum : 0.0f;
        box.cz = wsum > 0.0f ? czA / wsum : 0.0f;
        box.hx = std::max(1.0f, (maxx - minx) * 0.5f + half);
        box.hy = std::max(1.0f, (maxy - miny) * 0.5f + half);
        box.hz = std::max(1.0f, (maxz - minz) * 0.5f + half);
        box.index = k;
        out.push_back(box);
    }
    return out;
}

}  // namespace ftd::native::streamlines
