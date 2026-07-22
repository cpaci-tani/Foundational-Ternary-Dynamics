#!/usr/bin/env python3
"""Recompute the FTD-0405 NCEMC feasibility and obstruction contracts."""

from decimal import Decimal, getcontext
from fractions import Fraction
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
getcontext().prec = 80


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


failures = 0


def check(name: str, condition: bool) -> None:
    global failures
    print(("PASS  " if condition else "FAIL  ") + name)
    if not condition:
        failures += 1


# Exact harmonic-regime anchors. The additive kappa cancels from the force.
r = Fraction(16)
kappa_a = Fraction(0)
kappa_b = Fraction(7, 5)
u_a = r * r / 128 + kappa_a
u_b = r * r / 128 + kappa_b
force_a = r / 64
force_b = r / 64
check("A1 harmonic force magnitude is 1/4", force_a == Fraction(1, 4))
check("A2 additive zero leaves force invariant", force_a == force_b)
check("A3 additive zero shifts one-pair energy", u_b - u_a == kappa_b - kappa_a)
check("A4 equal and opposite pair forces close momentum", force_a + (-force_b) == 0)

# Recompute the actual FTD-0402 momentum kick and sub-voxel drift at high
# precision. This is a correctness calculation, not a numerical search.
m = Decimal(511) / Decimal(1000)
c2 = Decimal(1) / Decimal(3)
c = c2.sqrt()
force = Decimal(1) / Decimal(4)
q = force / m
velocity = q * c / (c2 + q * q).sqrt()
gamma = Decimal(1) / (Decimal(1) - velocity * velocity / c2).sqrt()
particle_ke = Decimal(2) * m * c2 * (gamma - Decimal(1))
r0 = Decimal(16)
r1 = r0 - Decimal(2) * velocity
delta_u = (r1 * r1 - r0 * r0) / Decimal(128)
work_residual = particle_ke + delta_u

check("A5 sub-voxel position remains in harmonic regime", Decimal(8) <= r1 < r0)
check("A6 individual normalized momentum equals the kick", abs(gamma * m * velocity - force) < Decimal("1e-70"))
check("A7 force-derived work residual is nonzero", abs(work_residual) > Decimal("1e-6"))

print(f"OBS velocity={velocity}")
print(f"OBS effective_separation={r1}")
print(f"OBS particle_ke={particle_ke}")
print(f"OBS delta_potential={delta_u}")
print(f"OBS work_residual={work_residual}")

cpu = read("engine/src/render_bridge_phases/phase_forces.cpp")
cuda = read("engine/cuda/kernels_forces.cu")
tick = read("engine/src/render_bridge.cpp")
movement = read("engine/src/render_bridge_phases/phase_movement.cpp")
diagnostics = read("engine/src/diagnostics_compute.cpp")
poisson = read("engine/src/poisson_solvers.cpp")
particle = read("engine/src/particle_engine.cpp")
test = read("engine/tests/test_ncemc_feasibility.cpp")

cpu_tokens = (
    "double cf = (v.color == cs.color) ? 0.5 : -1.0;",
    "F_mag = as * cf / r2;",
    "F_mag = as * cf / (COLOR_TRANSITION_DENOM * r);",
    "F_mag = as * cf * r / COLOR_LINEAR_DENOM;",
    "f_color.x -= F_mag * ddx / r;",
)
cuda_tokens = (
    "double cf = (ci == cj) ? 0.5 : -1.0;",
    "f_mag = as * cf / r2;",
    "f_mag = as * cf / (COLOR_TRANSITION_DENOM * r);",
    "f_mag = as * cf * r / COLOR_LINEAR_DENOM;",
    "fx -= f_mag * dx * inv_r;",
)
check("S1 CPU frozen color-force profile present", all(t in cpu for t in cpu_tokens))
check("S2 CUDA frozen color-force profile mirrors CPU", all(t in cuda for t in cuda_tokens))
check("S3 force precedes movement in the public tick",
      tick.index("if (toggles.forces)") < tick.index("if (toggles.movement)"))
check("S4 movement advances sub-voxel remainder",
      "v.remainder += v.velocity * rb.dt_;" in movement)
check("S5 audit uses normalized particle kinetic energy",
      "flat_particle_kinetic_energy" in diagnostics)
check("S6 current latency source omits strong pair energy",
      "local_field_wave_energy_density" in poisson
      and "strong" not in poisson[poisson.index("void solve_latency_poisson_cpu"):])
check("S7 ParticleEngine mismatch remains explicitly out of scope",
      "F_strong_mag = SIGMA_STRING * cf" in particle)
check("S8 test comparator includes effective remainder position",
      "X_RIGHT + o.remainder_right" in test
      and "X_LEFT + o.remainder_left" in test)
check("S9 no production strong Hamiltonian was silently installed",
      "strong_pair_potential" not in cpu
      and "strong_pair_potential" not in cuda)

print("VERDICT " + ("NCEMC-FEASIBILITY-PASS" if failures == 0 else "INVALID"))
raise SystemExit(1 if failures else 0)
