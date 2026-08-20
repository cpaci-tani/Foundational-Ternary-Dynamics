#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/gpu_buffers.h"
#include "ftd/gpu_engine.h"
#include "ftd/proper_time_rate.h"
#include "ftd/transmutation_phases.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool ok) {
    if (!ok) {
        ++failures;
        std::cerr << "FAIL: " << name << '\n';
    }
}

void close(const std::string& name, double a, double b,
           double rel = 2e-9, double abs = 2e-9) {
    const double tolerance = std::max(abs, rel * std::max(std::abs(a), std::abs(b)));
    check(name, std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= tolerance);
    if (std::abs(a - b) > tolerance)
        std::cerr << "  cpu=" << a << " gpu=" << b << " tol=" << tolerance << '\n';
}

void populate(ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    auto& voxels = rb.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = rb.lattice().index(x, y, z);
        auto& v = voxels[static_cast<std::size_t>(i)];
        const double q = static_cast<double>((x * 7 + y * 5 + z * 3) % 17 - 8) / 41.0;
        v.flux = {q + 0.01 * x, -0.7 * q + 0.005 * y, 0.4 * q - 0.003 * z};
        v.wave_vel = {-0.12 * q, 0.08 * q + 0.002 * z, -0.04 * q};
        v.flux_L = v.flux * 0.63;
        v.flux_R = v.flux * 0.37;
        v.wave_vel_L = v.wave_vel * 0.58;
        v.wave_vel_R = v.wave_vel * 0.42;
        v.flux_strong = {0.03 * q, -0.02 * q, 0.01 * q};
        v.flux_weak = {-0.015 * q, 0.012 * q, 0.006 * q};
        v.latency = ((i % 29) == 0) ? 0.18 + 0.001 * (i % 7) : 0.0;

        if ((i % 31) == 0 || (i % 47) == 0) {
            v.state = (i % 2 == 0) ? 1 : -1;
            v.velocity = {0.025 * ((i % 3) - 1), 0.014 * ((i % 5) - 2),
                          0.009 * ((i % 7) - 3)};
            v.spin = (i % 3 == 0) ? 1 : -1;
            v.color = static_cast<std::int8_t>(i % 4);
            v.locked = (i % 5) == 0;
        }
    }
    rb.toggles.dual_substrate = true;
    rb.toggles.color_forces = true;
    rb.toggles.field_energy_gravity = true;
    rb.toggles.coulomb_charge_coupling = 0.73;
}

}  // namespace

