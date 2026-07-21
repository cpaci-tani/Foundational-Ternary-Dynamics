/**
 * FTD-0402 exact causal-normalization and mass-role contract.
 */

#include <cmath>
#include <limits>
#include <iostream>

#include "ftd/causal_kinematics.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

using namespace ftd;

int main() {
    std::cout << "=== FTD-0402 causal normalization ===\n";

    static_assert(M_INERTIAL == K_B, "inertial calibration must remain K_B");
    static_assert(M_GRAVITATIONAL == K_B, "gravity charge must remain K_B");
    static_assert(M_REST == M_INERTIAL, "compatibility alias drifted");

    const double C2 = C_SPEED * C_SPEED;
    ftd::test::check_close("CN-1 C_SPEED^2 = 1/3", C2, 1.0 / 3.0, 1e-15);
    ftd::test::check_close("CN-2 E_REST = M_INERTIAL*C^2", E_REST,
                           M_INERTIAL * C2, 1e-15);
    ftd::test::check("CN-3 rate^2(u=C,L=0)=0",
                     proper_time_rate(0.0, C2) == 0.0);
    const double half_rate = proper_time_rate(0.0, C2 / 4.0);
    ftd::test::check_close("CN-4 rate^2(u=C/2,L=0)=3/4",
                           half_rate * half_rate, 0.75, 1e-15);
    for (double L : {0.0, 0.25, 0.8}) {
        const double rate = proper_time_rate(L, 0.0);
        ftd::test::check_close("CN-5 rest rate^2=1-L^2",
                               rate * rate, 1.0 - L * L, 1e-15);
    }

    const double L = 0.4;
    const double umax = max_raw_speed(L);
    ftd::test::check_close("CN-6 selected boundary bandwidth=1",
                           bandwidth_fraction(L, umax * umax), 1.0, 2e-15);
    ftd::test::check_close("CN-7 selected boundary budget=1",
                           causal_budget(L, umax * umax), 1.0, 2e-15);
    const double u2 = 0.2 * 0.2;
    const double rate = proper_time_rate(L, u2);
    const double gamma = transport_gamma(L, u2);
    ftd::test::check_close("CN-8 rate*gamma=1", rate * gamma, 1.0, 1e-15);

    // Born-Infeld Legendre pair. p=dL/du=M*gamma*u and H=p*u-L.
    const double u = 0.3 * C_SPEED;
    const double latency = 0.2;
    const double r = proper_time_rate(latency, u * u);
    const double lag = born_infeld_core(latency, u * u);
    const double p = M_INERTIAL * u / r;
    const double h_legendre = p * u - lag;
    const double h_closed = born_infeld_hamiltonian(latency, u * u);
    ftd::test::check_close("CN-9 Born-Infeld Legendre transform",
                           h_legendre, h_closed, 2e-15);

    // Flat energy-momentum invariant in raw lattice units.
    const double beta = 0.6;
    const double uf = beta * C_SPEED;
    const double gamma0 = flat_gamma(uf * uf);
    const double energy = gamma0 * E_REST;
    const double momentum = gamma0 * M_INERTIAL * uf;
    ftd::test::check_close("CN-10 E^2=E0^2+C^2P^2",
                           energy * energy,
                           E_REST * E_REST + C2 * momentum * momentum,
                           2e-15);

    // Horizons and non-finite input are closed, finite diagnostics.
    ftd::test::check("CN-11 horizon rate=0",
                     proper_time_rate(1.0, 0.0) == 0.0);
    ftd::test::check("CN-12 horizon gamma sentinel", transport_gamma(1.0, 0.0) >= 1e20);
    ftd::test::check("CN-13 NaN speed rate=0",
        proper_time_rate(0.0, std::numeric_limits<double>::quiet_NaN()) == 0.0);

    // Exact public audit semantics for one flat moving particle.
    {
        RenderBridge rb(8);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.inject_particle(3, 3, 3, +1, {});
        auto& v = rb.voxel_at(3, 3, 3);
        v.velocity = {C_SPEED / 2.0, 0.0, 0.0};
        const EnergyAudit a = rb.energy_audit();
        const double g = 2.0 / std::sqrt(3.0);
        ftd::test::check_close("CN-14 audit rest energy", a.particle_rest_energy, E_REST, 1e-15);
        ftd::test::check_close("CN-15 audit particle KE", a.particle_ke,
                               (g - 1.0) * E_REST, 1e-15);
        ftd::test::check_close("CN-16 audit momentum x", a.particle_momentum.x,
                               g * M_INERTIAL * C_SPEED / 2.0, 1e-15);
        ftd::test::check_close("CN-17 dynamic excludes rest",
                               a.dynamic_energy,
                               a.field_energy + a.wave_energy + a.particle_ke, 1e-15);
        ftd::test::check_close("CN-18 accounted total includes rest",
                               a.total_energy, a.dynamic_energy + E_REST, 1e-15);
    }

    // Movement is the last-resort repair point for direct external mutation.
    {
        RenderBridge rb(8);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.toggles.movement = true;
        rb.inject_particle(3, 3, 3, +1, {});
        rb.voxel_at(3, 3, 3).velocity = {2.0 * C_SPEED, 0.0, 0.0};
        rb.tick();
        const auto& v = rb.voxel_at(3, 3, 3);
        ftd::test::check("CN-19 external overspeed projected inside B<1",
                         v.causal_budget() < 1.0);
        ftd::test::check("CN-20 external projection counted once",
                         rb.causal_projection_events_this_tick() == 1);
    }
    {
        RenderBridge rb(8);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.toggles.movement = true;
        rb.inject_particle(3, 3, 3, +1, {});
        rb.voxel_at(3, 3, 3).velocity = {
            std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0};
        rb.tick();
        const auto& v = rb.voxel_at(3, 3, 3);
        ftd::test::check("CN-21 non-finite external velocity becomes zero",
                         v.velocity.mag2() == 0.0);
        ftd::test::check("CN-22 non-finite repair counted",
                         rb.causal_projection_events_this_tick() == 1);
    }

    // A very large native base force is integrated through momentum and must
    // arrive causal without needing the movement-entry repair.
    {
        RenderBridge rb(9);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.toggles.forces = true;
        rb.toggles.emergent_forces = true;
        rb.toggles.movement = true;
        rb.inject_particle(4, 4, 4, +1, {});
        rb.voxel_at(6, 4, 4).flux = {1e12, 0.0, 0.0};
        rb.tick();
        const auto& v = rb.voxel_at(4, 4, 4);
        ftd::test::check("CN-23 ordinary huge force remains B<1",
                         v.causal_budget() < 1.0);
        ftd::test::check("CN-24 ordinary force needs zero projection",
                         rb.causal_projection_events_this_tick() == 0);
    }

#ifdef FTD_ENABLE_CUDA
    // GPU force additions all feed the same single momentum integration. A
    // deliberately huge base-field force stresses the causal map while each
    // optional pair path proves that its diagnostic contribution was active.
    struct ForcePathResult {
        bool causal = false;
        bool zero_projection = false;
        double base_component = 0.0;
        double optional_component = 0.0;
    };
    auto run_gpu_force_path = [](bool color, bool yukawa, bool exchange) {
        RenderBridge rb(10);
        rb.toggles.disable_all();
        rb.toggles.forces = true;
        rb.toggles.emergent_forces = true;
        rb.toggles.movement = true;
        rb.toggles.color_forces = color;
        rb.toggles.strong_force = yukawa;
        rb.toggles.exchange_force = exchange;
        rb.toggles.poisson_coulomb = exchange;
        rb.inject_particle(4, 4, 4, +1, {}, +1, 1);
        rb.inject_particle(5, 4, 4, +1, {}, +1, color ? 2 : 1);
        rb.voxel_at(6, 4, 4).flux = {1e12, 0.0, 0.0};
        rb.tick();
        rb.sync_from_gpu();

        ForcePathResult out;
        out.causal = true;
        const RenderBridge& view = rb;
        for (const auto& v : view.voxels())
            if (v.state != 0) out.causal = out.causal && v.causal_budget() < 1.0;
        out.zero_projection = rb.causal_projection_events_this_tick() == 0;
        const auto& fd = rb.force_diag()[rb.lattice().index(4, 4, 4)];
        out.base_component = fd.f_coulomb.mag();
        out.optional_component = exchange ? fd.f_exchange.mag() : fd.f_strong.mag();
        return out;
    };
    {
        const auto r = run_gpu_force_path(false, false, false);
        ftd::test::check("CN-F1 GPU base force path active", r.base_component > 1e6);
        ftd::test::check("CN-F2 GPU base force remains causal", r.causal && r.zero_projection);
    }
    {
        const auto r = run_gpu_force_path(true, false, false);
        ftd::test::check("CN-F3 GPU color addition active", r.optional_component > 0.0);
        ftd::test::check("CN-F4 GPU color addition remains causal", r.causal && r.zero_projection);
    }
    {
        const auto r = run_gpu_force_path(false, true, false);
        ftd::test::check("CN-F5 GPU Yukawa addition active", r.optional_component > 0.0);
        ftd::test::check("CN-F6 GPU Yukawa addition remains causal", r.causal && r.zero_projection);
    }
    {
        const auto r = run_gpu_force_path(false, false, true);
        ftd::test::check("CN-F7 GPU exchange addition active", r.optional_component > 0.0);
        ftd::test::check("CN-F8 GPU exchange addition remains causal", r.causal && r.zero_projection);
    }
#endif

    // Sixteen-tick duplicate determinism for tau/phase under the common host
    // proper-time path.
    {
        RenderBridge a(8), b(8);
        a.force_cpu(); b.force_cpu();
        a.toggles.disable_all(); b.toggles.disable_all();
        a.toggles.de_broglie_clock = true;
        b.toggles.de_broglie_clock = true;
        a.toggles.omega0 = OMEGA0_COMPTON;
        b.toggles.omega0 = OMEGA0_COMPTON;
        a.inject_particle(3, 3, 3, +1, {0.1, 0.0, 0.0});
        b.inject_particle(3, 3, 3, +1, {0.1, 0.0, 0.0});
        a.voxel_at(3, 3, 3).velocity = {C_SPEED / 4.0, 0.0, 0.0};
        b.voxel_at(3, 3, 3).velocity = {C_SPEED / 4.0, 0.0, 0.0};
        for (int i = 0; i < 16; ++i) { a.tick(); b.tick(); }
        const auto& va = a.voxel_at(3, 3, 3);
        const auto& vb = b.voxel_at(3, 3, 3);
        ftd::test::check("CN-25 duplicate tau bit-identical", va.tau == vb.tau);
        ftd::test::check("CN-26 duplicate phase bit-identical", va.phase == vb.phase);
        ftd::test::check("CN-27 duplicate causal budget bit-identical",
                         va.causal_budget() == vb.causal_budget());
    }

    // Backend parity is evaluated in a deliberately exact fixture: field and
    // force evolution are off, the shared evaporation RNG is active, and the
    // proper-time post-pass is common. This avoids conflating the contract with
    // historically tolerated CPU/GPU stencil trajectory error.
#ifdef FTD_ENABLE_CUDA
    {
        RenderBridge cpu(8), gpu(8);
        cpu.force_cpu();
        ftd::test::check("CN-28 reference backend is CPU",
                         cpu.backend_kind() == Backend::Kind::Cpu);
        ftd::test::check("CN-29 comparison backend is GPU",
                         gpu.backend_kind() == Backend::Kind::Gpu);
        for (RenderBridge* rb : {&cpu, &gpu}) {
            rb->toggles.disable_all();
            rb->toggles.evaporation = true;
            rb->toggles.de_broglie_clock = true;
            rb->toggles.omega0 = 0.25;
            rb->seed_rng(20260402u);
            // A high-field clock particle remains present while zero-field
            // particles exercise the proper-time-scaled evaporation hazard.
            rb->inject_particle(3, 3, 3, +1, {5.0, 0.0, 0.0});
            rb->voxel_at(3, 3, 3).velocity = {C_SPEED / 4.0, 0.0, 0.0};
            for (int x = 1; x <= 6; ++x)
                for (int y = 1; y <= 6; y += 2)
                    rb->inject_particle(x, y, 6, (x + y) % 2 ? +1 : -1, {});
        }

        int initial_count = cpu.energy_audit().manifested_count;
        bool exact_states = true;
        bool exact_clock = true;
        bool exact_budget = true;
        bool exact_audit = true;
        int count_after_one = initial_count;
        int gpu_count_after_one = initial_count;
        for (int tick = 1; tick <= 16; ++tick) {
            cpu.tick();
            gpu.tick();
            gpu.sync_from_gpu();
            for (std::size_t i = 0; i < cpu.voxels().size(); ++i)
                exact_states = exact_states &&
                               cpu.voxels()[i].state == gpu.voxels()[i].state;
            const auto& cv = cpu.voxel_at(3, 3, 3);
            const auto& gv = gpu.voxel_at(3, 3, 3);
            exact_clock = exact_clock && cv.tau == gv.tau && cv.phase == gv.phase;
            exact_budget = exact_budget && cv.causal_budget() == gv.causal_budget();
            const EnergyAudit ca = cpu.energy_audit();
            const EnergyAudit ga = gpu.energy_audit();
            exact_audit = exact_audit &&
                          ca.dynamic_energy == ga.dynamic_energy &&
                          ca.total_energy == ga.total_energy &&
                          ca.particle_energy == ga.particle_energy &&
                          ca.particle_momentum.x == ga.particle_momentum.x &&
                          ca.particle_momentum.y == ga.particle_momentum.y &&
                          ca.particle_momentum.z == ga.particle_momentum.z;
            if (tick == 1) {
                count_after_one = ca.manifested_count;
                gpu_count_after_one = ga.manifested_count;
            }
        }
        const int final_count = cpu.energy_audit().manifested_count;
        ftd::test::check("CN-30 CPU/GPU one-tick evaporation decision parity",
                         count_after_one == gpu_count_after_one);
        ftd::test::check("CN-31 CPU/GPU sixteen-tick evaporation hazard parity",
                         exact_states && final_count < initial_count);
        ftd::test::check("CN-32 CPU/GPU tau and phase bit parity", exact_clock);
        ftd::test::check("CN-33 CPU/GPU causal-budget bit parity", exact_budget);
        ftd::test::check("CN-34 CPU/GPU energy and momentum bit parity", exact_audit);
    }
#endif

    return ftd::test::finalize();
}
