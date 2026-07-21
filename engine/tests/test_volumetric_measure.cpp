/**
 * FTD-0404: cubic cell measure and density/integral separation.
 */

#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"
#include "ftd/volumetric_measure.h"

#include <cmath>

int main() {
    using namespace ftd;

    ftd::test::init("volumetric_measure");

    ftd::test::check("VM-1 D_SPATIAL is three", D_SPATIAL == 3);
    constexpr double exact_tol = 1e-15;
    ftd::test::check_close("VM-2 unit face area", VOXEL_FACE_AREA, 1.0, exact_tol);
    ftd::test::check_close("VM-3 unit cell volume", VOXEL_VOLUME, 1.0, exact_tol);
    ftd::test::check_close("VM-4 edge-two face area is four",
                           square_face_area(2.0), 4.0, exact_tol);
    ftd::test::check_close("VM-5 edge-two cell volume is eight",
                           cubic_cell_volume(2.0), 8.0, exact_tol);
    ftd::test::check_close("VM-6 density integrates with cubic measure",
                           integrate_voxel_density(3.5, cubic_cell_volume(2.0)),
                           28.0, exact_tol);

    const Vec3 axis{0.0, 0.0, 5.0};
    const Vec3 rotated{3.0, 4.0, 0.0};
    ftd::test::check_close("VM-7 quadratic norm is rotationally invariant",
                           axis.mag2(), rotated.mag2(), exact_tol);

    RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.disable_all();

    auto& field_site = rb.voxel_at(2, 3, 4);
    field_site.flux = {1.0, 2.0, 2.0};       // |J|^2 = 9
    field_site.wave_vel = {2.0, 0.0, 0.0};   // |Jdot|^2 = 4

    rb.inject_particle(5, 5, 5, +1, {});
    rb.voxel_at(5, 5, 5).velocity = {C_SPEED / 2.0, 0.0, 0.0};

    const EnergyAudit audit = rb.energy_audit();
    ftd::test::check_close("VM-8 audit exposes cell volume",
                           audit.cell_volume, VOXEL_VOLUME, exact_tol);
    ftd::test::check_close("VM-9 field density sum",
                           audit.field_energy_density_sum, 4.5, exact_tol);
    ftd::test::check_close("VM-10 wave density sum",
                           audit.wave_energy_density_sum, 2.0, exact_tol);
    ftd::test::check_close("VM-11 integrated field energy",
                           audit.field_energy,
                           integrate_voxel_density(4.5), exact_tol);
    ftd::test::check_close("VM-12 integrated wave energy",
                           audit.wave_energy,
                           integrate_voxel_density(2.0), exact_tol);
    ftd::test::check_close("VM-13 point rest energy is not volume-scaled",
                           audit.particle_rest_energy, E_REST, exact_tol);

    const double source_density = local_field_wave_energy_density(9.0, 4.0);
    ftd::test::check_close("VM-14 local gravity source is density",
                           source_density, 6.5, exact_tol);
    ftd::test::check("VM-15 density differs from edge-two integrated energy",
                     source_density != integrate_voxel_density(
                         source_density, cubic_cell_volume(2.0)));

    const LagrangianDiag lag = compute_lagrangian_diagnostics(rb);
    ftd::test::check_close("VM-16 Lagrangian exposes cell volume",
                           lag.cell_volume, VOXEL_VOLUME, exact_tol);
    ftd::test::check_close("VM-17 Lagrangian wave integral matches audit",
                           lag.total_wave_energy, audit.wave_energy, exact_tol);

    return ftd::test::finalize();
}
