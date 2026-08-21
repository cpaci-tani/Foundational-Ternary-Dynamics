#pragma once
//
// app/app_pick.h — camera framing + click-to-inspect ray picking (Scale-0 voxel /
// Scale-1 particle). Split out of app/main.cpp (behavior-neutral). ray_perp and
// ray_hits stay file-local in the .cpp (used only by pick_scale0/1).
//
#include "native/d3d12_presenter.h"  // ftd::native::Camera
#include "native/native_frame.h"     // ftd::native::NativeFrame
#include "native/scene_rect.h"       // ftd::native::SceneRect

#include <string>

namespace ftd::native::app {

void apply_camera_for_lattice(ftd::native::Camera& cam, int lattice);
// ── Click-to-inspect: unproject a scene click to a world ray, then pick ──────
struct PickRay {
    float ox = 0.0f, oy = 0.0f, oz = 0.0f;   // origin (camera eye)
    float dx = 0.0f, dy = 1.0f, dz = 0.0f;   // unit direction (into the scene)
};

PickRay make_pick_ray(const ftd::native::Camera& cam, const ftd::native::SceneRect& rect,
                      int client_x, int client_y);
bool pick_scale0(const ftd::native::NativeFrame& frame, const PickRay& ray, int L,
                 int& vx, int& vy, int& vz);
bool pick_scale1(const ftd::native::NativeFrame& frame, const PickRay& ray, int& pidx);
bool parse_ijk(const std::string& s, int& i, int& j, int& k);

}  // namespace ftd::native::app