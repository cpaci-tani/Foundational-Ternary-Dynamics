/**
 * GPU term-completeness oracle.
 *
 * Pins live implementation class for every TOGGLE_SPECS row against the
 * declared ToggleBackend mask. Fails closed when ANY (or GPU) is claimed
 * for a CPU-only, intent-flag, or host-mirror term, and when a NativeCuda
 * row drops the GPU bit.
 *
 * This is characterization + contract, not a derivation. Ports update
 * gpu_term_contract.h in the same change as the kernel.
 */

#include "ftd/gpu_term_contract.h"
#include "ftd/term_toggles.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace {

int passed = 0;
int failed = 0;

void check(const char* name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
    ok ? ++passed : ++failed;
}

bool declares_gpu(std::uint8_t backends) {
    return (backends & ftd::ToggleBackend::GPU) != 0;
}

}  // namespace

int main() {
    using ftd::GpuTermImpl;
    using ftd::GPU_TERM_CONTRACT;
    using ftd::TOGGLE_SPECS;
    using ftd::TermToggles;
    using ftd::gpu_term_contract_count;
    using ftd::toggle_spec_count;

    std::printf("GPU term contract\n");

    check("TOGGLE_SPECS and GPU_TERM_CONTRACT have the same length",
          toggle_spec_count() == gpu_term_contract_count());

    bool names_match = true;
    bool backends_match = true;
    int native_without_gpu = 0;
    int cpu_with_gpu = 0;
    int gpu_only_without_warning = 0;
    int intent_flags = 0;

    const std::size_t n = toggle_spec_count();
    const std::size_t m = gpu_term_contract_count();
    const std::size_t k = n < m ? n : m;
    for (std::size_t i = 0; i < k; ++i) {
        if (std::strcmp(TOGGLE_SPECS[i].name, GPU_TERM_CONTRACT[i].name) != 0)
            names_match = false;
        if (TOGGLE_SPECS[i].backends != GPU_TERM_CONTRACT[i].declared_backends)
            backends_match = false;

        const auto impl = GPU_TERM_CONTRACT[i].impl;
        const auto backends = GPU_TERM_CONTRACT[i].declared_backends;
        if ((impl == GpuTermImpl::NativeCuda
             || impl == GpuTermImpl::GpuOnlyNoCpu)
            && !declares_gpu(backends)) {
            ++native_without_gpu;
        }
        if ((impl == GpuTermImpl::CpuOnly
             || impl == GpuTermImpl::CpuFallbackSync)
            && declares_gpu(backends)) {
            ++cpu_with_gpu;
        }
        if (impl == GpuTermImpl::GpuOnlyNoCpu) {
            const char* warning = TOGGLE_SPECS[i].gpu_only_warning;
            if (!warning || !*warning) ++gpu_only_without_warning;
        }
        if (impl == GpuTermImpl::IntentFlag) ++intent_flags;
    }

    check("contract names match TOGGLE_SPECS in order", names_match);
    check("contract declared_backends match TOGGLE_SPECS.backends",
          backends_match);
    check("NativeCuda / GpuOnlyNoCpu rows declare the GPU bit",
          native_without_gpu == 0);
    check("CpuOnly / CpuFallbackSync rows do not declare the GPU bit",
          cpu_with_gpu == 0);
    check("GpuOnlyNoCpu rows carry a gpu_only_warning",
          gpu_only_without_warning == 0);
    check("no IntentFlag rows remain", intent_flags == 0);

    const auto* langevin = ftd::find_gpu_term_contract("langevin");
    check("langevin is classified NativeCuda",
          langevin && langevin->impl == GpuTermImpl::NativeCuda);
    const auto* langevin_spec = ftd::term_toggles_detail::find_spec("langevin");
    check("langevin description does not claim CPU-only runtime",
          langevin_spec
          && std::string_view(langevin_spec->description).find("CPU only")
                 == std::string_view::npos);

    const auto* confinement = ftd::find_gpu_term_contract("confinement");
    check("confinement is classified NativeCuda",
          confinement && confinement->impl == GpuTermImpl::NativeCuda);

    std::string error;
    TermToggles confinement_on;
    confinement_on.disable_all();
    confinement_on.color_forces = true;
    confinement_on.confinement = true;
    check("GPU validator accepts confinement with color_forces",
          confinement_on.validate_backend(ftd::ToggleBackend::GPU, true, &error));

    for (const char* name : {"knot_tracking"}) {
        const auto* row = ftd::find_gpu_term_contract(name);
        check((std::string(name) + " is HostMirrorHybrid").c_str(),
              row && row->impl == GpuTermImpl::HostMirrorHybrid);
        TermToggles staged;
        staged.disable_all();
        staged.*(ftd::term_toggles_detail::find_spec(name)->field) = true;
        error.clear();
        check((std::string(name)
               + " is rejected by device-resident GPU validator").c_str(),
              !staged.validate_backend(ftd::ToggleBackend::GPU, true, &error));
    }

    for (const char* name : {"strong_force", "exchange_force", "cluster_inertia"}) {
        const auto* row = ftd::find_gpu_term_contract(name);
        check((std::string(name) + " is classified NativeCuda").c_str(),
              row && row->impl == GpuTermImpl::NativeCuda);
        TermToggles staged;
        staged.disable_all();
        staged.*(ftd::term_toggles_detail::find_spec(name)->field) = true;
        if (std::string_view(name) == "exchange_force")
            staged.poisson_coulomb = true;
        if (std::string_view(name) == "cluster_inertia")
            staged.color_forces = true;
        error.clear();
        check((std::string(name) + " is accepted by GPU validator").c_str(),
              staged.validate_backend(ftd::ToggleBackend::GPU, true, &error));
    }

    {
        TermToggles colour_cluster;
        colour_cluster.disable_all();
        colour_cluster.color_forces = true;
        colour_cluster.cluster_inertia = true;
        error.clear();
        check("cluster_inertia validates with color_forces and forces=false",
              colour_cluster.validate(&error));
        TermToggles cluster_only;
        cluster_only.disable_all();
        cluster_only.cluster_inertia = true;
        error.clear();
        check("cluster_inertia without a force channel fails closed",
              !cluster_only.validate(&error));
    }

    for (const char* name : {"strong_stress_energy", "matched_gauss_dynamics"}) {
        const auto* row = ftd::find_gpu_term_contract(name);
        check((std::string(name) + " is classified NativeCuda").c_str(),
              row && row->impl == GpuTermImpl::NativeCuda);
        TermToggles staged;
        staged.disable_all();
        staged.*(ftd::term_toggles_detail::find_spec(name)->field) = true;
        if (std::string_view(name) == "strong_stress_energy")
            staged.color_forces = true;
        error.clear();
        check((std::string(name) + " is accepted by GPU validator").c_str(),
              staged.validate_backend(ftd::ToggleBackend::GPU, true, &error));
    }

    const auto* verlet = ftd::find_gpu_term_contract("verlet_wave_integrator");
    check("verlet_wave_integrator is classified NativeCuda",
          verlet && verlet->impl == GpuTermImpl::NativeCuda);
    TermToggles verlet_on;
    verlet_on.disable_all();
    verlet_on.verlet_wave_integrator = true;
    error.clear();
    check("GPU validator accepts verlet_wave_integrator",
          verlet_on.validate_backend(ftd::ToggleBackend::GPU, true, &error));

    for (const char* name : {"lorentz_period2_floquet", "lorentz_bcc_time_floquet"}) {
        const auto* row = ftd::find_gpu_term_contract(name);
        check((std::string(name) + " is classified NativeCuda").c_str(),
              row && row->impl == GpuTermImpl::NativeCuda);
        TermToggles staged;
        staged.disable_all();
        staged.*(ftd::term_toggles_detail::find_spec(name)->field) = true;
        error.clear();
        check((std::string(name) + " is accepted by GPU validator").c_str(),
              staged.validate_backend(ftd::ToggleBackend::GPU, true, &error));
    }

    const auto* smo = ftd::find_gpu_term_contract("symmetric_movement_order");
    check("symmetric_movement_order is classified NativeCuda",
          smo && smo->impl == GpuTermImpl::NativeCuda);
    TermToggles smo_on;
    smo_on.disable_all();
    smo_on.movement = true;
    smo_on.symmetric_movement_order = true;
    error.clear();
    check("GPU validator accepts symmetric_movement_order",
          smo_on.validate_backend(ftd::ToggleBackend::GPU, true, &error));

    const auto* control = ftd::find_gpu_term_contract("strict_validation");
    check("strict_validation is ControlOnly",
          control && control->impl == GpuTermImpl::ControlOnly);

    const auto* geo = ftd::find_gpu_term_contract("geometric_gravity");
    check("geometric_gravity is classified NativeCuda",
          geo && geo->impl == GpuTermImpl::NativeCuda);
    TermToggles geo_on;
    geo_on.disable_all();
    geo_on.forces = true;
    geo_on.gravity = true;
    geo_on.geometric_gravity = true;
    error.clear();
    check("GPU validator accepts geometric_gravity",
          geo_on.validate_backend(ftd::ToggleBackend::GPU, true, &error));
    error.clear();
    check("CPU validator accepts geometric_gravity",
          geo_on.validate_backend(ftd::ToggleBackend::CPU, false, &error));

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
