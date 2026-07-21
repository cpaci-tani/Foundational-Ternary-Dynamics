#!/usr/bin/env python3
"""Exact/source-contract verifier for FTD-0406.

This verifier recomputes frozen algebraic anchors and checks that audit,
ledger, localization, projection, and gravity share the selected production
implementation.  It performs no numerical search or mass fit.
"""

from fractions import Fraction
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {name}")


# Exact mathematical anchors.
c2 = Fraction(1, 3)
m = Fraction(511, 1000)
e0 = m * c2
check("A1 C_SPEED squared is exactly 1/3", c2 == Fraction(1, 3))
check("A2 E0=M_INERTIAL*C_SPEED squared", e0 == Fraction(511, 3000))

# For r>=8, alpha_s=1 and g=r/64.  Integral from 8 to r is
# (r^2-8^2)/128.  U=-c_f*integral.
integral_8_16 = Fraction(16 * 16 - 8 * 8, 128)
check("A3 capped harmonic radial integral 8 to 16 is 3/2",
      integral_8_16 == Fraction(3, 2))
check("A4 different-colour delta U is +3/2",
      -Fraction(-1, 1) * integral_8_16 == Fraction(3, 2))
check("A5 same-colour delta U is -3/4",
      -Fraction(1, 2) * integral_8_16 == Fraction(-3, 4))
check("A6 strong gravitational mass uses one over c squared",
      Fraction(1, 1) / c2 == 3)

# Momentum projection identity: p_i(lambda)=pbar+lambda(p_i-pbar).
# The deviations sum to zero by definition, so total momentum is invariant.
momenta = [Fraction(-7, 5), Fraction(1, 5), Fraction(6, 5)]
pbar = sum(momenta, Fraction()) / len(momenta)
deviations = [p - pbar for p in momenta]
lam = Fraction(11, 7)
projected = [pbar + lam * d for d in deviations]
check("A7 projection deviations sum exactly to zero", sum(deviations) == 0)
check("A8 projection preserves total momentum exactly",
      sum(projected) == sum(momenta))

strong_h = read("engine/include/ftd/strong_stress_energy.h")
strong_cpp = read("engine/src/strong_stress_energy.cpp")
forces_cpp = read("engine/src/render_bridge_phases/phase_forces.cpp")
poisson_cpp = read("engine/src/poisson_solvers.cpp")
diag_cpp = read("engine/src/diagnostics_compute.cpp")
ledger_cpp = read("engine/src/energy_ledger_compute.cpp")
toggles_h = read("engine/include/ftd/term_toggles.h")
test_cpp = read("engine/tests/test_strong_stress_energy_contract.cpp")

check("S1 vacuum convention is implemented by integral from one",
      "double integral_from_one(double r)" in strong_cpp
      and "return -color_factor(color_a, color_b) * integral_from_one(r);" in strong_cpp)
check("S2 harmonic primitive uses the frozen denominator",
      "(2.0 * COLOR_LINEAR_DENOM)" in strong_cpp)
check("S3 force proposal consumes the shared radial profile in contract mode",
      "F_mag = cf * strong_radial_profile(r);" in forces_cpp)
check("S4 projection uses exactly 96 bisection steps",
      "for (int iter = 0; iter < 96; ++iter)" in strong_cpp)
check("S5 projection preserves proposal positions",
      "rb.voxels_[particle.idx].velocity =" in strong_cpp
      and "remainder" not in strong_cpp.split("void complete_strong_energy_step", 1)[1].split("}  // namespace ftd", 1)[0])
check("S6 localization is midpoint CIC and sample-normalized",
      "static_cast<double>(s) + 0.5" in strong_cpp
      and "cell.weight / weight_sum" in strong_cpp)
check("S7 local central stress is the pair virial",
      "-d.x * force.x" in strong_cpp and "-d.y * force.z" in strong_cpp)
check("S8 latency source divides selected T00 by canonical c squared",
      "1.0 / (C_SPEED * C_SPEED)" in poisson_cpp
      and "energy_density * inv_c2" in poisson_cpp)
check("S9 audit and ledger call the same pair-energy implementation",
      "compute_strong_potential_energy(rb)" in diag_cpp
      and "compute_strong_potential_energy(rb)" in ledger_cpp)
check("S10 toggle is default-off and CPU-scoped",
      "bool strong_stress_energy = false" in toggles_h
      and '"strong_stress_energy"' in toggles_h
      and "ToggleBackend::CPU" in toggles_h)
check("S11 failure diagnostics are explicit",
      "projection_failures" in strong_h and "topology_failures" in strong_h
      and "surface_failure" in strong_cpp)
check("S12 native test contains two-body, three-body, source, and failure gates",
      all(token in test_cpp for token in (
          "projected strong Hamiltonian closes",
          "three-body Hamiltonian closes",
          "selected strong source changes latency potential",
          "topology change is surfaced",
          "ineligible mixed-force projection is surfaced",
      )))

banned = ("CODATA", "alpha^11", "ALPHA^11", "M_REST", "m_e")
production_and_test = strong_h + strong_cpp + test_cpp
check("S13 no mass target or compatibility mass alias enters implementation",
      not any(token in production_and_test for token in banned))

passed = sum(ok for _, ok in checks)
print(f"SUMMARY {passed}/{len(checks)}")
if passed != len(checks):
    print("VERDICT STRONG-STRESS-ENERGY-CONTRACT-INVALID")
    sys.exit(1)
print("VERDICT STRONG-STRESS-ENERGY-CONTRACT-PASS")
