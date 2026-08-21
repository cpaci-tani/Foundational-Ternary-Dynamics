// app/app_pick.cpp — camera framing + ray picking (see app/app_pick.h).

#include "app/app_pick.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd::native::app {
void apply_camera_for_lattice(ftd::native::Camera& cam, int lattice) {
    const float c = static_cast<float>(lattice) * 0.5f;
    cam.target_x = cam.target_y = cam.target_z = c;
    cam.distance = static_cast<float>(lattice) * 1.8f;
}


// Build a world-space ray from a click at (client_x, client_y) inside `rect`,
// using the SAME orbit-camera math D3D12Presenter::render() uses (look_at + DX
// perspective, row-vector convention). No 4×4 inverse: the eye + view basis are
// reconstructed directly and the view-space ray direction (nx·tan·aspect,
// ny·tan, 1) is rotated into world by that basis. Ray = origin + t·dir, t ≥ 0.
PickRay make_pick_ray(const ftd::native::Camera& cam, const ftd::native::SceneRect& rect,
                      int client_x, int client_y) {
    const float w = rect.width > 0 ? static_cast<float>(rect.width) : 1.0f;
    const float h = rect.height > 0 ? static_cast<float>(rect.height) : 1.0f;
    const float ndc_x = 2.0f * (static_cast<float>(client_x - rect.x)) / w - 1.0f;
    const float ndc_y = 1.0f - 2.0f * (static_cast<float>(client_y - rect.y)) / h;
    const float aspect = w / h;
    const float tan_half = std::tan(cam.fov_y * 0.5f);

    // eye + forward — identical to the presenter's eye_{x,y,z} + look_at forward.
    const float cp = std::cos(cam.pitch);
    const float ex = cam.target_x + cam.distance * cp * std::sin(cam.yaw);
    const float ey = cam.target_y + cam.distance * std::sin(cam.pitch);
    const float ez = cam.target_z + cam.distance * cp * std::cos(cam.yaw);
    float fx = cam.target_x - ex, fy = cam.target_y - ey, fz = cam.target_z - ez;
    float fl = std::sqrt(fx * fx + fy * fy + fz * fz);
    if (fl < 1e-6f) fl = 1.0f;
    fx /= fl; fy /= fl; fz /= fl;
    // right = normalize(cross(forward, up)) with up = (0,1,0) → (-fz, 0, fx).
    float sx = -fz, sy = 0.0f, sz = fx;
    float sl = std::sqrt(sx * sx + sy * sy + sz * sz);
    if (sl < 1e-6f) sl = 1.0f;
    sx /= sl; sy /= sl; sz /= sl;
    // up2 = cross(right, forward).
    const float ux = sy * fz - sz * fy;
    const float uy = sz * fx - sx * fz;
    const float uz = sx * fy - sy * fx;

    const float vx = ndc_x * tan_half * aspect;
    const float vy = ndc_y * tan_half;
    float dx = vx * sx + vy * ux + fx;
    float dy = vx * sy + vy * uy + fy;
    float dz = vx * sz + vy * uz + fz;
    float dl = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dl < 1e-6f) dl = 1.0f;
    return PickRay{ex, ey, ez, dx / dl, dy / dl, dz / dl};
}

// Perpendicular distance from world point P to the ray; t_out = distance along
// the (unit) direction (in front of the camera when > 0).
float ray_perp(const PickRay& r, float px, float py, float pz, float& t_out) {
    const float wx = px - r.ox, wy = py - r.oy, wz = pz - r.oz;
    const float t = wx * r.dx + wy * r.dy + wz * r.dz;
    t_out = t;
    const float cx = wx - t * r.dx, cy = wy - t * r.dy, cz = wz - t * r.dz;
    return std::sqrt(cx * cx + cy * cy + cz * cz);
}

// A sample is "hit" when it is in front of the camera and within a narrow
// angular cone (0.05·t ≈ 2.9°), with a 1.2-unit floor so nearby samples stay
// easy to click. The cone (rather than a fixed world radius) keeps distant
// samples clickable under perspective.
inline bool ray_hits(float perp, float t) {
    return t > 0.0f && perp < std::max(1.2f, 0.05f * t);
}

// Scale 0: nearest rendered sample to the ray → its lattice cell. Manifested
// particles are preferred; the ambient flux cloud is the fallback so a click on
// a field-only region still resolves a cell. Returns false (→ clear the
// inspector) when nothing is near the ray — a click on empty space.
bool pick_scale0(const ftd::native::NativeFrame& frame, const PickRay& ray, int L,
                 int& vx, int& vy, int& vz) {
    auto scan = [&](const std::vector<ftd::native::NativeParticle>& pts, float& best_perp,
                    float& bx, float& by, float& bz) {
        bool any = false;
        for (const ftd::native::NativeParticle& p : pts) {
            float t = 0.0f;
            const float perp = ray_perp(ray, p.x, p.y, p.z, t);
            if (ray_hits(perp, t) && perp < best_perp) {
                best_perp = perp; bx = p.x; by = p.y; bz = p.z; any = true;
            }
        }
        return any;
    };
    float bp = 1e30f, bx = 0.0f, by = 0.0f, bz = 0.0f;
    bool hit = scan(frame.particles, bp, bx, by, bz);
    if (!hit) hit = scan(frame.flux, bp, bx, by, bz);
    if (!hit) return false;
    const int hi = std::max(0, L - 1);
    vx = std::min(hi, std::max(0, static_cast<int>(std::floor(bx))));
    vy = std::min(hi, std::max(0, static_cast<int>(std::floor(by))));
    vz = std::min(hi, std::max(0, static_cast<int>(std::floor(bz))));
    return true;
}

// Scale 1: nearest particle to the ray → its index. frame.particles is 1:1 with
// the engine's particle list (Scale1Adapter::capture() preserves order), so the
// index feeds InspectParticle1 directly. Returns false (→ clear) on a miss.
bool pick_scale1(const ftd::native::NativeFrame& frame, const PickRay& ray, int& pidx) {
    float bp = 1e30f;
    int best = -1;
    for (std::size_t i = 0; i < frame.particles.size(); ++i) {
        const ftd::native::NativeParticle& p = frame.particles[i];
        float t = 0.0f;
        const float perp = ray_perp(ray, p.x, p.y, p.z, t);
        if (ray_hits(perp, t) && perp < bp) { bp = perp; best = static_cast<int>(i); }
    }
    if (best < 0) return false;
    pidx = best;
    return true;
}

// Parse "i,j,k" (the --inspect-voxel argument) into three ints. Returns false on
// a malformed value (the flag is then ignored with a warning).
bool parse_ijk(const std::string& s, int& i, int& j, int& k) {
    return std::sscanf(s.c_str(), "%d,%d,%d", &i, &j, &k) == 3;
}

}  // namespace ftd::native::app