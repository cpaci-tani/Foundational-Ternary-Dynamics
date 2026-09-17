/**
 * Test: the flux-cell port integrates the OPENED APERTURE, not the whole slab.
 *
 * WHAT THIS PINS. `RenderBridge::accumulate_flux_cell_port_work` integrates the
 * two port currents over `flux_cell_port_surface_`, the one-layer accounting
 * surface built in `apply_flux_cell_port`: the slab |d.n - surface_offset| <= 1/2
 * INTERSECTED WITH THE HOLE BALL. Two failure modes are easy to introduce here
 * and both are silent (the toggle defaults OFF and the port ledger still
 * "closes" approximately), so they get a test:
 *
 *   (a) integrating the whole lattice plane rather than the aperture — i.e.
 *       dropping the hole-ball restriction — would book energy that never went
 *       through the port;
 *   (b) integrating `flux_cell_port_sites_` (the expired WALL sites) instead —
 *       that set is a plug as thick as the membrane, so each streamline would
 *       be counted once per layer it crosses, and the parts of the aperture
 *       that were already void before the port opened (where most of the flux
 *       actually leaves) would be missed entirely.
 *
 * The test freezes the field (every dynamics toggle OFF, so the leapfrog adds
 * a zero wave_vel to a zero-wave_vel neighbourhood and nothing moves) and
 * places a single analytic probe — wave_vel at one site, an antisymmetric flux
 * pair on its +/-x neighbours — at chosen positions. The booked current is then
 * exactly that probe's contribution, so "counted" vs "not counted" is an exact
 * zero/non-zero decision rather than a tolerance.
 *
 * Checks:
 *   PAP-1: the port opens on schedule, expires wall sites, and the accounting
 *          surface is strictly SMALLER than the expired plug (one layer vs a
 *          3-layer membrane) — the plug is not what gets integrated.
 *   PAP-2: a probe at the aperture centre is counted (both channels non-zero).
 *   PAP-3: a probe elsewhere in the aperture is counted.
 *   PAP-4: a probe in the SAME lattice plane but outside the hole ball books
 *          exactly zero — the slab alone is not the accounting surface.
 *   PAP-5: a probe off the accounting plane books exactly zero.
 *   PAP-6: the two channels are distinct quantities (the wave-Hamiltonian
 *          current S^H and the EM-like Poynting current c^2 E x B are both
 *          reported, separately, and are not the same number).
 */

#include "ftd/constants.h"
#include "ftd/flux_cell.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <memory>

namespace {

constexpr int L = 25;
constexpr int C = 12;              // lattice centre
constexpr double PORT_RADIUS = 3.0;
constexpr int OPEN_TICK = 1;

// A 3-layer membrane slab normal to +x, centred on the port plane. Three
// layers so the expired plug is unambiguously thicker than the one-layer
// accounting surface.
constexpr int WALL_X0 = C - 1;
constexpr int WALL_X1 = C + 1;

std::unique_ptr<ftd::RenderBridge> make_port_bridge() {
    auto rb = std::make_unique<ftd::RenderBridge>(L);
    rb->force_cpu();
    rb->toggles.disable_all();
    rb->toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    rb->toggles.flux_cell_port = true;

    // Locked manifested wall. inject_particle sets state/id and overwrites
    // flux, so it runs before any probe seeding.
    for (int x = WALL_X0; x <= WALL_X1; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        rb->inject_particle(x, y, z, static_cast<int8_t>(((x + y + z) & 1) ? 1 : -1),
                            ftd::Vec3(0, 0, 0));
        rb->voxel_at(x, y, z).locked = true;
    }

    ftd::FluxCellPortSpec spec;
    spec.cx = C; spec.cy = C; spec.cz = C;
    spec.nx = 1.0; spec.ny = 0.0; spec.nz = 0.0;
    spec.radius = PORT_RADIUS;
    spec.surface_offset = 0.0;      // accounting plane through the hole centre
    spec.open_tick = OPEN_TICK;
    rb->set_flux_cell_port(spec);
    return rb;
}

// One analytic probe: wave_vel at (x,y,z) and antisymmetric flux pairs on its
// +/-x and +/-y neighbours, so flux_cell_site_hamiltonian_flux sees a non-zero
// (n.grad)J contracted with a non-zero E at exactly that site and nowhere else.
// The transverse J_x gradient makes the two channels differ: with n = x,
//     S.n - S^H.n = -c^2 (E.grad) J_x,
// so a non-zero d(J_x)/dy against a non-zero E_y separates them (PAP-6). Every
// neighbour seeded here carries wave_vel = 0, so no site other than (x,y,z)
// can contribute to either integral.
void seed_probe(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto wrap = [](int v) { return (v % L + L) % L; };
    rb.voxel_at(x, y, z).wave_vel = ftd::Vec3(0.0, 0.7, 0.4);
    rb.voxel_at(wrap(x + 1), y, z).flux = ftd::Vec3(0.0, 0.5, -0.3);
    rb.voxel_at(wrap(x - 1), y, z).flux = ftd::Vec3(0.0, -0.5, 0.3);
    rb.voxel_at(x, wrap(y + 1), z).flux = ftd::Vec3(0.6, 0.0, 0.0);
    rb.voxel_at(x, wrap(y - 1), z).flux = ftd::Vec3(-0.6, 0.0, 0.0);
}

struct PortRun {
    double work = 0.0;
    double poynting = 0.0;
    int surface = 0;
    int plug = 0;
};

PortRun run_with_probe(int px, int py, int pz) {
    auto rb = make_port_bridge();
    seed_probe(*rb, px, py, pz);
    rb->run(1);                     // opens at the top of tick 0, books once
    PortRun r;
    r.work = rb->flux_cell_port_work_out();
    r.poynting = rb->flux_cell_port_poynting_out();
    r.surface = rb->flux_cell_port_surface_count();
    r.plug = rb->flux_cell_port_site_count();
    return r;
}

}  // namespace

