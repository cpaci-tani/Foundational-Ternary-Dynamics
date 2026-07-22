#!/usr/bin/env python3
"""Exact source-contract audit for lattice-c and mass/energy normalization.

FTD-0401 is a static implementation audit, not a numerical campaign.  It
checks whether the current engine maps its raw transport velocity (voxels per
tick) into the dimensionless beta consumed by c=1 clock/Born-Infeld formulas,
and whether M_REST has one dimensionally consistent role across rest energy,
inertia, kinetic energy, and gravity.

No fitted value, physical target, or near-miss search appears here.  The two
algebraic anchors use exact rational arithmetic with c_lat^2 = 1/3.
"""

from __future__ import annotations

from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def section(text: str, start: str, end: str | None = None) -> str:
    begin = text.index(start)
    if end is None:
        return text[begin:]
    finish = text.index(end, begin + len(start))
    return text[begin:finish]


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def main() -> None:
    gauge = read("engine/include/ftd/ontic/gauge_couplings.h")
    voxel = read("engine/include/ftd/voxel.h")
    proper_time = read("engine/include/ftd/proper_time_rate.h")
    forces_cpu = read("engine/src/render_bridge_phases/phase_forces.cpp")
    forces_gpu = read("engine/cuda/kernels_forces.cu")
    poisson_gpu = read("engine/cuda/kernels_poisson.cu")
    backend = read("engine/src/backend.cpp")
    render_bridge = read("engine/src/render_bridge.cpp")
    transmutation = read("engine/src/transmutation_phases.cpp")
    hazard_gpu = read("engine/cuda/kernels_stencil_single.cu")
    diagnostics = read("engine/src/diagnostics_compute.cpp")
    energy_ledger = read("engine/src/energy_ledger_compute.cpp")
    gpu_engine = read("engine/cuda/gpu_engine.cu")
    particle_masses = read("engine/include/ftd/ontic/particle_masses.h")
    poisson_cpu = read("engine/src/poisson_solvers.cpp")
    redshift_test = read("engine/tests/test_de_broglie_redshift.cpp")
    wave_clock = read(
        "docs/theory/03_derivations/foundational_mechanics/"
        "ANALYSIS_DYNAMICAL_TIME_DILATION.md"
    )

    voxel_gamma = section(voxel, "double gamma_ftd() const", "double born_infeld_core() const")
    voxel_bi = section(voxel, "double born_infeld_core() const", "\n};")
    cpu_push = section(
        forces_cpu,
        "// FTD bandwidth postulate: v²/C² + L² < 1",
        "void phase_forces_integrate_clusters",
    )
    gpu_push = section(
        forces_gpu,
        "// --- γ_FTD momentum integration",
        "// ---------- Movement Kernel ----------",
    )
    cpu_audit = section(
        diagnostics,
        "EnergyAudit compute_energy_audit",
        "EMFieldDiag compute_em_field_at",
    )
    gpu_audit = section(
        gpu_engine,
        "EnergyAudit GpuEngine::energy_audit",
        "// ---------- Injection ----------",
    )

    checks = 0

    require(
        "inline constexpr double C_SPEED = 0.57735026918962576451" in gauge
        and "= C_WAVE = 1/sqrt(3)" in gauge,
        "C1 native causal speed is C_SPEED = 1/sqrt(3), not 1",
    )
    checks += 1

    require(
        "// Lattice velocity (nodes per G*-tick)" in voxel,
        "C2 Voxel::velocity is a raw lattice transport velocity",
    )
    checks += 1

    require(
        "const double C      = C_SPEED;" in cpu_push
        and "double budget  = v2 / C2 + L2;" in cpu_push,
        "C3 CPU momentum integration normalizes raw velocity by C_SPEED",
    )
    checks += 1

    require(
        "const double C  = C_SPEED;" in gpu_push
        and "double budget = v2 / C2 + L2;" in gpu_push,
        "C4 GPU momentum integration normalizes raw velocity by C_SPEED",
    )
    checks += 1

    require(
        "proper_time_rate(double latency, double speed2)" in proper_time
        and "const double arg = f * f - speed2;" in proper_time
        and "C_SPEED" not in section(
            proper_time,
            "FTD_PTR_HD double proper_time_rate",
            "}  // namespace ftd",
        ),
        "C5 the shared matter clock consumes raw speed2 under a separate c=1 formula",
    )
    checks += 1

    require(
        "proper_time_rate(v.latency, v.speed() * v.speed())" in transmutation,
        "C6 CPU proper time passes the raw Voxel velocity without v/C_SPEED conversion",
    )
    checks += 1

    require(
        "::ftd::proper_time_rate(latency[i], speed2)" in hazard_gpu,
        "C7 GPU matter ageing passes the raw transport speed to the same c=1 clock",
    )
    checks += 1

    require(
        "return 1.0 / std::sqrt(1.0 - bw);" in voxel_gamma
        and "C_SPEED" not in voxel_gamma,
        "C8 Voxel gamma_ftd retains c=1 normalization",
    )
    checks += 1

    require(
        "return -M_REST * std::sqrt(1.0 - bw);" in voxel_bi
        and "C_SPEED" not in voxel_bi,
        "C9 the Voxel Born-Infeld rest/kinetic term retains c=1 normalization",
    )
    checks += 1

    require(
        "d.total_energy += std::abs(v.born_infeld_core());" in diagnostics
        and "double bw = v.bandwidth_used();" in diagnostics,
        "C10 public diagnostics consume the c=1 Voxel energy and bandwidth as load-bearing outputs",
    )
    checks += 1

    c2 = Fraction(1, 3)
    legacy_clock_rate2_at_transport_cap = 1 - c2
    require(
        legacy_clock_rate2_at_transport_cap == Fraction(2, 3),
        "C11 exact causal-cap anchor: the c=1 clock still has rate squared 2/3 at v=C_SPEED",
    )
    checks += 1

    require(
        "inline constexpr double M_REST     = K_B;" in particle_masses
        and "simulation energy units" in particle_masses
        and "const double m        = static_cast<double>(N) * M_REST;" in forces_cpu
        and "return -M_REST * std::sqrt(1.0 - bw);" in voxel_bi,
        "C12 one raw M_REST value is consumed both as inertial mass and rest energy",
    )
    checks += 1

    require(
        "a.particle_ke += 0.5 * v.velocity.mag2();" in cpu_audit
        and "M_REST" not in section(
            cpu_audit,
            "if (s != 0)",
            "// Constrained-site Gauss residual",
        ),
        "C13 CPU EnergyAudit kinetic energy omits M_REST and c normalization",
    )
    checks += 1

    require(
        "if (v.state != 0) E_kin += 0.5 * v.velocity.mag2();" in energy_ledger
        and "ea.particle_ke += 0.5 * v.velocity.mag2();" in gpu_audit,
        "C14 tick ledger and GPU audit preserve the same unit-mass kinetic convention",
    )
    checks += 1

    require(
        "double rho = M_REST * std::abs(state.state_at(i));" in poisson_cpu,
        "C15 gravity consumes the same unconverted M_REST as source density",
    )
    checks += 1

    require(
        "const double vmove  = 0.3;" in redshift_test
        and "< C_SPEED = 1/sqrt(3)" in redshift_test
        and "std::sqrt(1.0 - v_read * v_read)" in redshift_test
        and "v=v_g/C_WAVE" in wave_clock
        and "No `voxel.tau` is ever read" in wave_clock,
        "C16 the de Broglie test self-checks raw sqrt(1-v^2), while the independent wave-clock result uses v/C_WAVE",
    )
    checks += 1

    # Exact role fork.  If the shared symbol denotes inertial mass, E0=M*c^2.
    # If it denotes rest energy, m=E0/c^2.  Literal equality of the two roles
    # is possible only in a c=1 coordinate system, not raw engine tick units.
    rest_energy_per_mass = c2
    inertial_mass_per_rest_energy = 1 / c2
    require(
        rest_energy_per_mass == Fraction(1, 3)
        and inertial_mass_per_rest_energy == 3,
        "C17 exact mass-energy fork is E0=M/3 or m=3E0 in raw lattice units",
    )
    checks += 1

    require(
        "tau[i] += sqrt(arg) / sqrt(f);" in poisson_gpu
        and "double v_max = c_speed * f_clamped;" in poisson_gpu,
        "C18 the GPU latency solve additionally advances the c=1 clock and enforces a distinct C_SPEED*f cap",
    )
    checks += 1

    gpu_host_tick = section(
        render_bridge,
        "if (backend_ && backend_->kind() == Backend::Kind::Gpu)",
        "// F2 (callstack audit",
    )
    require(
        "engine_->tick();" in backend
        and "sync_to_host();" in backend
        and "accumulate_proper_time();" in gpu_host_tick,
        "C19 the GPU path advances tau on device and then invokes the host accumulator again",
    )
    checks += 1

    print()
    print(f"RESULT  {checks}/{checks} source-contract checks passed")
    print("ANCHOR  c_lat^2 = 1/3")
    print("ANCHOR  legacy matter-clock rate^2 at the transport cap = 2/3 (not 0)")
    print("ANCHOR  rest-energy/inertial-mass conversion factor = 1/3 or 3")
    print("VERDICT UNMAPPED-DUAL-NORMALIZATION")
    print(
        "The same raw lattice velocity and M_REST scalar are consumed under "
        "incompatible c_lat and c=1 conventions without an explicit map."
    )


if __name__ == "__main__":
    main()
