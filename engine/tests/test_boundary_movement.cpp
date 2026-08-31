/**
 * test_boundary_movement.cpp
 *
 * Verifies phase_movement face handling with an in-budget raw velocity and
 * accumulated movement remainder:
 *   Dispersal       -> particle exhausts into the void
 *   Reflective      -> normal-velocity mirror bounce at the face
 *   Periodic        -> every face wraps; axis is orientation metadata only
 */

#include <array>
#include <cmath>
#include <iostream>
#include "ftd/render_bridge.h"

int failures = 0;

static void check(const char* name, bool cond) {
    std::cout << (cond ? "  PASS  " : "  FAIL  ") << name << "\n";
    if (!cond) ++failures;
}

static void disable_extras(ftd::RenderBridge& rb) {
    rb.toggles.wave_propagation = false;
    rb.toggles.coupling = false;
    rb.toggles.damping = false;
    rb.toggles.genesis = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.forces = false;
    rb.toggles.gravity = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.selective_damping = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.movement = true;
    rb.set_dt(1.0);
}

int main() {
    std::cout << "=== test_boundary_movement ===\n";

    const int L = 17;
    const int far_x = L - 1;
    const int c = L / 2;
    struct FaceCase {
        int sx, sy, sz;
        int tx, ty, tz;
        ftd::Vec3 velocity;
        ftd::Vec3 remainder;
    };
    const std::array<FaceCase, 6> faces{{
        {0, c, c, L - 1, c, c,
         {-0.5 * ftd::C_SPEED, 0.0, 0.0}, {-0.75, 0.0, 0.0}},
        {L - 1, c, c, 0, c, c,
         {0.5 * ftd::C_SPEED, 0.0, 0.0}, {0.75, 0.0, 0.0}},
        {c, 0, c, c, L - 1, c,
         {0.0, -0.5 * ftd::C_SPEED, 0.0}, {0.0, -0.75, 0.0}},
        {c, L - 1, c, c, 0, c,
         {0.0, 0.5 * ftd::C_SPEED, 0.0}, {0.0, 0.75, 0.0}},
        {c, c, 0, c, c, L - 1,
         {0.0, 0.0, -0.5 * ftd::C_SPEED}, {0.0, 0.0, -0.75}},
        {c, c, L - 1, c, c, 0,
         {0.0, 0.0, 0.5 * ftd::C_SPEED}, {0.0, 0.0, 0.75}},
    }};

    // --- Dispersal: no wrap to opposite face ---
    {
        ftd::RenderBridge rb(L);
        disable_extras(rb);
        rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
        rb.toggles.reflective_boundary = false;
        rb.force_cpu();

        rb.inject_particle(0, 8, 8, +1, ftd::Vec3{0.0, 0.0, 0.0});
        auto& source = rb.voxel_at(0, 8, 8);
        source.velocity = ftd::Vec3{-0.5 * ftd::C_SPEED, 0.0, 0.0};
        source.remainder = ftd::Vec3{-0.75, 0.0, 0.0};
        rb.tick();

        check("dispersal: source voxel void after exit attempt", rb.voxel_at(0, 8, 8).state == 0);
        check("dispersal: opposite face stays void", rb.voxel_at(far_x, 8, 8).state == 0);
        check("dispersal: causal projection is not invoked", rb.causal_projection_events_this_tick() == 0);
    }

    // --- Reflective: normal velocity reverses; tangential velocity survives ---
    {
        ftd::RenderBridge rb(L);
        disable_extras(rb);
        rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Reflective;
        rb.toggles.reflective_boundary = false;  // selector is authoritative
        rb.force_cpu();

        rb.inject_particle(0, 8, 8, +1, ftd::Vec3{0.0, 0.0, 0.0});
        auto& source = rb.voxel_at(0, 8, 8);
        source.velocity = ftd::Vec3{-0.5 * ftd::C_SPEED,
                                     0.1 * ftd::C_SPEED,
                                    -0.1 * ftd::C_SPEED};
        const double tangential_y = source.velocity.y;
        const double tangential_z = source.velocity.z;
        source.remainder = ftd::Vec3{-0.75, 0.0, 0.0};
        rb.tick();

        check("reflective: particle remains at source voxel", rb.voxel_at(0, 8, 8).state == 1);
        check("reflective: velocity x flipped", rb.voxel_at(0, 8, 8).velocity.x > 0.0);
        check("reflective: tangential velocity is unchanged",
              std::fabs(rb.voxel_at(0, 8, 8).velocity.y - tangential_y) < 1e-15
              && std::fabs(rb.voxel_at(0, 8, 8).velocity.z - tangential_z) < 1e-15);
        check("reflective: opposite face stays void", rb.voxel_at(far_x, 8, 8).state == 0);
        check("reflective: causal projection is not invoked", rb.causal_projection_events_this_tick() == 0);
    }

    // --- Periodic: all seams wrap even when orientation metadata is Z ---
    {
        ftd::RenderBridge rb(L);
        disable_extras(rb);
        rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
        rb.toggles.periodic_axis = ftd::PeriodicAxis::Z;
        rb.toggles.reflective_boundary = false;
        rb.force_cpu();

        rb.inject_particle(8, 8, far_x, +1, ftd::Vec3{});
        auto& source = rb.voxel_at(8, 8, far_x);
        source.velocity = ftd::Vec3{0.0, 0.0, 0.5 * ftd::C_SPEED};
        source.remainder = ftd::Vec3{0.0, 0.0, 0.75};
        rb.tick();

        check("periodic with Z orientation: forward face wraps to aft face",
              rb.voxel_at(8, 8, far_x).state == 0
              && rb.voxel_at(8, 8, 0).state == 1);
        check("periodic with Z orientation: wrapped velocity keeps its direction",
              rb.voxel_at(8, 8, 0).velocity.z > 0.0);
    }

    {
        ftd::RenderBridge rb(L);
        disable_extras(rb);
        rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
        rb.toggles.periodic_axis = ftd::PeriodicAxis::Z;
        rb.toggles.reflective_boundary = false;
        rb.force_cpu();

        rb.inject_particle(far_x, 8, 8, +1, ftd::Vec3{});
        auto& source = rb.voxel_at(far_x, 8, 8);
        source.velocity = ftd::Vec3{0.5 * ftd::C_SPEED, 0.0, 0.0};
        source.remainder = ftd::Vec3{0.75, 0.0, 0.0};
        rb.tick();

        check("periodic with Z orientation: lateral face also wraps",
              rb.voxel_at(far_x, 8, 8).state == 0
              && rb.voxel_at(0, 8, 8).state == 1);
    }

    // Every law owns the complete cubic boundary, not just the orientation pair.
    bool all_dispersal_faces = true;
    bool all_dispersal_face_hits = true;
    bool all_reflective_faces = true;
    bool all_periodic_faces = true;
    for (const FaceCase& face : faces) {
        {
            ftd::RenderBridge rb(L);
            disable_extras(rb);
            rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
            rb.toggles.periodic_axis = ftd::PeriodicAxis::Z;
            rb.force_cpu();
            rb.inject_particle(face.sx, face.sy, face.sz, +1, ftd::Vec3{});
            auto& source = rb.voxel_at(face.sx, face.sy, face.sz);
            source.velocity = face.velocity;
            source.remainder = face.remainder;
            rb.tick();
            all_dispersal_faces = all_dispersal_faces
                && rb.voxel_at(face.sx, face.sy, face.sz).state == 0
                && rb.voxel_at(face.tx, face.ty, face.tz).state == 0;
        }
        {
            // The face shell is the boundary, so an outward hop from the
            // adjacent interior voxel is retired immediately rather than
            // occupying the face until a later out-of-range attempt.
            const int ix = face.sx == 0 ? 1 : (face.sx == L - 1 ? L - 2 : face.sx);
            const int iy = face.sy == 0 ? 1 : (face.sy == L - 1 ? L - 2 : face.sy);
            const int iz = face.sz == 0 ? 1 : (face.sz == L - 1 ? L - 2 : face.sz);
            ftd::RenderBridge rb(L);
            disable_extras(rb);
            rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
            rb.toggles.periodic_axis = ftd::PeriodicAxis::Z;
            rb.force_cpu();
            rb.inject_particle(ix, iy, iz, +1, ftd::Vec3{});
            auto& source = rb.voxel_at(ix, iy, iz);
            source.velocity = face.velocity;
            source.remainder = face.remainder;
            rb.tick();
            all_dispersal_face_hits = all_dispersal_face_hits
                && rb.voxel_at(ix, iy, iz).state == 0
                && rb.voxel_at(face.sx, face.sy, face.sz).state == 0;
        }
        {
            ftd::RenderBridge rb(L);
            disable_extras(rb);
            rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Reflective;
            rb.toggles.periodic_axis = ftd::PeriodicAxis::Z;
            rb.force_cpu();
            rb.inject_particle(face.sx, face.sy, face.sz, +1, ftd::Vec3{});
            auto& source = rb.voxel_at(face.sx, face.sy, face.sz);
            source.velocity = face.velocity;
            source.remainder = face.remainder;
            rb.tick();
            const auto& settled = rb.voxel_at(face.sx, face.sy, face.sz);
            const double incoming = face.velocity.x != 0.0 ? face.velocity.x
                                  : face.velocity.y != 0.0 ? face.velocity.y
                                                          : face.velocity.z;
            const double outgoing = face.velocity.x != 0.0 ? settled.velocity.x
                                  : face.velocity.y != 0.0 ? settled.velocity.y
                                                          : settled.velocity.z;
            all_reflective_faces = all_reflective_faces
                && settled.state == 1 && incoming * outgoing < 0.0
                && rb.voxel_at(face.tx, face.ty, face.tz).state == 0;
        }
        {
            ftd::RenderBridge rb(L);
            disable_extras(rb);
            rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
            rb.toggles.periodic_axis = ftd::PeriodicAxis::Z;
            rb.force_cpu();
            rb.inject_particle(face.sx, face.sy, face.sz, +1, ftd::Vec3{});
            auto& source = rb.voxel_at(face.sx, face.sy, face.sz);
            source.velocity = face.velocity;
            source.remainder = face.remainder;
            rb.tick();
            all_periodic_faces = all_periodic_faces
                && rb.voxel_at(face.sx, face.sy, face.sz).state == 0
                && rb.voxel_at(face.tx, face.ty, face.tz).state == 1;
        }
    }
    check("dispersal owns all six particle-crossing faces", all_dispersal_faces);
    check("dispersal retires outward hops on first contact with all six faces",
          all_dispersal_face_hits);
    check("reflective owns all six particle-crossing faces", all_reflective_faces);
    check("periodic owns all six particle-crossing faces with Z orientation",
          all_periodic_faces);

    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
