#pragma once
//
// model/draw_list.h — the scale-generic render vocabulary (SPEC_NATIVE_REBUILD_R0R1 §4.3).
//
// A DrawList is what an adapter's capture() emits: a bag of GPU-primitive
// descriptions the renderer consumes, one pipeline per primitive kind. It is
// deliberately free of any concrete scale type — every scale (voxel field,
// particle N-body, atoms, cosmic bodies) expresses its frame as the SAME five
// primitives plus a HUD/scene hint.
//
// R1 STEP 1 STATUS: the *types* are defined now, per decision A (all types up
// front). The live Scale-0 render path in this step still travels through the
// legacy NativeFrame (native/native_frame.h) into D3D12Presenter::render(); the
// DrawList renderer overhaul is a later step. Defining the vocabulary now lets
// Scale-1+ adapters target a stable contract without reshaping the seam.
//
#include <cstdint>
#include <string>
#include <vector>

namespace ftd::native {

enum class Blend : std::uint8_t { Normal, Additive };
enum class Shape : std::uint8_t { Disc, Square, Diamond, Star, Triangle, Hexagon, Ring, Cross };
enum class SizeMode : std::uint8_t { PerspLinear, SqrtDepth };
enum class MeshId : std::uint8_t {
    Cone, Cylinder, Sphere, Ring, BoxEdges, Octahedron, Cuboctahedron, Cube
};
enum class PassId : std::uint8_t {
    BhDisk, BhJet, PlanetTerrain, StarSurface,
    BgStarfield, BgNebula, BgFoam, BgFluxStorm, BgBeyond
};
enum class BoundaryShape : std::uint8_t { Cube, Sphere, None };

struct PointCloud {                 // particles · flux voxels · bodies · atoms · field samples
    std::vector<float> pos;         // count*3
    std::vector<float> rgba;        // count*4
    std::vector<float> size;        // count
    Blend    blend = Blend::Normal;
    Shape    shape = Shape::Disc;
    SizeMode size_mode = SizeMode::PerspLinear;
    bool     manifest_blink = false;
};

struct LineSet {                    // vectors · streamlines · bonds · orbits · axes · wireframes
    std::vector<float> verts;       // segment pairs (2N*3) or strip
    std::vector<float> rgba;        // per-vertex
    float  width  = 1.0f;
    bool   strip  = false;
    bool   dashed = false;
    Blend  blend  = Blend::Normal;
};

struct InstanceSet {                // glyphs · nuclei · orbital shells · polyhedra · bonds
    MeshId mesh = MeshId::Cone;
    std::vector<float> xform;       // count*16 (row-major 4x4)
    std::vector<float> rgba;        // count*4
    bool   lit = false;
};

struct SheetMesh {                  // topology rubber-sheets (Scale 0)
    int nx = 0, ny = 0;
    std::vector<float> height;      // nx*ny
    std::vector<float> rgba;        // per-vertex
    bool wireframe_twin = true;
};

struct CustomPass {                 // bespoke HLSL: BH disk/jets · planet terrain · backgrounds
    PassId pass = PassId::BhDisk;
    std::vector<float> params;      // pass uniforms
    std::vector<float> instances;   // optional per-instance
};

struct LegendSpec {
    float ramp_lo = 0.0f, ramp_hi = 1.0f;
    int   requested_stride = 1, effective_stride = 1;
    std::string units;
    std::string origin;
};

struct LabelSpec {
    float pos[3] = {0, 0, 0};
    std::string text;
};

struct Selection {
    bool  present = false;
    float pos[3] = {0, 0, 0};
    std::string label;
};

struct HudSpec {                    // 2D chrome (drawn by the UI layer, not a GPU pass)
    bool axis_gizmo = true;
    LegendSpec legend;
    std::vector<LabelSpec> labels;
    bool has_selection = false;
    Selection selection;
};

struct SceneParams {                // camera + world hints
    BoundaryShape boundary = BoundaryShape::Cube;
    bool has_background = false;
    PassId background = PassId::BgStarfield;
    float cam_target[3] = {0, 0, 0};
};

struct DrawList {
    std::vector<PointCloud>  points;
    std::vector<LineSet>     lines;
    std::vector<InstanceSet> instances;
    std::vector<SheetMesh>   sheets;
    std::vector<CustomPass>  custom;
    HudSpec                  hud;
    SceneParams              scene;
};

}  // namespace ftd::native
