#!/usr/bin/env python3
"""Exact source-contract audit for the confinement -> mass -> gravity bridge.

FTD-0400 is a static data-flow result, not a numerical physics campaign.  The
audit checks whether the current engine has one common strong-sector energy
object that (a) generates the colour force, (b) appears in the conserved energy
and momentum bookkeeping, and (c) sources the gravitational/latency field.

The checks deliberately assert source-level contracts rather than fitted
values.  A changed implementation must make this audit fail and force a fresh
review of the FTD-0400 scoped verdict.
"""

from __future__ import annotations

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
    phase_forces = read("engine/src/render_bridge_phases/phase_forces.cpp")
    diagnostics = read("engine/src/diagnostics_compute.cpp")
    energy_ledger = read("engine/src/energy_ledger_compute.cpp")
    lagrangian = read("engine/include/ftd/lagrangian.h")
    poisson_cpu = read("engine/src/poisson_solvers.cpp")
    particle = read("engine/src/particle_engine.cpp")
    gpu_engine = read("engine/cuda/gpu_engine.cu")
    gpu_poisson = read("engine/cuda/kernels_poisson.cu")
    audit_header = read("engine/include/ftd/render_bridge_diagnostics.h")

    cpu_audit = section(
        diagnostics,
        "EnergyAudit compute_energy_audit",
        "EMFieldDiag compute_em_field_at",
    )
    cpu_latency = section(
        poisson_cpu,
        "void solve_latency_poisson_cpu",
        "\n}\n\n}  // namespace ftd",
    )
    particle_diag = section(
        particle,
        "ParticleDiagnostics ParticleEngine::diagnostics",
        "\n}\n\n}  // namespace ftd",
    )
    gpu_audit = section(
        gpu_engine,
        "EnergyAudit GpuEngine::energy_audit",
        "// ---------- Injection ----------",
    )
    gpu_latency = section(
        gpu_poisson,
        "void launch_solve_latency",
        "\n}\n\n}  // namespace kernels",
    )

    checks = 0

    require(
        "Vec3 f_total = f_em_grav_lorentz + f_color;" in phase_forces,
        "C1 RenderBridge colour force acts on particle momentum",
    )
    checks += 1

    require(
        "a.total_energy = a.field_energy + a.wave_energy + a.particle_ke;"
        in cpu_audit
        and "strong_energy" not in cpu_audit
        and "f_strong" not in cpu_audit,
        "C2 CPU EnergyAudit omits strong-field and colour-interaction energy",
    )
    checks += 1

    require(
        "const double E_total = 0.5 * (E_field + E_wave) + E_kin;"
        in energy_ledger
        and "strong" not in energy_ledger
        and "color" not in energy_ledger,
        "C3 EnergyLedger has no strong-sector Hamiltonian term",
    )
    checks += 1

    require(
        "f_color" not in lagrangian
        and "flux_strong" not in lagrangian
        and "strong_energy" not in lagrangian,
        "C4 declared lattice Lagrangian/Hamiltonian has no colour-force term",
    )
    checks += 1

    require(
        "double rho = M_REST * std::abs(state.state_at(i));" in cpu_latency
        and "voxels[i].flux.mag2() + voxels[i].wave_vel.mag2()" in cpu_latency
        and "flux_strong" not in cpu_latency
        and "wave_vel_strong" not in cpu_latency,
        "C5 CPU latency source is imposed M_REST plus ordinary-field energy only",
    )
    checks += 1

    require(
        "f_grav = grad_rho * G_N;" in phase_forces
        and "phi_latency" not in section(
            phase_forces,
            "// Gravitational force from density gradient",
            "// Lorentz (magnetic) force",
        ),
        "C6 force-side gravity is a separate ordinary-density gradient path",
    )
    checks += 1

    require(
        "ea.strong_energy += 0.5 * v.flux_strong.mag2();" in gpu_audit
        and "wave_vel_strong" not in gpu_audit
        and "ea.total_energy = ea.field_energy + ea.wave_energy + ea.particle_ke;"
        in gpu_audit,
        "C7 GPU reports partial strong energy but excludes it from total energy",
    )
    checks += 1

    require(
        "FOUR_PI_G_K_B" in gpu_latency
        and "field_energy" not in gpu_latency
        and "flux_strong" not in gpu_latency,
        "C8 GPU latency source omits both ordinary and strong field energy",
    )
    checks += 1

    require(
        "F_mag = as * cf * r / COLOR_LINEAR_DENOM;" in phase_forces
        and "F_strong_mag = SIGMA_STRING * cf;" in particle,
        "C9 RenderBridge and ParticleEngine use different long-range confinement laws",
    )
    checks += 1

    require(
        "d.total_pe = d.coulomb_pe + d.gravity_pe;" in particle_diag
        and "strong_pe" not in particle_diag,
        "C10 ParticleEngine diagnostics omit the active strong potential",
    )
    checks += 1

    require(
        "Vec3 total_poynting" in audit_header
        and "total_momentum" not in section(
            audit_header,
            "struct EnergyAudit",
            "struct EnergyLedger",
        ),
        "C11 RenderBridge has no complete total-momentum observable",
    )
    checks += 1

    require(
        "const double m        = static_cast<double>(N) * M_REST;"
        in phase_forces,
        "C12 cluster inertial response consumes imposed M_REST directly",
    )
    checks += 1

    print()
    print(f"RESULT  {checks}/{checks} source-contract checks passed")
    print("VERDICT SPLIT-BOOKKEEPING")
    print(
        "No common current-engine strong-sector energy-momentum object both "
        "generates confinement and sources inertia plus gravity."
    )


if __name__ == "__main__":
    main()