int main() {
    ftd::test::init("test_flux_cell_port_aperture");

    // ------------------------------------------------------------------
    ftd::test::section("PAP-1: the accounting surface is the aperture, not the plug");
    {
        auto rb = make_port_bridge();
        ftd::test::check("PAP-1: port configured and closed before open_tick",
                         rb->flux_cell_port_configured()
                             && !rb->flux_cell_port_open());
        rb->run(1);
        const int surface = rb->flux_cell_port_surface_count();
        const int plug = rb->flux_cell_port_site_count();
        std::printf("    plug (expired wall sites) = %d, accounting surface = %d\n",
                    plug, surface);
        ftd::test::metric("pap1.plug_sites", plug, 1);
        ftd::test::metric("pap1.surface_sites", surface, 1);

        ftd::test::check("PAP-1: port opened on schedule", rb->flux_cell_port_open());
        ftd::test::check("PAP-1: the port expired wall sites", plug > 0);
        ftd::test::check("PAP-1: the accounting surface is non-empty", surface > 0);
        // The plug spans 3 membrane layers inside the ball; the surface is the
        // single layer at dx = 0. If someone swaps the two sets, this fails.
        ftd::test::check("PAP-1: surface is strictly smaller than the plug "
                         "(one layer vs a 3-layer membrane)",
                         surface < plug);
        // Geometry: the surface is the disc dy^2 + dz^2 <= radius^2 at dx = 0.
        int expected = 0;
        for (int dy = -4; dy <= 4; ++dy)
        for (int dz = -4; dz <= 4; ++dz)
            if (dy * dy + dz * dz <= PORT_RADIUS * PORT_RADIUS) ++expected;
        ftd::test::check("PAP-1: surface size == the aperture cross-section disc",
                         surface == expected);
    }

    // ------------------------------------------------------------------
    ftd::test::section("PAP-2/3: flux through the aperture IS counted");
    double w_center = 0.0;
    {
        const PortRun centre = run_with_probe(C, C, C);
        std::printf("    probe at aperture centre: W=%.12g  S=%.12g\n",
                    centre.work, centre.poynting);
        ftd::test::metric("pap2.work", centre.work, 1);
        ftd::test::metric("pap2.poynting", centre.poynting, 1);
        w_center = centre.work;
        ftd::test::check("PAP-2: a probe at the aperture centre books a "
                         "non-zero wave-Hamiltonian current",
                         std::isfinite(centre.work)
                             && std::fabs(centre.work) > 1e-12);

        const PortRun offset = run_with_probe(C, C + 2, C);
        std::printf("    probe 2 cells off-axis, still inside the hole: W=%.12g\n",
                    offset.work);
        ftd::test::check("PAP-3: a probe elsewhere inside the aperture is also "
                         "counted",
                         std::isfinite(offset.work)
                             && std::fabs(offset.work) > 1e-12);
    }

    // ------------------------------------------------------------------
    ftd::test::section("PAP-4: flux elsewhere in the same plane is NOT counted");
    {
        // Same x = C accounting plane, but 14.1 cells from the hole centre —
        // far outside the radius-3 ball. If the integration ever drops the
        // hole-ball restriction and sums the whole slab, this becomes non-zero.
        const PortRun far = run_with_probe(C, 2, 2);
        std::printf("    probe in the same plane, outside the hole: W=%.17g  S=%.17g\n",
                    far.work, far.poynting);
        ftd::test::check("PAP-4: same-plane flux outside the hole books exactly "
                         "zero work",
                         far.work == 0.0);
        ftd::test::check("PAP-4: ... and exactly zero Poynting current",
                         far.poynting == 0.0);
        ftd::test::check("PAP-4: the aperture probe was measurable, so the zero "
                         "above is a real exclusion and not a dead integral",
                         std::fabs(w_center) > 1e-12);
    }

    // ------------------------------------------------------------------
    ftd::test::section("PAP-5: flux off the accounting plane is NOT counted");
    {
        // Inside the hole ball's x-range would be dx = 0 only; put the probe
        // three layers downstream, outside both the slab and the ball.
        const PortRun off = run_with_probe(C + 4, C, C);
        std::printf("    probe off the accounting plane: W=%.17g\n", off.work);
        ftd::test::check("PAP-5: off-plane flux books exactly zero work",
                         off.work == 0.0);
    }

    // ------------------------------------------------------------------
    ftd::test::section("PAP-6: the two port channels are distinct quantities");
    {
        const PortRun centre = run_with_probe(C, C, C);
        std::printf("    S^H channel = %.12g, EM Poynting channel = %.12g\n",
                    centre.work, centre.poynting);
        // S^H = c^2 sum_a E_a grad J_a closes the wave-Hamiltonian ledger;
        // c^2 E x B differs from it by the curl-type term c^2 (E.grad)J and is
        // reported separately. They must not be silently conflated.
        ftd::test::check("PAP-6: the wave-Hamiltonian current and the EM-like "
                         "Poynting current are reported separately and differ",
                         std::fabs(centre.work - centre.poynting) > 1e-12);
    }

    return ftd::test::finalize();
}