int main() {
    constexpr int L = 12;

    // Direct CPU/GPU parity for the shared Rule-8 update, independent of the
    // CPU SOR vs CUDA FFT latency solvers.
    {
        constexpr int TL = 8;
        ftd::RenderBridge tau_cpu(TL);
        tau_cpu.force_cpu();
        tau_cpu.toggles.de_broglie_clock = true;
        tau_cpu.toggles.omega0 = 0.37;
        std::vector<ftd::Voxel> initial(static_cast<std::size_t>(TL * TL * TL));
        for (int k = 0; k < 3; ++k) {
            auto& v = initial[static_cast<std::size_t>(13 + k * 71)];
            v.state = (k == 1) ? -1 : 1;
            v.latency = 0.11 + 0.07 * k;
            v.velocity = {0.02 * k, -0.015 * (k + 1), 0.01};
            v.tau = 0.25 * k;
            v.phase = -0.2 * k;
        }
        tau_cpu.voxels() = initial;
        ftd::accumulate_proper_time(tau_cpu);

        ftd::gpu::GpuEngine tau_gpu(TL);
        tau_gpu.upload_from_host(initial);
        tau_gpu.accumulate_proper_time(true, tau_cpu.toggles.omega0);
        std::vector<ftd::Voxel> gpu_after;
        tau_gpu.sync_to_host(gpu_after);
        for (int k = 0; k < 3; ++k) {
            const std::size_t i = static_cast<std::size_t>(13 + k * 71);
            close("proper-time CPU/GPU site " + std::to_string(k),
                  tau_cpu.voxels()[i].tau, gpu_after[i].tau, 1e-13, 1e-13);
            close("de-Broglie phase CPU/GPU site " + std::to_string(k),
                  tau_cpu.voxels()[i].phase, gpu_after[i].phase, 1e-13, 1e-13);
        }
    }

    // The publisher contract is backend-neutral: CPU keeps an immediately
    // pollable compatibility snapshot rather than making server code branch on
    // CUDA availability. Its provenance is stamped at begin, not at poll.
    {
        ftd::RenderBridge cpu_snapshot(6);
        cpu_snapshot.force_cpu();
        populate(cpu_snapshot);
        cpu_snapshot.toggles.symplectic_leapfrog = true;
        cpu_snapshot.set_dt(0.5);
        ftd::TelemetrySnapshotRequest request;
        request.groups = ftd::TELEMETRY_DIAGNOSTICS | ftd::TELEMETRY_AUDIT;
        request.epoch = 700;
        check("CPU telemetry snapshot begins",
              cpu_snapshot.begin_telemetry_snapshot(request));
        check("CPU telemetry snapshot is immediately ready",
              cpu_snapshot.telemetry_snapshot_ready());
        ftd::TelemetrySnapshot snapshot;
        check("CPU telemetry snapshot polls", cpu_snapshot.poll_telemetry_snapshot(snapshot));
        check("CPU telemetry snapshot consumes once",
              !cpu_snapshot.telemetry_snapshot_ready()
              && !cpu_snapshot.poll_telemetry_snapshot(snapshot));
        check("CPU telemetry snapshot stamps source clock",
              snapshot.groups == request.groups && snapshot.epoch == 700
              && snapshot.diagnostics_meta.epoch == 700
              && snapshot.audit_meta.epoch == 700
              && snapshot.diagnostics_meta.dt == 0.5
              && snapshot.audit_meta.lattice_size == 6
              && snapshot.physical_time == 0.0);
    }

    ftd::RenderBridge cpu(L);
    cpu.force_cpu();
    ftd::RenderBridge gpu(L);
    gpu.set_interactive_gpu_mode(true);
    populate(cpu);
    populate(gpu);

    // Slice transport must read exactly L^2 magnitudes, not materialize the
    // old dense L^3 visual buffer. Check every axis plus periodic indices.
    ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
    ftd::gpu::g_gpu_full_voxel_download_calls = 0;
    for (int axis = 0; axis < 3; ++axis) {
        const int index = axis == 0 ? -1 : (axis == 1 ? L / 2 : L + 2);
        std::vector<float> cpu_plane, gpu_plane;
        cpu.copy_visual_flux_magnitude_plane(axis, index, cpu_plane);
        gpu.copy_visual_flux_magnitude_plane(axis, index, gpu_plane);
        check("flux plane size axis " + std::to_string(axis),
              cpu_plane.size() == static_cast<std::size_t>(L * L)
              && gpu_plane.size() == cpu_plane.size());
        bool equal = gpu_plane.size() == cpu_plane.size();
        for (std::size_t i = 0; equal && i < cpu_plane.size(); ++i)
            equal = std::abs(cpu_plane[i] - gpu_plane[i]) <= 1e-7f;
        check("flux plane CPU/GPU parity axis " + std::to_string(axis), equal);
    }
    check("flux plane avoids full voxel mirror",
          ftd::gpu::g_gpu_full_voxel_download_calls == 0
          && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);

    // Native clicks and bounded seed primitives mutate device memory directly.
    // None may round-trip the N^3 host shadow or invoke an N^3 upload.
    {
        ftd::RenderBridge inject_gpu(16);
        inject_gpu.set_interactive_gpu_mode(true);
        inject_gpu.toggles.disable_all();
        inject_gpu.toggles.dual_substrate = true;
        ftd::gpu::g_gpu_upload_bytes = 0;
        ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
        ftd::gpu::g_gpu_full_voxel_download_calls = 0;

        inject_gpu.inject_flux(1, 2, 3, {0.2, -0.3, 0.4});
        inject_gpu.inject_flux_add(1, 2, 3, {0.05, 0.1, -0.2});
        inject_gpu.inject_wave_vel_add(1, 2, 3, {-0.4, 0.2, 0.1});
        inject_gpu.inject_particle(4, 5, 6, 1, {0.11, 0.22, 0.33}, 1, 2);
        inject_gpu.inject_wavepacket(8, 8, 8, -1, 1.2, 0.2);
        inject_gpu.create_entangled_pair(10, 10, 10, {0.07, -0.04, 0.02});

        check("device injection primitives avoid lattice transfers",
              ftd::gpu::g_gpu_upload_bytes == 0
              && ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
        const auto field = inject_gpu.inspect_voxel(1, 2, 3);
        close("device flux set+add x", field.voxel.flux.x, 0.25, 1e-13, 1e-13);
        close("device flux set+add y", field.voxel.flux.y, -0.2, 1e-13, 1e-13);
        close("device wave-velocity add", field.voxel.wave_vel.x,
              -0.4, 1e-13, 1e-13);
        const auto particle = inject_gpu.inspect_voxel(4, 5, 6);
        check("device particle metadata",
              particle.voxel.state == 1 && particle.voxel.spin == 1
              && particle.voxel.color == 2 && particle.voxel.particle_id >= 0);
        const auto packet = inject_gpu.inspect_voxel(8, 8, 8);
        check("device wavepacket center", packet.voxel.state == -1
              && packet.voxel.particle_id >= 0);
        const auto entangled = inject_gpu.inspect_voxel(10, 10, 10);
        const auto partner = inject_gpu.inspect_voxel(11, 10, 10);
        check("device entangled pair metadata",
              entangled.voxel.state == 1 && partner.voxel.state == -1
              && entangled.voxel.pair_id >= 0
              && entangled.voxel.pair_id == partner.voxel.pair_id);
        check("compact validation after injection avoids full mirror",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
    }

    const ftd::Diagnostics d_cpu = cpu.diagnostics();
    const ftd::EnergyAudit e_cpu = cpu.energy_audit();
    const ftd::GravityMetricAgg g_cpu = cpu.gravity_metric_agg();
    const ftd::LagrangianDiag l_cpu = ftd::compute_lagrangian_diagnostics(cpu);
    const ftd::VoxelInspection v_cpu = cpu.inspect_voxel(0, 0, 0);
    const ftd::ForceDiag f_cpu = cpu.inspect_force(0, 0, 0);

    ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
    ftd::gpu::g_gpu_full_voxel_download_calls = 0;
    ftd::gpu::g_gpu_compact_diagnostic_download_bytes = 0;

    const ftd::Diagnostics d_gpu = gpu.diagnostics();
    const ftd::EnergyAudit e_gpu = gpu.energy_audit();
    const ftd::GravityMetricAgg g_gpu = gpu.gravity_metric_agg();
    const ftd::LagrangianDiag l_gpu = ftd::compute_lagrangian_diagnostics(gpu);
    const ftd::VoxelInspection v_gpu = gpu.inspect_voxel(0, 0, 0);
    const ftd::ForceDiag f_gpu = gpu.inspect_force(0, 0, 0);

    check("compact requests avoid full voxel mirror",
          ftd::gpu::g_gpu_full_voxel_download_calls == 0
          && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
    // Diagnostics=25, energy=24, gravity=4, Lagrangian=15, voxel=23,
    // force=15 doubles.
    check("compact readback is fixed scalar payload",
          ftd::gpu::g_gpu_compact_diagnostic_download_bytes
              == static_cast<std::size_t>(25 + 24 + 4 + 15 + 23 + 15)
                   * sizeof(double));

    close("diagnostics total flux", d_cpu.total_flux, d_gpu.total_flux);
    close("diagnostics BI energy", d_cpu.total_energy, d_gpu.total_energy);
    close("diagnostics entropy", d_cpu.total_entropy, d_gpu.total_entropy);
    close("diagnostics bandwidth", d_cpu.max_bandwidth, d_gpu.max_bandwidth);
    close("diagnostics budget", d_cpu.max_causal_budget, d_gpu.max_causal_budget);
    check("diagnostics manifested", d_cpu.manifested_count == d_gpu.manifested_count);
    check("diagnostics charge signs", d_cpu.positive_count == d_gpu.positive_count
          && d_cpu.negative_count == d_gpu.negative_count);
    check("diagnostics spin", d_cpu.spin_up_count == d_gpu.spin_up_count
          && d_cpu.spin_down_count == d_gpu.spin_down_count);
    for (int c = 0; c < 4; ++c)
        check("diagnostics color " + std::to_string(c),
              d_cpu.color_count[c] == d_gpu.color_count[c]);
    close("diagnostics angular x", d_cpu.total_angular_momentum.x,
          d_gpu.total_angular_momentum.x);
    close("diagnostics angular y", d_cpu.total_angular_momentum.y,
          d_gpu.total_angular_momentum.y);
    close("diagnostics angular z", d_cpu.total_angular_momentum.z,
          d_gpu.total_angular_momentum.z);

    close("energy field", e_cpu.field_energy, e_gpu.field_energy);
    close("energy wave", e_cpu.wave_energy, e_gpu.wave_energy);
    close("energy kinetic", e_cpu.particle_ke, e_gpu.particle_ke);
    close("energy rest", e_cpu.particle_rest_energy, e_gpu.particle_rest_energy);
    close("energy left", e_cpu.E_L_total, e_gpu.E_L_total);
    close("energy right", e_cpu.E_R_total, e_gpu.E_R_total);
    close("energy wave left", e_cpu.wv_L_total, e_gpu.wv_L_total);
    close("energy wave right", e_cpu.wv_R_total, e_gpu.wv_R_total);
    close("energy chirality", e_cpu.chirality_total, e_gpu.chirality_total);
    close("energy strong", e_cpu.strong_energy, e_gpu.strong_energy);
    close("energy weak", e_cpu.weak_energy, e_gpu.weak_energy);
    close("energy electric", e_cpu.E_field_energy, e_gpu.E_field_energy);
    close("energy magnetic", e_cpu.B_field_energy, e_gpu.B_field_energy);
    close("energy gauss", e_cpu.gauss_violation, e_gpu.gauss_violation);
    close("energy gauss max", e_cpu.max_gauss_error, e_gpu.max_gauss_error);
    close("energy poynting x", e_cpu.total_poynting.x, e_gpu.total_poynting.x);
    close("energy poynting y", e_cpu.total_poynting.y, e_gpu.total_poynting.y);
    close("energy poynting z", e_cpu.total_poynting.z, e_gpu.total_poynting.z);
    check("energy counts", e_cpu.manifested_count == e_gpu.manifested_count
          && e_cpu.charge_total == e_gpu.charge_total);

    close("gravity max", g_cpu.latency_max, g_gpu.latency_max);
    close("gravity mean", g_cpu.latency_mean, g_gpu.latency_mean);
    close("gravity gamma", g_cpu.gamma_max, g_gpu.gamma_max);
    close("gravity dilation", g_cpu.dilation_max_pct, g_gpu.dilation_max_pct);
    check("gravity count", g_cpu.voxel_count == g_gpu.voxel_count);

    close("lagrangian kinetic", l_cpu.field_kinetic_sum, l_gpu.field_kinetic_sum);
    close("lagrangian gradient", l_cpu.field_gradient_sum, l_gpu.field_gradient_sum);
    close("lagrangian BI", l_cpu.born_infeld_sum, l_gpu.born_infeld_sum);
    close("lagrangian coupling", l_cpu.coupling_sum, l_gpu.coupling_sum);
    close("lagrangian velocity coupling", l_cpu.velocity_coupling_sum,
          l_gpu.velocity_coupling_sum);
    close("lagrangian gauss", l_cpu.gauss_sum, l_gpu.gauss_sum);
    close("lagrangian dissipation", l_cpu.dissipation_sum, l_gpu.dissipation_sum);
    close("lagrangian total", l_cpu.total_lagrangian, l_gpu.total_lagrangian);
    close("lagrangian Hamiltonian", l_cpu.total_hamiltonian, l_gpu.total_hamiltonian);
    close("lagrangian gauss residual", l_cpu.gauss_violation, l_gpu.gauss_violation);
    close("lagrangian max residual", l_cpu.max_gauss_error, l_gpu.max_gauss_error);
    check("lagrangian counts", l_cpu.manifested_count == l_gpu.manifested_count
          && l_cpu.locked_count == l_gpu.locked_count);

    check("voxel state", v_cpu.voxel.state == v_gpu.voxel.state);
    check("voxel identity", v_cpu.voxel.particle_id == v_gpu.voxel.particle_id
          && v_cpu.voxel.pair_id == v_gpu.voxel.pair_id);
    close("voxel flux x", v_cpu.voxel.flux.x, v_gpu.voxel.flux.x);
    close("voxel wave y", v_cpu.voxel.wave_vel.y, v_gpu.voxel.wave_vel.y);
    close("voxel velocity z", v_cpu.voxel.velocity.z, v_gpu.voxel.velocity.z);
    close("voxel latency", v_cpu.voxel.latency, v_gpu.voxel.latency);
    close("voxel phase", v_cpu.voxel.phase, v_gpu.voxel.phase);
    close("voxel divergence", v_cpu.divergence, v_gpu.divergence);
    close("voxel curl x", v_cpu.curl.x, v_gpu.curl.x);
    close("voxel curl y", v_cpu.curl.y, v_gpu.curl.y);
    close("voxel curl z", v_cpu.curl.z, v_gpu.curl.z);

    close("force Coulomb", f_cpu.f_coulomb.mag(), f_gpu.f_coulomb.mag());
    close("force strong", f_cpu.f_strong.mag(), f_gpu.f_strong.mag());
    close("force magnetic", f_cpu.f_magnetic.mag(), f_gpu.f_magnetic.mag());
    close("force gravity", f_cpu.f_gravity.mag(), f_gpu.f_gravity.mag());
    close("force exchange", f_cpu.f_exchange.mag(), f_gpu.f_exchange.mag());

    // A native publisher observes one GPU epoch through a fence-backed
    // snapshot, rather than issuing four synchronous public getters. The
    // engine may queue its next tick immediately after begin(); the captured
    // scalar result must remain tied to the pre-tick epoch and must never
    // materialize the canonical AoS mirror.
    {
        ftd::gpu::GpuEngine snapshot_gpu(L);
        snapshot_gpu.toggles = gpu.toggles;
        snapshot_gpu.upload_from_host(
            static_cast<const ftd::RenderBridge&>(gpu).voxels());

        ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
        ftd::gpu::g_gpu_full_voxel_download_calls = 0;
        ftd::gpu::g_gpu_telemetry_snapshot_download_bytes = 0;
        ftd::gpu::g_gpu_telemetry_snapshot_launches = 0;

        ftd::TelemetrySnapshotRequest request;
        request.groups = ftd::TELEMETRY_ALL;
        request.epoch = 701;
        request.physical_time = 12.5;
        request.dt = 0.25;
        request.lattice_size = L;
        check("telemetry snapshot accepts first epoch",
              snapshot_gpu.begin_telemetry_snapshot(request));
        check("telemetry snapshot rejects overlapping epoch",
              !snapshot_gpu.begin_telemetry_snapshot(request));

        // This is intentionally issued before polling the snapshot. The GPU
        // default stream preserves snapshot -> tick order, while the host does
        // not wait at begin_telemetry_snapshot().
        snapshot_gpu.toggles.disable_all();
        snapshot_gpu.tick();

        ftd::TelemetrySnapshot snapshot;
        bool polled = false;
        for (int attempt = 0; attempt < 1000 && !polled; ++attempt) {
            polled = snapshot_gpu.poll_telemetry_snapshot(snapshot);
            if (!polled) std::this_thread::yield();
        }
        if (!polled) snapshot_gpu.wait_telemetry_snapshot(snapshot);
        check("telemetry snapshot poll eventually completes", polled
              || snapshot.groups == ftd::TELEMETRY_ALL);
        check("telemetry snapshot groups are exact",
              snapshot.groups == ftd::TELEMETRY_ALL);
        check("telemetry snapshot captures pre-queued-tick epoch",
              snapshot.tick == 0 && snapshot.diagnostics.tick == 0);
        check("telemetry snapshot metadata is coherent",
              snapshot.epoch == 701 && snapshot.state_version > 0
              && snapshot.physical_time == 12.5 && snapshot.dt == 0.25
              && snapshot.lattice_size == L
              && snapshot.diagnostics_meta.epoch == 701
              && snapshot.audit_meta.epoch == 701
              && snapshot.gravity_meta.epoch == 701
              && snapshot.lagrangian_meta.epoch == 701
              && snapshot.audit_meta.state_version == snapshot.state_version
              && snapshot.lagrangian_meta.tick == snapshot.tick);
        check("telemetry snapshot avoids AoS mirror",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
        check("telemetry snapshot has one fixed async readback",
              ftd::gpu::g_gpu_telemetry_snapshot_launches == 1
              && ftd::gpu::g_gpu_telemetry_snapshot_download_bytes
                  == static_cast<std::size_t>(68) * sizeof(double));
        close("snapshot diagnostics parity", snapshot.diagnostics.total_flux,
              d_gpu.total_flux);
        close("snapshot audit parity", snapshot.audit.field_energy,
              e_gpu.field_energy);
        close("snapshot gravity parity", snapshot.gravity.latency_max,
              g_gpu.latency_max);
        close("snapshot Lagrangian parity", snapshot.lagrangian.total_lagrangian,
              l_gpu.total_lagrangian);

        // A fresh, diagnostics-only request gets a new state version after a
        // direct GPU mutation and does not claim stale groups were refreshed.
        snapshot_gpu.inject_flux_add(1, 2, 3, {0.01, 0.0, 0.0});
        ftd::TelemetrySnapshotRequest fast;
        fast.groups = ftd::TELEMETRY_DIAGNOSTICS;
        fast.epoch = 702;
        check("telemetry fast group begins",
              snapshot_gpu.begin_telemetry_snapshot(fast));
        ftd::TelemetrySnapshot fast_snapshot;
        snapshot_gpu.wait_telemetry_snapshot(fast_snapshot);
        check("telemetry fast group preserves mask and new version",
              fast_snapshot.groups == ftd::TELEMETRY_DIAGNOSTICS
              && fast_snapshot.diagnostics_meta.epoch == 702
              && fast_snapshot.audit_meta.epoch == 0
              && fast_snapshot.state_version > snapshot.state_version);
        check("telemetry cached poll stays compact",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
    }

    // A real interactive latency tick must remain fully device-resident.
    // Reading its one-site result uses the compact inspector, not a mirror.
    {
        ftd::RenderBridge latency_gpu(L);
        latency_gpu.set_interactive_gpu_mode(true);
        latency_gpu.toggles.disable_all();
        latency_gpu.toggles.gravity = true;
        latency_gpu.toggles.latency_field = true;
        const int c = L / 2;
        auto& center = latency_gpu.voxel_at(c, c, c);
        center.state = 1;
        center.particle_id = 1;
        center.velocity = {0.01, 0.0, 0.0};

        ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
        ftd::gpu::g_gpu_full_voxel_download_calls = 0;
        ftd::gpu::g_gpu_compact_diagnostic_download_bytes = 0;
        latency_gpu.tick();
        check("latency tick avoids full voxel mirror",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
        const auto center_after = latency_gpu.inspect_voxel(c, c, c);
        check("latency tick accumulates tau on device", center_after.voxel.tau > 0.0);
        check("latency inspection remains compact",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_compact_diagnostic_download_bytes
                   == static_cast<std::size_t>(23) * sizeof(double));
    }

    // The de-Broglie clock's tau/phase pair must remain device-resident too.
    // The former host post-pass mirrored and re-uploaded every voxel per tick.
    {
        ftd::RenderBridge clock_gpu(L);
        clock_gpu.set_interactive_gpu_mode(true);
        clock_gpu.toggles.disable_all();
        clock_gpu.toggles.de_broglie_clock = true;
        clock_gpu.toggles.omega0 = 0.41;
        const int c = L / 2;
        auto& center = clock_gpu.voxel_at(c, c, c);
        center.state = 1;
        center.particle_id = 2;

        ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
        ftd::gpu::g_gpu_full_voxel_download_calls = 0;
        ftd::gpu::g_gpu_compact_diagnostic_download_bytes = 0;
        clock_gpu.tick();
        check("de-Broglie tick avoids full voxel mirror",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
        const auto center_after = clock_gpu.inspect_voxel(c, c, c);
        close("de-Broglie tau advances on device", center_after.voxel.tau,
              1.0, 1e-13, 1e-13);
        close("de-Broglie phase advances on device", center_after.voxel.phase,
              0.41, 1e-13, 1e-13);
        check("de-Broglie inspection remains compact",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_compact_diagnostic_download_bytes
                   == static_cast<std::size_t>(23) * sizeof(double));
    }

    // Host-only per-tick extensions are rejected explicitly once an
    // interactive lattice crosses the supported L=64 boundary. This is a
    // pre-tick capability error, not a multi-hundred-MiB surprise transfer.
    {
        ftd::RenderBridge gated_gpu(65);
        gated_gpu.set_interactive_gpu_mode(true);
        gated_gpu.toggles.disable_all();
        ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
        ftd::gpu::g_gpu_full_voxel_download_calls = 0;

        gated_gpu.toggles.knot_tracking = true;
        bool knot_rejected = false;
        try { gated_gpu.tick(); }
        catch (const std::logic_error& e) {
            knot_rejected = std::string(e.what()).find("knot_tracking")
                          != std::string::npos;
        }
        check("large interactive knot tracking is rejected explicitly",
              knot_rejected);

        gated_gpu.toggles.knot_tracking = false;
        gated_gpu.toggles.forces = true;
        gated_gpu.toggles.cluster_inertia = true;
        bool cluster_ok = true;
        try { gated_gpu.tick(); }
        catch (const std::logic_error&) {
            cluster_ok = false;
        }
        check("large interactive cluster inertia is native CUDA",
              cluster_ok);
        check("cluster inertia does not force a full voxel mirror",
              ftd::gpu::g_gpu_full_voxel_download_calls == 0
              && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
    }

    if (failures == 0) {
        std::cout << "PASS: compact CUDA diagnostics preserve CPU semantics "
                     "(848-byte full diagnostic batch; 184-byte probes) "
                     "with zero full-mirror downloads\n";
    }
    return failures == 0 ? 0 : 1;
}
