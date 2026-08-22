// Boundary-shape wireframes — see native/boundary_shapes.h.
// Port of engine/web/js/viewport/boundary-geometry.js. Every shape is a set of
// coloured line segments centred on the lattice ((N/2)^3, half-extent N/2), the
// same frame the legacy cube used. Deterministic; no RNG.

#include "native/boundary_shapes.h"

#include <array>
#include <cmath>
#include <cstring>
#include <vector>

namespace ftd::native {
namespace {

constexpr float kR = 0.35f, kG = 0.42f, kB = 0.52f;   // boundary wire colour (matches legacy cube)
constexpr double kPi = 3.14159265358979323846;

struct V3 { double x, y, z; };

inline double len(const V3& a) { return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z); }
inline double dist2(const V3& a, const V3& b) {
    const double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

// Map a relative coord in the unit cube [-1,1]^3 to lattice world coords
// (centre N/2, half-extent N/2) and push a segment a->b into `out`.
struct Emitter {
    std::vector<NativeLine>& out;
    double c, h;   // centre, half-extent (both N/2)
    void line(const V3& a, const V3& b) {
        NativeLine l;
        l.x0 = float(c + a.x * h); l.y0 = float(c + a.y * h); l.z0 = float(c + a.z * h);
        l.x1 = float(c + b.x * h); l.y1 = float(c + b.y * h); l.z1 = float(c + b.z * h);
        l.r0 = l.r1 = kR; l.g0 = l.g1 = kG; l.b0 = l.b1 = kB;
        out.push_back(l);
    }
    // A closed polyline through `pts` (wraps last->first).
    void ring(const std::vector<V3>& pts) {
        for (std::size_t i = 0; i < pts.size(); ++i)
            line(pts[i], pts[(i + 1) % pts.size()]);
    }
};

// Connect every vertex pair at (approximately) the minimum pairwise distance —
// the edges of a convex regular polyhedron. Verts are normalised to the unit
// sphere first so the half-extent scales them uniformly.
void emit_polyhedron_edges(Emitter& e, std::vector<V3> verts) {
    for (V3& v : verts) {
        const double n = len(v);
        if (n > 1e-9) { v.x /= n; v.y /= n; v.z /= n; }
    }
    double mind2 = 1e30;
    for (std::size_t i = 0; i < verts.size(); ++i)
        for (std::size_t j = i + 1; j < verts.size(); ++j)
            mind2 = std::min(mind2, dist2(verts[i], verts[j]));
    const double thresh = mind2 * 1.02;   // small tolerance for float wobble
    for (std::size_t i = 0; i < verts.size(); ++i)
        for (std::size_t j = i + 1; j < verts.size(); ++j)
            if (dist2(verts[i], verts[j]) <= thresh) e.line(verts[i], verts[j]);
}

void build_cube(Emitter& e) {
    const V3 c[8] = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1},
    };
    const int ed[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
                           {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    for (auto& s : ed) e.line(c[s[0]], c[s[1]]);
}

void build_sphere(Emitter& e) {
    constexpr int kSeg = 40;              // segments per circle
    // Latitude circles at 5 heights + longitude meridians every 30°, plus the
    // three axis-aligned great circles for structure (as the web's ring cue).
    const double lats[5] = {-0.6, -0.3, 0.0, 0.3, 0.6};
    for (double sy : lats) {
        const double rr = std::sqrt(std::max(0.0, 1.0 - sy * sy));
        std::vector<V3> pts;
        for (int i = 0; i < kSeg; ++i) {
            const double t = 2.0 * kPi * i / kSeg;
            pts.push_back({rr * std::cos(t), sy, rr * std::sin(t)});
        }
        e.ring(pts);
    }
    for (int m = 0; m < 12; ++m) {
        const double lon = 2.0 * kPi * m / 12.0;
        std::vector<V3> pts;
        for (int i = 0; i <= kSeg / 2; ++i) {       // pole-to-pole meridian
            const double t = kPi * i / (kSeg / 2);   // 0..pi
            const double rr = std::sin(t);
            pts.push_back({rr * std::cos(lon), std::cos(t), rr * std::sin(lon)});
        }
        for (std::size_t i = 0; i + 1 < pts.size(); ++i) e.line(pts[i], pts[i + 1]);
    }
}

void build_cylinder(Emitter& e) {
    constexpr int kSeg = 48;
    // Axis along Y, radius 1 in XZ, y in [-1,1]. Cap circles + vertical side wires.
    for (double sy : {-1.0, 1.0}) {
        std::vector<V3> pts;
        for (int i = 0; i < kSeg; ++i) {
            const double t = 2.0 * kPi * i / kSeg;
            pts.push_back({std::cos(t), sy, std::sin(t)});
        }
        e.ring(pts);
    }
    for (int i = 0; i < 16; ++i) {                  // side wires
        const double t = 2.0 * kPi * i / 16.0;
        e.line({std::cos(t), -1.0, std::sin(t)}, {std::cos(t), 1.0, std::sin(t)});
    }
}

void build_torus(Emitter& e) {
    // Hole along Y (donut lying flat). Major radius R, minor r (in unit-cube space).
    constexpr double R = 0.72, r = 0.28;
    constexpr int kMajor = 36, kMinor = 18;
    auto pt = [&](double u, double v) -> V3 {
        const double cu = std::cos(u), su = std::sin(u), cv = std::cos(v), sv = std::sin(v);
        return {(R + r * cv) * cu, r * sv, (R + r * cv) * su};   // hole along Y
    };
    for (int j = 0; j < kMinor; ++j) {              // minor rings (tube cross-sections)
        const double v0 = 2.0 * kPi * j / kMinor;
        std::vector<V3> pts;
        for (int i = 0; i < kMajor; ++i) pts.push_back(pt(2.0 * kPi * i / kMajor, v0));
        e.ring(pts);
    }
    for (int i = 0; i < kMajor; ++i) {              // longitudinal loops
        const double u0 = 2.0 * kPi * i / kMajor;
        std::vector<V3> pts;
        for (int j = 0; j < kMinor; ++j) pts.push_back(pt(u0, 2.0 * kPi * j / kMinor));
        e.ring(pts);
    }
}

}  // namespace

void build_boundary_lines(int shape, int lattice_size, std::vector<NativeLine>& out) {
    out.clear();
    const double c = std::max(1, lattice_size) * 0.5;
    Emitter e{out, c, c};
    const double phi = (1.0 + std::sqrt(5.0)) / 2.0;
    switch (static_cast<BoundaryShape>(shape)) {
        case BoundaryShape::Cube:   build_cube(e); break;
        case BoundaryShape::Sphere: build_sphere(e); break;
        case BoundaryShape::Octahedron:
            emit_polyhedron_edges(e, {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
                                      {0, -1, 0}, {0, 0, 1}, {0, 0, -1}});
            break;
        case BoundaryShape::Icosahedron:
            emit_polyhedron_edges(e, {
                {0, 1, phi}, {0, 1, -phi}, {0, -1, phi}, {0, -1, -phi},
                {1, phi, 0}, {1, -phi, 0}, {-1, phi, 0}, {-1, -phi, 0},
                {phi, 0, 1}, {phi, 0, -1}, {-phi, 0, 1}, {-phi, 0, -1}});
            break;
        case BoundaryShape::Dodecahedron: {
            std::vector<V3> v = {{1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1},
                                 {-1, 1, 1}, {-1, 1, -1}, {-1, -1, 1}, {-1, -1, -1}};
            const double ip = 1.0 / phi;
            for (double a : {-1.0, 1.0}) for (double b : {-1.0, 1.0}) {
                v.push_back({0, a * ip, b * phi});
                v.push_back({a * ip, b * phi, 0});
                v.push_back({a * phi, 0, b * ip});
            }
            emit_polyhedron_edges(e, std::move(v));
            break;
        }
        case BoundaryShape::Cylinder: build_cylinder(e); break;
        case BoundaryShape::Torus:    build_torus(e); break;
        case BoundaryShape::None:
        default: break;   // no lines
    }
}

namespace {
constexpr const char* kNames[] = {"cube",     "sphere",   "dodecahedron", "icosahedron",
                                  "octahedron", "cylinder", "torus",        "none"};
}

const char* boundary_shape_name(int shape) {
    if (shape < 0 || shape >= static_cast<int>(BoundaryShape::Count)) return "";
    return kNames[shape];
}

int boundary_shape_from_name(const char* name) {
    if (!name) return -1;
    for (int i = 0; i < static_cast<int>(BoundaryShape::Count); ++i)
        if (std::strcmp(name, kNames[i]) == 0) return i;
    return -1;
}

}  // namespace ftd::native
