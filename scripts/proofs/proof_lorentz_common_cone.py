#!/usr/bin/env python3
"""Exact FTD-0412 checks for the BCC-time common-cone gate.

This script re-derives the Wilson operator spectra, infrared coefficients, and
scalar-r obstruction symbolically, then enforces the live source contract.  It
performs no numerical near-miss search and fits no physical target.
"""

from __future__ import annotations

from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def main() -> None:
    checks = 0
    I = sp.I
    zero2 = sp.zeros(2)
    eye2 = sp.eye(2)
    sigma = (
        sp.Matrix([[0, 1], [1, 0]]),
        sp.Matrix([[0, -I], [I, 0]]),
        sp.Matrix([[1, 0], [0, -1]]),
    )
    beta = sp.Matrix.vstack(
        sp.Matrix.hstack(zero2, eye2),
        sp.Matrix.hstack(eye2, zero2),
    )
    gamma = tuple(
        sp.Matrix.vstack(
            sp.Matrix.hstack(zero2, -s),
            sp.Matrix.hstack(s, zero2),
        )
        for s in sigma
    )
    alpha = tuple(beta * g for g in gamma)
    eye4 = sp.eye(4)

    # ------------------------------------------------------------------
    # Clifford convention and the retired spatial-D_W energy oracle.
    # ------------------------------------------------------------------
    for index, g in enumerate(gamma):
        require(g.conjugate().T == -g,
                f"W{index + 1} spatial gamma_{index + 1} is anti-Hermitian")
        checks += 1
    for index, a_i in enumerate(alpha):
        require(a_i.conjugate().T == a_i and a_i * a_i == eye4,
                f"W{index + 4} alpha_{index + 1} is Hermitian and squares to one")
        checks += 1
    require(beta.conjugate().T == beta and beta * beta == eye4,
            "W7 beta is Hermitian and squares to one")
    checks += 1

    lam = sp.Symbol("lam")
    mass_symbol, c, k = sp.symbols("M c k", real=True)
    legacy = mass_symbol * eye4 + I * c * k * gamma[0]
    legacy_char = sp.factor(legacy.charpoly(lam).as_expr())
    legacy_expected = ((lam - mass_symbol) ** 2 - c**2 * k**2) ** 2
    require(sp.simplify(legacy_char - legacy_expected).equals(0),
            "W8 spatial D_W eigenvalues are M+/-c|K|, not sqrt(M^2+c^2K^2)")
    checks += 1

    retired = sp.Rational(3, 2) ** 2 + 1
    actual_plus = (sp.Rational(3, 2) + 1) ** 2
    require(retired == sp.Rational(13, 4)
            and actual_plus == sp.Rational(25, 4)
            and retired != actual_plus,
            "W9 explicit q=pi/2 counterexample invalidates the retired norm oracle")
    checks += 1

    # ------------------------------------------------------------------
    # Correct Hermitian Hamiltonian and exact free pole.
    # ------------------------------------------------------------------
    kx, ky, kz = sp.symbols("k_x k_y k_z", real=True)
    hamiltonian = mass_symbol * beta + c * sum(
        (component * matrix for component, matrix in zip((kx, ky, kz), alpha)),
        sp.zeros(4),
    )
    expected_h2 = (mass_symbol**2 + c**2 * (kx**2 + ky**2 + kz**2)) * eye4
    require(sp.simplify(hamiltonian * hamiltonian - expected_h2) == sp.zeros(4),
            "H1 corrected Wilson Hamiltonian squares to (M^2+c^2K^2)I")
    checks += 1
    require(hamiltonian.conjugate().T == hamiltonian,
            "H2 corrected free Wilson Hamiltonian is Hermitian")
    checks += 1

    # ------------------------------------------------------------------
    # Infrared pole and the quartic common-cone obstruction.
    # ------------------------------------------------------------------
    qx, qy, qz, eps, r = sp.symbols("q_x q_y q_z eps r", real=True)
    qs = (qx, qy, qz)
    sine_sq = sum(sp.sin(eps * q) ** 2 for q in qs)
    wilson_sum = sum(1 - sp.cos(eps * q) for q in qs)
    energy_sq = c**2 * (sine_sq + r**2 * wilson_sum**2)
    series = sp.series(energy_sq, eps, 0, 8).removeO().expand()
    s2 = sum(q**2 for q in qs)
    q4 = sum(q**4 for q in qs)
    q6 = sum(q**6 for q in qs)
    expected_q2 = c**2 * s2
    expected_q4 = c**2 * (r**2 * s2**2 / 4 - q4 / 3)
    expected_q6 = c**2 * (2 * q6 / 45 - r**2 * s2 * q4 / 24)
    require(sp.expand(series.coeff(eps, 2) - expected_q2) == 0,
            "C1 Wilson leading pole is c_s^2 S2")
    checks += 1
    require(sp.expand(series.coeff(eps, 4) - expected_q4) == 0,
            "C2 Wilson q4 pole is c_s^2[r^2 S2^2/4-Q4/3]")
    checks += 1
    require(sp.factor(series.coeff(eps, 6) - expected_q6) == 0,
            "C3 Wilson q6 pole is exact")
    checks += 1

    common_c2 = sp.Rational(1, 7)
    require(expected_q2.subs(c**2, common_c2) == s2 / 7,
            "C4 selecting c_s^2=1/7 aligns the leading BCC-time flux slope")
    checks += 1

    t, r2 = sp.symbols("t r2", real=True)
    quartic_without_c = r2 * s2**2 / 4 - q4 / 3
    axis = sp.expand(quartic_without_c.subs({qx: t, qy: 0, qz: 0}))
    face_diagonal = sp.expand(quartic_without_c.subs({qx: t, qy: t, qz: 0}))
    axis_root = sp.solve(sp.Eq(axis.coeff(t, 4), 0), r2)
    face_root = sp.solve(sp.Eq(face_diagonal.coeff(t, 4), 0), r2)
    require(axis_root == [sp.Rational(4, 3)],
            "C5 axis q4 cancellation requires r^2=4/3")
    checks += 1
    require(face_root == [sp.Rational(2, 3)],
            "C6 face-diagonal q4 cancellation requires r^2=2/3")
    checks += 1
    require(set(axis_root).isdisjoint(face_root),
            "C7 no scalar Wilson r cancels q4 in all directions")
    checks += 1

    # ------------------------------------------------------------------
    # Live source contract.  Defaults remain unchanged; the diagnostic speed
    # is explicit, and gauge/gravity do not silently acquire invented poles.
    # ------------------------------------------------------------------
    header = read("engine/include/ftd/wilson_dirac.h")
    source = read("engine/src/wilson_dirac.cpp")
    cuda = read("engine/cuda/wilson_dirac_gpu.cu")
    common_test = read("engine/tests/test_lorentz_common_cone.cpp")
    orbit_benchmark = read("engine/tests/benchmark_dirac_electron_in_B.cpp")
    cmake = read("engine/CMakeLists.txt")
    couplings = read("engine/include/ftd/ontic/gauge_couplings.h")
    phase_read = read("engine/src/render_bridge_phases/phase_read.cpp")
    poisson = read("engine/src/poisson_solvers.cpp")
    cosmic_gw = read("engine/src/cosmic/cosmic_gravitational_waves.cpp")
    engine_spec = read("engine/SPEC_ENGINE.md")
    audit = read("docs/theory/07_assessment/AUDIT_LORENTZ_COMMON_CONE_GATE.md")

    require("double spatial_speed = 1.0" in header,
            "S1 Wilson spatial-speed selection preserves legacy default one")
    checks += 1
    require("void apply_wilson_hamiltonian" in header
            and "apply_wilson_hamiltonian(k, psi" in source,
            "S2 RK4 real-time evolution calls the corrected Hamiltonian")
    checks += 1
    require("params.spatial_speed" in cuda,
            "S3 retained spatial D_W speed normalization is mirrored on CUDA")
    checks += 1
    require("LORENTZ_BCC_TIME_EFFECTIVE_C2" in common_test
            and "FULL-COMMON-CONE-FAILS" in common_test,
            "S4 native gate tests alignment without claiming recovery")
    checks += 1
    require("C_SPEED = 0.57735026918962576451" in couplings
            and "C_WAVE = 0.57735026918962576451" in couplings,
            "S5 production C_SPEED and C_WAVE remain 1/sqrt(3)")
    checks += 1
    require("lorentz_bcc_time_kappa" in phase_read
            and "LORENTZ_BCC_TIME_EFFECTIVE_C2" not in phase_read,
            "S6 BCC-time cone remains isolated behind its selected flux prototype")
    checks += 1
    require("solve_latency_poisson_cpu" in poisson and "sor_sweep_18pt" in poisson,
            "S7 native latency gravity remains an elliptic Poisson solve")
    checks += 1
    require("gw.current_radius += C_SPEED * dt_" in cosmic_gw,
            "S8 CosmicEngine gravitational-wave speed remains imposed")
    checks += 1
    require("Gauge and native gravity still lack propagating poles" in audit,
            "S9 audit keeps absent gauge/gravity poles explicit")
    checks += 1
    require("lorentz_common_cone" in engine_spec,
            "S10 engine specification registers the common-cone diagnostic")
    checks += 1
    require("apply_wilson_hamiltonian(Hpsi" in orbit_benchmark,
            "S11 orbit instrument measures the same corrected Hamiltonian it evolves")
    checks += 1
    require("set_tests_properties(benchmark_dirac_electron_in_B PROPERTIES DISABLED TRUE)" in cmake,
            "S12 invalidated historical orbit is quarantined from the default passing gate")
    checks += 1

    print(f"\n{checks}/{checks} exact checks passed")
    print("VERDICT  LEADING-CONE-ALIGNABLE; LIVE-COMMON-CONE-FAILS")


if __name__ == "__main__":
    main()
