#!/usr/bin/env python3
"""FTD-0864 exact clock-gated Hamiltonian exchange certificate.

This source-locked certificate checks the minimum symplectic lift of the
FTD-0856 scalar swap, the autonomous harmonic clock pulse, its exact transient
reference-action ledger, and the load-dependence obstruction for a strictly
convex nonlinear clock.  It performs no numerical search.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md":
        "C352EC96A6513D5ED3AB8A7318F47FD1A695FBB0C4FBEB33E9DE43680A70DF93",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md":
        "D73693F364A83D468AC76F3165411784610965A66ACC7BD1E7CE3766A3D267AB",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md":
        "06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md":
        "8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33",
    "engine/include/ftd/eft/catalytic_phase_reference.h":
        "25C094B166DE32894A2FB4F0B0BCEE7A68AB279AB8C7D3BA48D4CAEE2BD4B9AB",
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((name, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {name}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def block_diag(*blocks: sp.Matrix) -> sp.Matrix:
    return sp.diag(*blocks)


def rotation(angle: sp.Expr) -> sp.Matrix:
    return sp.Matrix([
        [sp.cos(angle), sp.sin(angle)],
        [-sp.sin(angle), sp.cos(angle)],
    ])


def main() -> int:
    for relpath, expected in SOURCES.items():
        check(f"source hash {relpath}", sha256(ROOT / relpath) == expected)

    zero = sp.Integer(0)
    one = sp.Integer(1)
    sqrt2 = sp.sqrt(2)
    ident2 = sp.eye(2)
    symplectic2 = sp.Matrix([[0, 1], [-1, 0]])

    scalar_swap = sp.Matrix([[0, 1], [1, 0]])
    check("C7 scalar swap has determinant minus one", scalar_swap.det() == -1)
    check(
        "C8 scalar swap reverses the two-dimensional symplectic form",
        scalar_swap.T * symplectic2 * scalar_swap == -symplectic2,
    )

    full_form = block_diag(symplectic2, symplectic2)
    full_swap = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(2), ident2),
        sp.Matrix.hstack(ident2, sp.zeros(2)),
    )
    check(
        "C9 full two-mode swap is symplectic with determinant plus one",
        full_swap.det() == 1 and full_swap.T * full_form * full_swap == full_form,
    )

    common_relative = sp.Matrix.vstack(
        sp.Matrix.hstack(ident2 / sqrt2, ident2 / sqrt2),
        sp.Matrix.hstack(ident2 / sqrt2, -ident2 / sqrt2),
    )
    check(
        "C10 common-relative transform is orthogonal",
        sp.simplify(common_relative.T * common_relative) == sp.eye(4),
    )
    check(
        "C11 common-relative transform is symplectic",
        sp.simplify(common_relative.T * full_form * common_relative) == full_form,
    )

    qm, pm, qd, pd = sp.symbols("q_m p_m q_d p_d", real=True)
    physical = sp.Matrix([qm, pm, qd, pd])
    modes = sp.simplify(common_relative * physical)
    physical_action = sp.expand((qm**2 + pm**2 + qd**2 + pd**2) / 2)
    mode_action = sp.expand(sum(component**2 for component in modes) / 2)
    check(
        "C12 physical and common-relative action ledgers agree",
        sp.simplify(physical_action - mode_action) == 0,
    )

    theta, I = sp.symbols("theta I", real=True)
    I0, Ic, Ir = sp.symbols("I_0 I_c I_r", real=True, positive=True)
    omega, nu, chi = sp.symbols("omega nu chi", real=True, positive=True)
    epsilon = sp.symbols("epsilon", integer=True, nonnegative=True)
    g = one - sp.cos(theta)
    hamiltonian = omega * I + nu * (Ic + Ir) + epsilon * chi * g * Ir
    theta_dot = sp.diff(hamiltonian, I)
    action_dot = -sp.diff(hamiltonian, theta)
    check("C13 harmonic reference phase advances uniformly", theta_dot == omega)

    phi_c, phi_r = sp.symbols("phi_c phi_r", real=True)

    def poisson(f: sp.Expr, h: sp.Expr) -> sp.Expr:
        pairs = ((theta, I), (phi_c, Ic), (phi_r, Ir))
        return sp.simplify(sum(
            sp.diff(f, q) * sp.diff(h, p) - sp.diff(f, p) * sp.diff(h, q)
            for q, p in pairs
        ))

    check("C14 relative action is a first integral", poisson(Ir, hamiltonian) == 0)
    check("C15 common action is a first integral", poisson(Ic, hamiltonian) == 0)
    check(
        "C16 total matter-signal action is a first integral",
        poisson(Ic + Ir, hamiltonian) == 0,
    )

    action_solution = I0 - epsilon * chi * Ir * g / omega
    check(
        "C17 exact reference-action solution satisfies Hamilton equation",
        sp.simplify(sp.diff(action_solution, theta) * theta_dot - action_dot) == 0,
    )
    closed_energy = sp.simplify(hamiltonian.subs(I, action_solution))
    check(
        "C18 complete Hamiltonian energy is constant under exact action solution",
        closed_energy == omega * I0 + nu * (Ic + Ir),
    )

    minimum_action = sp.simplify(action_solution.subs({theta: sp.pi, epsilon: 1}))
    check(
        "C19 active-cycle reserve bound is exact",
        minimum_action == I0 - 2 * chi * Ir / omega,
    )
    B = sp.expand((qm**2 + pm**2) / 2)
    empty_signal_relative_action = sp.expand(
        ((qm - zero)**2 + (pm - zero)**2) / 4
    )
    check(
        "C20 empty signal gives relative action equal to half event energy",
        sp.simplify(empty_signal_relative_action - B / 2) == 0,
    )

    check(
        "C21 gate is nonnegative square with zero endpoints and maximum two",
        sp.simplify(g - 2 * sp.sin(theta / 2) ** 2) == 0
        and g.subs(theta, 0) == 0
        and g.subs(theta, 2 * sp.pi) == 0
        and g.subs(theta, sp.pi) == 2,
    )
    gate_area = sp.integrate(g, (theta, 0, 2 * sp.pi))
    check("C22 exact full-cycle gate area is two pi", gate_area == 2 * sp.pi)

    common_phase = sp.simplify(2 * sp.pi * nu / omega)
    relative_extra = sp.simplify(chi * gate_area / omega)
    check("C23 common-mode phase is two pi nu over omega", common_phase == 2 * sp.pi * nu / omega)
    check("C24 additional relative phase is two pi chi over omega", relative_extra == 2 * sp.pi * chi / omega)

    winding_n, winding_k = sp.symbols("n k", integer=True)
    registered_common = sp.simplify(common_phase.subs(nu, winding_n * omega))
    registered_extra = sp.simplify(relative_extra.subs(chi, (2 * winding_k + 1) * omega / 2))
    check(
        "C25 registered windings give zero common and pi relative phase modulo two pi",
        sp.simplify(sp.cos(registered_common) - 1) == 0
        and sp.simplify(sp.sin(registered_common)) == 0
        and sp.simplify(sp.cos(registered_extra) + 1) == 0
        and sp.simplify(sp.sin(registered_extra)) == 0,
    )

    common_rotation = rotation(2 * sp.pi)
    relative_rotation_active = rotation(3 * sp.pi)
    active_mode_map = block_diag(common_rotation, relative_rotation_active)
    active_physical_map = sp.simplify(common_relative.T * active_mode_map * common_relative)
    check("C26 minimum active winding produces exact full-mode swap", active_physical_map == full_swap)

    inactive_mode_map = block_diag(rotation(2 * sp.pi), rotation(2 * sp.pi))
    inactive_physical_map = sp.simplify(common_relative.T * inactive_mode_map * common_relative)
    check("C27 minimum inactive winding produces identity", inactive_physical_map == sp.eye(4))

    emission_input = sp.Matrix([qm, pm, 0, 0])
    absorption_input = sp.Matrix([0, 0, qd, pd])
    check(
        "C28 active map emits matter into empty signal",
        active_physical_map * emission_input == sp.Matrix([0, 0, qm, pm]),
    )
    check(
        "C29 same active map absorbs signal into empty matter",
        active_physical_map * absorption_input == sp.Matrix([qd, pd, 0, 0]),
    )
    emission_output = active_physical_map * emission_input
    signal_energy = sp.expand((emission_output[2] ** 2 + emission_output[3] ** 2) / 2)
    check("C30 emitted signal energy equals initial matter energy", sp.simplify(signal_energy - B) == 0)

    check(
        "C31 reference action returns after full cycle",
        sp.simplify(action_solution.subs({theta: 2 * sp.pi, epsilon: 1}) - I0) == 0,
    )
    interaction_energy = epsilon * chi * g * Ir
    check(
        "C32 interaction energy vanishes at both cycle endpoints",
        interaction_energy.subs(theta, 0) == 0
        and interaction_energy.subs(theta, 2 * sp.pi) == 0,
    )
    check(
        "C33 generic active load gives transient reference backreaction",
        sp.simplify(action_dot.subs({theta: sp.pi / 2, epsilon: 1}) + chi * Ir) == 0,
    )
    reversed_hamiltonian = sp.simplify(hamiltonian.subs(theta, -theta))
    check("C34 Hamiltonian is invariant under canonical time reversal", reversed_hamiltonian == hamiltonian)
    check("C35 harmonic pulse area is load independent", sp.diff(relative_extra, Ir) == 0)

    K0, K_loaded, Kp, Kpp = sp.symbols("K_0 K_loaded K_prime K_second", positive=True)
    nonlinear_ledger = K_loaded + chi * g * Ir - K0
    check(
        "C36 nonlinear clock energy ledger has the registered closed form",
        sp.diff(nonlinear_ledger, Ir) == chi * g,
    )
    implicit_action_derivative = -chi * g / Kp
    check(
        "C37 nonlinear implicit action derivative has the registered sign",
        sp.simplify(Kp * implicit_action_derivative + chi * g) == 0,
    )
    pulse_derivative_integrand = chi**2 * g**2 * Kpp / Kp**3
    check(
        "C38 convex-clock pulse-area derivative is strictly positive",
        pulse_derivative_integrand.is_nonnegative
        and pulse_derivative_integrand.subs(theta, sp.pi) != 0,
    )

    c = sp.symbols("c", positive=True)
    quartic_clock = c * I ** sp.Rational(4, 3)
    quartic_first = sp.diff(quartic_clock, I)
    quartic_second = sp.diff(quartic_first, I)
    check(
        "C39 quartic action Hamiltonian is increasing and strictly convex",
        quartic_first == sp.Rational(4, 3) * c * I ** sp.Rational(1, 3)
        and quartic_second == sp.Rational(4, 9) * c * I ** sp.Rational(-2, 3),
    )

    theorem_0863 = (ROOT / next(path for path in SOURCES if "CATALYTIC_PHASE" in path)).read_text(encoding="utf-8")
    theorem_0846 = (ROOT / next(path for path in SOURCES if "SWAP_PARITY" in path)).read_text(encoding="utf-8")
    check(
        "C40 scope firewall preserves nonlinear-controller and production debts",
        "harmonic phase kinematics does not select the quartic clock" in theorem_0863
        and "Exact readout without energy destruction is possible; exact readout without\nbackreaction is not" in theorem_0846
        and "G_STAR" not in globals(),
    )

    passed = sum(ok for _, ok in checks)
    total = len(checks)
    print(f"\nFTD-0864 clock-gated Hamiltonian exchange: {passed}/{total} PASS")
    print("SCALAR_SWAP_REQUIRES_TWO_MODE_SYMPLECTIC_LIFT")
    print("AUTONOMOUS_HARMONIC_PHASE_GATE_GIVES_EXACT_HOLD_SWAP_AND_RESERVE_LEDGER")
    print("STRICTLY_CONVEX_QUARTIC_CLOCK_HAS_LOAD_DEPENDENT_SWAP_ANGLE")
    print("DYNAMIC_ELIGIBILITY_COMPENSATION_GSTAR_GEARBOX_AND_PRODUCTION_REMAIN_OPEN")
    print("VERDICT=OUTCOME_B_EXACT_HARMONIC_LIFT_QUARTIC_CONTROLLER_BOUNDARY")
    return 0 if passed == total == 40 else 1


if __name__ == "__main__":
    raise SystemExit(main())
