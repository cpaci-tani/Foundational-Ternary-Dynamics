/**
 * Focused CUDA qualification for the Scale-0 `empty` canonical digest hook.
 *
 * This target proves the digest is computed from resident SoA fields in both
 * ordinary and interactive GPU modes, carries exact provenance/counters, and
 * does not hide a full voxel mirror behind the observation API.
 */

#include "ftd/backend.h"
#include "ftd/dynamical_state_digest.h"
#include "ftd/gpu_buffers.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/test_telemetry.h"

#include <cuda_runtime_api.h>

#include <array>
#include <cstdint>
#include <limits>
#include <string>

namespace {

bool require_gpu(ftd::RenderBridge& bridge) {
    if (bridge.backend_kind() == ftd::Backend::Kind::Gpu
        && bridge.gpu_engine_ptr() != nullptr) {
        return true;
    }
    ftd::test::check("native CUDA backend is active", false,
                     "configure through WSL2 with FTD_ENABLE_CUDA=ON");
    return false;
}

bool same_hash(const ftd::DynamicalStateDigest& lhs,
               const ftd::DynamicalStateDigest& rhs) {
    return lhs.hash_lo == rhs.hash_lo && lhs.hash_hi == rhs.hash_hi;
}

bool copy_to_device(void* destination, const void* source, std::size_t bytes) {
    return cudaMemcpy(destination, source, bytes, cudaMemcpyHostToDevice)
        == cudaSuccess;
}

ftd::DynamicalStateDigest capture(ftd::RenderBridge& bridge,
                                  const std::string& label) {
    ftd::DynamicalStateDigest digest{};
    ftd::test::check((label + " capture succeeds").c_str(),
                     bridge.capture_dynamical_state_digest(digest));
    return digest;
}

void check_no_full_mirror(ftd::RenderBridge& bridge,
                          const std::string& label) {
    const std::size_t full_bytes_before =
        ftd::gpu::g_gpu_full_voxel_download_bytes;
    const std::size_t full_calls_before =
        ftd::gpu::g_gpu_full_voxel_download_calls;
    const std::size_t digest_bytes_before =
        ftd::gpu::g_gpu_dynamical_digest_download_bytes;
    const std::size_t digest_calls_before =
        ftd::gpu::g_gpu_dynamical_digest_download_calls;

    const auto digest = capture(bridge, label);
    ftd::test::check((label + " transfers one fixed accumulator").c_str(),
        digest.device_to_host_bytes
            == sizeof(ftd::DynamicalStateDigestAccumulator)
        && ftd::gpu::g_gpu_dynamical_digest_download_bytes
            == digest_bytes_before
                + sizeof(ftd::DynamicalStateDigestAccumulator)
        && ftd::gpu::g_gpu_dynamical_digest_download_calls
            == digest_calls_before + 1);
    ftd::test::check((label + " does not materialize voxel mirror").c_str(),
        ftd::gpu::g_gpu_full_voxel_download_bytes == full_bytes_before
        && ftd::gpu::g_gpu_full_voxel_download_calls == full_calls_before);
}

void configure_empty(ftd::RenderBridge& bridge,
                     ftd::FluxBoundaryMode boundary,
                     bool interactive) {
    ftd::test::check("empty scenario dispatch succeeds",
                     ftd::dispatch_scenario(bridge, "empty"));
    bridge.toggles.flux_boundary = boundary;
    bridge.set_interactive_gpu_mode(interactive);
    std::string error;
    ftd::test::check("empty GPU profile validates",
                     bridge.toggles.validate(&error), error.c_str());
}

void run_mode_boundary_matrix() {
    ftd::test::section(
        "GPU-resident empty digest: ordinary/interactive and boundary matrix");
    constexpr std::array<ftd::FluxBoundaryMode, 3> boundaries = {
        ftd::FluxBoundaryMode::Periodic,
        ftd::FluxBoundaryMode::Reflective,
        ftd::FluxBoundaryMode::Dispersal,
    };

    for (bool interactive : {false, true}) {
        for (std::size_t boundary_index = 0;
             boundary_index < boundaries.size(); ++boundary_index) {
            ftd::RenderBridge bridge(interactive ? 17 : 8);
            if (!require_gpu(bridge)) return;
            configure_empty(bridge, boundaries[boundary_index], interactive);

            const std::string prefix = std::string(interactive
                ? "interactive" : "ordinary") + " boundary="
                + std::to_string(boundary_index);
            const auto initial = capture(bridge, prefix + " tick=0");
            ftd::test::check((prefix + " initial record is exact default").c_str(),
                initial.schema_version == ftd::DYNAMICAL_STATE_DIGEST_SCHEMA
                && initial.lattice_size == bridge.lattice().size()
                && initial.site_count == bridge.lattice().total_sites()
                && initial.tick == 0
                && initial.exact_default_record());

            bridge.run(64);
            const auto after = capture(bridge, prefix + " tick=64");
            ftd::test::check((prefix + " provenance is exact").c_str(),
                after.lattice_size == bridge.lattice().size()
                && after.site_count == bridge.lattice().total_sites()
                && after.tick == bridge.current_tick()
                && after.tick == bridge.gpu_engine_ptr()->device_tick()
                && after.tick == 64
                && after.state_version
                    == bridge.gpu_engine_ptr()->state_version()
                && after.state_version > initial.state_version);
            ftd::test::check((prefix + " null digest is invariant").c_str(),
                same_hash(initial, after) && after.exact_default_record());

            check_no_full_mirror(bridge, prefix + " resident request");
        }
    }
}

void run_schema_boundary_probes() {
    ftd::test::section("Schema inclusion/exclusion and exact counters on CUDA");
    ftd::RenderBridge bridge(8);
    if (!require_gpu(bridge)) return;
    configure_empty(bridge, ftd::FluxBoundaryMode::Periodic, true);
    auto* engine = bridge.gpu_engine_ptr();
    const auto& buffers = engine->bufs();

    const auto baseline = capture(bridge, "schema baseline");
    const double negative_zero = -0.0;
    ftd::test::check("signed zero write reaches included device field",
        copy_to_device(buffers.d_flux_x, &negative_zero,
                       sizeof(negative_zero)));
    const auto signed_zero = capture(bridge, "signed-zero probe");
    ftd::test::check("signed zero is canonicalized",
        same_hash(baseline, signed_zero)
        && signed_zero.exact_default_record());

    // These are deliberately direct test writes to excluded device buffers.
    // No tick follows, so they cannot influence physics; the purpose is to pin
    // the schema boundary without adding public mutation APIs for scratch.
    const double excluded_double = 7.0;
    const std::int32_t excluded_id = 19;
    ftd::test::check("excluded device probes are written",
        copy_to_device(buffers.d_delta_j_x, &excluded_double,
                       sizeof(excluded_double))
        && copy_to_device(buffers.d_tau, &excluded_double,
                          sizeof(excluded_double))
        && copy_to_device(buffers.d_phase, &excluded_double,
                          sizeof(excluded_double))
        && copy_to_device(buffers.d_phi, &excluded_double,
                          sizeof(excluded_double))
        && copy_to_device(buffers.d_particle_id, &excluded_id,
                          sizeof(excluded_id))
        && copy_to_device(buffers.d_pair_id, &excluded_id,
                          sizeof(excluded_id)));
    const auto excluded = capture(bridge, "excluded-buffer probe");
    ftd::test::check(
        "delta_j, clocks, identities, and Gauss scratch are excluded",
        same_hash(baseline, excluded) && excluded.exact_default_record());

    const double one = 1.0;
    ftd::test::check("included nondefault write reaches device",
        copy_to_device(buffers.d_flux_x, &one, sizeof(one)));
    const auto included = capture(bridge, "included-field probe");
    ftd::test::check("included value changes hash and exact count",
        !same_hash(baseline, included)
        && included.nondefault_value_count == 1
        && included.nonfinite_value_count == 0);

    const double nan = std::numeric_limits<double>::quiet_NaN();
    ftd::test::check("nonfinite write reaches device",
        copy_to_device(buffers.d_flux_x, &nan, sizeof(nan)));
    const auto nonfinite = capture(bridge, "nonfinite probe");
    ftd::test::check("nonfinite and nondefault counters are exact",
        nonfinite.nonfinite_value_count == 1
        && nonfinite.nondefault_value_count == 1
        && !nonfinite.exact_default_record());

    const double zero = 0.0;
    const double coulomb = 0.125;
    const double latency_potential = -0.25;
    ftd::test::check("persistent potential probes reach device",
        copy_to_device(buffers.d_flux_x, &zero, sizeof(zero))
        && copy_to_device(buffers.d_phi_coulomb, &coulomb,
                          sizeof(coulomb))
        && copy_to_device(buffers.d_phi_latency, &latency_potential,
                          sizeof(latency_potential)));
    const auto potentials = capture(bridge, "persistent-potential probe");
    ftd::test::check("Coulomb and latency potentials are included exactly",
        !same_hash(baseline, potentials)
        && potentials.nonfinite_value_count == 0
        && potentials.nondefault_value_count == 2);
}

void populate_shared_probe(ftd::RenderBridge& bridge) {
    auto& voxel = bridge.voxel_at(1, 2, 3);
    voxel.state = -1;
    voxel.flux = {0.25, -0.5, 0.75};
    voxel.wave_vel = {-0.125, 0.0625, 0.03125};
    voxel.flux_L = {0.2, -0.3, 0.4};
    voxel.flux_R = {0.05, -0.2, 0.35};
    voxel.wave_vel_L = {-0.1, 0.04, 0.02};
    voxel.wave_vel_R = {-0.025, 0.0225, 0.01125};
    voxel.velocity = {0.01, -0.02, 0.03};
    voxel.remainder = {0.4, 0.5, 0.6};
    voxel.latency = 0.15;
    voxel.locked = true;
    voxel.spin = -1;
    voxel.color = 3;
    voxel.flavor = 2;
    voxel.accel_mag = 0.04;
    voxel.flux_strong = {0.03, -0.02, 0.01};
    voxel.wave_vel_strong = {-0.01, 0.02, -0.03};
    voxel.flux_weak = {0.005, 0.006, -0.007};
    voxel.wave_vel_weak = {-0.008, 0.009, 0.01};
    // Excluded fields intentionally differ between the two probes below.
    voxel.tau = 9.0;
    voxel.phase = 10.0;
    voxel.particle_id = 101;
    voxel.pair_id = 202;
}

void run_cpu_gpu_shared_contract_parity() {
    ftd::test::section("Shared host/device schema parity");
    ftd::RenderBridge cpu(8);
    cpu.force_cpu();
    ftd::RenderBridge gpu(8);
    if (!require_gpu(gpu)) return;
    configure_empty(cpu, ftd::FluxBoundaryMode::Periodic, false);
    configure_empty(gpu, ftd::FluxBoundaryMode::Periodic, true);
    populate_shared_probe(cpu);
    populate_shared_probe(gpu);
    // Excluded clock/identity values are not required to match.
    cpu.voxel_at(1, 2, 3).tau = -3.0;
    cpu.voxel_at(1, 2, 3).particle_id = 777;

    const auto cpu_digest = capture(cpu, "CPU shared probe");
    const auto gpu_digest = capture(gpu, "GPU shared probe");
    ftd::test::check("CPU/GPU named-field hashes match exactly",
        same_hash(cpu_digest, gpu_digest)
        && cpu_digest.nonfinite_value_count
            == gpu_digest.nonfinite_value_count
        && cpu_digest.nondefault_value_count
            == gpu_digest.nondefault_value_count);
    ftd::test::check("backend transfer instrumentation is explicit",
        cpu_digest.device_to_host_bytes == 0
        && gpu_digest.device_to_host_bytes
            == sizeof(ftd::DynamicalStateDigestAccumulator));
}

}  // namespace

int main() {
    static_assert(sizeof(ftd::DynamicalStateDigestAccumulator) == 32,
                  "digest transfer contract changed; bump schema/evidence");
    ftd::test::init("test_empty_scenario_qualification_gpu");
    run_mode_boundary_matrix();
    run_schema_boundary_probes();
    run_cpu_gpu_shared_contract_parity();
    return ftd::test::finalize();
}
