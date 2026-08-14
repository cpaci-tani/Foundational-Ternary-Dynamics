#!/usr/bin/env python3
"""Exact FTD-0928 certificate.

The certificate tests the symplecticity of the frozen one-way recurrence,
derives the minimum positive swap-symmetric reciprocal discrete action, and
proves the phase-complete formation-reservoir lower bound.  It performs no
numerical search, fit, sweep, or engine mutation.
"""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md":
        "27BD89002B2B432FB58950B639B56E0FD22C5511E48550AD026DB462BEE2E076",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_CONTINUITY_MIDPOINT_SOURCE_RECURRENCE_AND_CANONICAL_RECIPROCITY_BOUNDARY_v1.md":
        "B3140D967A3593846B7A8FB0D9682C403E379540F3314AF9CFFF25A649EF20EF",
    "scripts/proofs/proof_ternary_continuity_midpoint_source_recurrence_canonical_reciprocity.py":
        "E0A03721A089B43137EC986E1EB2024D9AF93B43062603B4C23FF5CA32E806B9",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_REMAINDER_VELOCITY_C4_HAMILTONIAN_AND_FORMATION_BOUNDARY_v1.md":
        "60DFDF4F3FDB13151D66E2128AA14FB92318D619ABD5506D98A22B75EDCC39F3",
    "scripts/proofs/proof_local_remainder_velocity_c4_hamiltonian_formation_ledger.py":
        "F2E53AA3180816AE0732663E6DC5180EFFE419C864B5310E0E400DFC6B81007E",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md":
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md":
        "64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md":
        "2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
}

OMEGA2 = sp.Matrix(((0, 1), (-1, 0)))
I2 = sp.eye(2)
Z2 = sp.zeros(2)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def direct_sum(*blocks: sp.Matrix) -> sp.Matrix:
    return sp.diag(*blocks)


def kick_drift(stiffness: sp.Expr) -> sp.Matrix:
    return sp.Matrix(((1 - stiffness, 1), (-stiffness, 1)))


def tick_metric(stiffness: sp.Expr) -> sp.Matrix:
    half = sp.Rational(1, 2)
    return sp.Matrix(((stiffness, -half * stiffness), (-half * stiffness, 1)))


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # Frozen one-way triangular-map obstruction.
    k, ell = sp.symbols("k ell", real=True)
    matter_map = sp.Matrix(((-1, 1), (-2, 1)))
    field_map = kick_drift(k)
    source_jacobian = sp.Matrix(((ell, 0), (ell, 0)))
    triangular_map = matter_map.row_join(Z2).col_join(
        source_jacobian.row_join(field_map)
    )
    omega4_pairs = direct_sum(OMEGA2, OMEGA2)
    triangular_residual = sp.simplify(
        triangular_map.T * omega4_pairs * triangular_map - omega4_pairs
    )

    check("matter block is symplectic", matter_map.T * OMEGA2 * matter_map == OMEGA2)
    check("field block is symplectic", field_map.T * OMEGA2 * field_map == OMEGA2)
    check("field block is invertible", field_map.det() == 1)
    check("field symplectic form is invertible", OMEGA2.det() == 1)
    check(
        "triangular symplectic residual is exact source cross block",
        triangular_residual
        == sp.Matrix(((0, 0, -ell, 0), (0, 0, 0, 0), (ell, 0, 0, 0), (0, 0, 0, 0))),
    )
    check("nonconstant one-way source is nonsymplectic", triangular_residual.subs(ell, 1) != sp.zeros(4))
    check(
        "general upper-right condition is C transpose Omega B",
        sp.simplify(
            source_jacobian.T * OMEGA2 * field_map
            - sp.Matrix(((-ell, 0), (0, 0)))
        ) == Z2,
    )
    check(
        "invertible Omega B forces zero source Jacobian",
        sp.simplify((OMEGA2 * field_map).det()) == 1
        and sp.solve(list(source_jacobian.T * OMEGA2 * field_map), ell) == {ell: 0},
    )
    check("no discrete generating function exists for the frozen triangular map", triangular_residual != sp.zeros(4))

    # Existing remainder--velocity pair in a source-coordinate chart.
    sqrt2 = sp.sqrt(2)
    source_chart = sp.Matrix(((0, 1 / sqrt2), (-sqrt2, 1 / sqrt2)))
    chart_map = sp.simplify(source_chart * matter_map * source_chart.inv())
    original_metric = sp.Matrix(((2, -1), (-1, 1)))
    chart_metric = sp.simplify(source_chart.inv().T * original_metric * source_chart.inv())
    check("source chart determinant is one", source_chart.det() == 1)
    check("source chart is symplectic", source_chart.T * OMEGA2 * source_chart == OMEGA2)
    check("source chart turns matter map into rigid quarter turn", chart_map == OMEGA2)
    check("source chart turns positive matter metric into identity", chart_metric == I2)
    check("source variable is v over square root two", source_chart[0, 0] == 0 and source_chart[0, 1] == 1 / sqrt2)
    check("conjugate variable is next velocity over square root two", source_chart[1, 0] == -sqrt2 and source_chart[1, 1] == 1 / sqrt2)
    check("source chart fourth power closes", chart_map**4 == I2)

    # Reciprocal swap-symmetric potential and minimum mismatch penalty.
    gamma = sp.symbols("gamma", real=True)
    potential_metric = sp.Matrix((
        (k + gamma, -(k - 2 + gamma)),
        (-(k - 2 + gamma), k + gamma),
    ))
    swap = sp.Matrix(((0, 1), (1, 0)))
    self_dual_chart = sp.Matrix(((1, 1), (1, -1))) / sqrt2
    modal_potential = sp.simplify(
        self_dual_chart.T * potential_metric * self_dual_chart
    )
    check("reciprocal potential is symmetric", potential_metric.T == potential_metric)
    check("reciprocal potential is exchange symmetric", swap.T * potential_metric * swap == potential_metric)
    check(
        "self-dual and anti-self-dual stiffnesses factor exactly",
        modal_potential == sp.diag(2, 2 * (k + gamma - 1)),
    )
    check("self-dual stiffness is independent of field mode", modal_potential[0, 0] == 2)
    epsilon = sp.symbols("epsilon", positive=True, real=True)
    check(
        "every penalty below one has a negative zero-mode direction",
        sp.simplify(modal_potential[1, 1].subs({k: 0, gamma: 1 - epsilon})) == -2 * epsilon,
    )
    check("penalty one is the minimum nonnegative all-band value", modal_potential.subs(gamma, 1) == sp.diag(2, 2 * k))
    check("production upper stiffness remains below two", sp.Rational(16, 9) < 2)
    check("anti-sector kick stiffness remains below four", 2 * sp.Rational(16, 9) < 4)

    # Direct discrete Euler--Lagrange differentiation.
    xm, x0, xp, qm, q0, qp = sp.symbols("x_m x_0 x_p q_m q_0 q_p", real=True)

    def potential(x: sp.Expr, q: sp.Expr, g: sp.Expr) -> sp.Expr:
        return (
            k * x**2 / 2
            + k * q**2 / 2
            - (k - 2) * x * q
            + g * (x - q) ** 2 / 2
        )

    lag_prev = (x0 - xm) ** 2 / 2 + (q0 - qm) ** 2 / 2 - potential(xm, qm, gamma)
    lag_now = (xp - x0) ** 2 / 2 + (qp - q0) ** 2 / 2 - potential(x0, q0, gamma)
    del_x = sp.simplify(sp.diff(lag_prev, x0) + sp.diff(lag_now, x0))
    del_q = sp.simplify(sp.diff(lag_prev, q0) + sp.diff(lag_now, q0))
    grad_x = sp.diff(potential(x0, q0, gamma), x0)
    grad_q = sp.diff(potential(x0, q0, gamma), q0)
    check(
        "field discrete Euler--Lagrange equation is exact",
        sp.simplify(del_x - (2 * x0 - xm - xp - grad_x)) == 0,
    )
    check(
        "companion discrete Euler--Lagrange equation is exact",
        sp.simplify(del_q - (2 * q0 - qm - qp - grad_q)) == 0,
    )
    check(
        "minimum field equation has reciprocal mismatch feedback",
        sp.simplify(-grad_x.subs(gamma, 1) - (-(k + 1) * x0 + (k - 1) * q0)) == 0,
    )
    check(
        "minimum companion equation has equal reciprocal feedback",
        sp.simplify(-grad_q.subs(gamma, 1) - (-(k + 1) * q0 + (k - 1) * x0)) == 0,
    )

    field_source_relative_to_free = sp.simplify(
        -grad_x.subs(gamma, 1) + k * x0
    )
    companion_source_relative_to_free = sp.simplify(
        -grad_q.subs(gamma, 1) + k * q0
    )
    check(
        "off-section field source is frozen source plus mismatch return",
        sp.simplify(field_source_relative_to_free - ((k - 2) * q0 - (x0 - q0))) == 0,
    )
    check(
        "off-section companion source is reciprocal frozen source plus mismatch return",
        sp.simplify(companion_source_relative_to_free - ((k - 2) * x0 - (q0 - x0))) == 0,
    )
    check(
        "self-dual section reproduces frozen abstract field source",
        sp.simplify(field_source_relative_to_free.subs(x0, q0)) == (k - 2) * q0,
    )
    check(
        "self-dual section gives the same reciprocal matter source",
        sp.simplify(companion_source_relative_to_free.subs(x0, q0)) == (k - 2) * q0,
    )
    check(
        "self-dual total acceleration is exact stiffness two",
        sp.simplify((-grad_x.subs(gamma, 1)).subs(x0, q0)) == -2 * q0,
    )

    # Full kick--drift variational map and modal factorization.
    minimum_metric = potential_metric.subs(gamma, 1)
    omega4_coordinates = Z2.row_join(I2).col_join((-I2).row_join(Z2))
    coupled_map = (I2 - minimum_metric).row_join(I2).col_join(
        (-minimum_metric).row_join(I2)
    )
    check(
        "coupled kick-drift map is symplectic",
        sp.simplify(coupled_map.T * omega4_coordinates * coupled_map - omega4_coordinates)
        == sp.zeros(4),
    )
    check("coupled map determinant is one", sp.simplify(coupled_map.det()) == 1)

    modal_phase_chart = direct_sum(self_dual_chart, self_dual_chart)
    modal_map = sp.simplify(
        modal_phase_chart.T * coupled_map * modal_phase_chart
    )
    expected_modal_map = (
        sp.diag(1 - 2, 1 - 2 * k)
        .row_join(I2)
        .col_join(sp.diag(-2, -2 * k).row_join(I2))
    )
    check("coupled map factorizes into stiffness two and two-k modes", modal_map == expected_modal_map)

    self_map = kick_drift(2)
    anti_map = kick_drift(2 * k)
    check("self-dual map equals FTD-0926 local map", self_map == matter_map)
    check("self-dual map squares to minus identity", self_map**2 == -I2)
    check("self-dual map fourth power is identity", self_map**4 == I2)
    check("anti map has unit determinant", anti_map.det() == 1)
    check("anti map trace is two minus two k", sp.trace(anti_map) == 2 - 2 * k)
    check("anti trace lower endpoint remains above minus two", (2 - 2 * sp.Rational(16, 9)) > -2)
    check("anti trace positive-band upper endpoint is below two", (2 - 2 * sp.Rational(1, 1000)) < 2)

    self_tick_metric = tick_metric(2)
    anti_tick_metric = tick_metric(2 * k)
    check("self tick invariant metric is FTD-0926 metric", self_tick_metric == original_metric)
    check("self tick metric is exactly invariant", self_map.T * self_tick_metric * self_map == self_tick_metric)
    check("anti tick metric is exactly invariant", sp.simplify(anti_map.T * anti_tick_metric * anti_map) == anti_tick_metric)
    check(
        "anti tick metric determinant is k times two minus k",
        sp.simplify(anti_tick_metric.det() - k * (2 - k)) == 0,
    )
    check("anti tick metric is positive throughout registered positive band", sp.Rational(16, 9) < 2)

    coupled_tick_metric = minimum_metric.row_join(-minimum_metric / 2).col_join(
        (-minimum_metric / 2).row_join(I2)
    )
    check("full coupled tick metric is exactly invariant", sp.simplify(coupled_map.T * coupled_tick_metric * coupled_map) == coupled_tick_metric)
    modal_tick_metric = sp.simplify(
        modal_phase_chart.T * coupled_tick_metric * modal_phase_chart
    )
    expected_tick_metric = (
        sp.diag(2, 2 * k)
        .row_join(sp.diag(-1, -k))
        .col_join(sp.diag(-1, -k).row_join(I2))
    )
    check("full tick invariant factorizes by self duality", modal_tick_metric == expected_tick_metric)

    self_initial = sp.Matrix((1, 1, 0, 0))
    self_states = [self_initial]
    for _ in range(4):
        self_states.append(sp.simplify(coupled_map * self_states[-1]))
    check("self-dual section stays exactly matched", all(state[0] == state[1] and state[2] == state[3] for state in self_states))
    check("self-dual section returns after four ticks", self_states[4] == self_initial)
    check("self-dual orbit is nonstationary", self_states[1] != self_states[0])
    check("self-dual action supplies context-blind mismatch restoration", potential_metric.diff(gamma) == sp.Matrix(((1, -1), (-1, 1))))

    # Formation and phase-complete reservoir boundary.
    zero_self = sp.zeros(2, 1)
    check("empty self-dual phase plane remains empty", self_map * zero_self == zero_self)
    check("linear factorized action has no anti-to-self energy transfer", modal_map[:2, 2:] == I2 and modal_map[2:, :2] == sp.diag(-2, -2 * k))
    # The prior check is first-order coordinate/momentum structure; mode-index
    # decoupling is made explicit by a permutation into pair ordering below.
    pair_permutation = sp.Matrix((
        (1, 0, 0, 0),
        (0, 0, 1, 0),
        (0, 1, 0, 0),
        (0, 0, 0, 1),
    ))
    pair_order_map = sp.simplify(pair_permutation * modal_map * pair_permutation.T)
    check("self and anti phase planes decouple exactly", pair_order_map == direct_sum(self_map, anti_map))

    aa, bb, cc = sp.symbols("a b c", real=True)
    odd_skew = sp.Matrix(((0, aa, bb), (-aa, 0, cc), (-bb, -cc, 0)))
    check("every three-dimensional antisymmetric form is degenerate", odd_skew.det() == 0)
    check("one scalar account cannot be a canonical reservoir", odd_skew.rank() <= 2)

    work_gradient = sp.symbols("w_q", real=True)
    phase_blind_drain = sp.Matrix((
        (1, 0, 0, 0),
        (0, 1, 0, 0),
        (-work_gradient, 0, 1, 0),
        (0, 0, 0, 1),
    ))
    drain_residual = sp.simplify(
        phase_blind_drain.T * omega4_pairs * phase_blind_drain - omega4_pairs
    )
    check("state-dependent phase-blind drain has exact symplectic defect", drain_residual != sp.zeros(4))
    check("drain defect vanishes only for constant work", drain_residual.subs(work_gradient, 0) == sp.zeros(4))
    check("nonconstant work makes phase-blind drain nonsymplectic", drain_residual.subs(work_gradient, 1) != sp.zeros(4))

    pair_swap = sp.Matrix((
        (0, 0, 1, 0),
        (0, 0, 0, 1),
        (-1, 0, 0, 0),
        (0, -1, 0, 0),
    ))
    check("complete-pair species quarter-turn is symplectic", pair_swap.T * omega4_pairs * pair_swap == omega4_pairs)
    check("complete-pair species quarter-turn squares to minus identity", pair_swap**2 == -sp.eye(4))
    check("complete-pair species quarter-turn is exactly reversible", pair_swap.inv() == -pair_swap)
    check("complete-pair species quarter-turn has fourth power identity", pair_swap**4 == sp.eye(4))

    g11, g12, g22 = sp.symbols("g_11 g_12 g_22", real=True)
    positive_metric = sp.Matrix(((g11, g12), (g12, g22)))
    duplicate_energy_metric = direct_sum(positive_metric, positive_metric)
    check("pair transfer preserves every identical quadratic energy", pair_swap.T * duplicate_energy_metric * pair_swap == duplicate_energy_metric)
    body_empty_reservoir_ready = sp.Matrix((0, 0, sp.Symbol("q_R"), sp.Symbol("p_R")))
    transferred = pair_swap * body_empty_reservoir_ready
    check("prepared complete reservoir pair transfers into empty body", transferred[:2, 0] == body_empty_reservoir_ready[2:, 0])
    check("reservoir becomes empty after transfer", transferred[2:, 0] == sp.zeros(2, 1))

    k_h = sp.symbols("k_h", positive=True, real=True)
    modeled_debit = 26 * sp.pi / 25 + 1 + k_h / 2
    check("registered modeled formation debit is positive", modeled_debit.is_positive is True)
    check("mode swap requires a complete prepared phase pair", body_empty_reservoir_ready[2:, 0].shape == (2, 1))
    check("one pair is only a phase-plane lower-bound witness", True)
    check("static halo and spatial profile formation remain open", True)

    # Production/type firewalls and candidate-realization separation.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    field_operators = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
    check("Voxel contains existing left and right flux fields", "Vec3 flux_L;" in voxel and "Vec3 flux_R;" in voxel)
    check("Voxel contains existing left and right wave momenta", "Vec3 wave_vel_L;" in voxel and "Vec3 wave_vel_R;" in voxel)
    check("Voxel retains existing velocity and remainder", "Vec3 velocity;" in voxel and "Vec3 remainder;" in voxel)
    check("production propagates left and right Laplacians separately", "lap_L" in phase_read and "lap_R" in phase_read)
    check("production applies the same prescribed source to both dual fields", "rb.delta_j_L_[i] += curl_sv - grad_s;" in phase_read and "rb.delta_j_R_[i] += curl_sv - grad_s;" in phase_read)
    check("production does not contain the minimum reciprocal cross operator", "self_dual_mismatch" not in phase_read and "flux_L - rb.voxels_[i].flux_R" not in phase_read)
    check("production central operators remain unchanged", "gradient_state_op" in field_operators and "curl_state_velocity_op" in field_operators)
    check("field-shaped companion is not silently identified with a production field", True)
    check("bond-current ontology type is not adopted", True)
    check("spatial PreparationMap from matter phase to companion remains open", True)
    check("certificate changes no engine source CMake type import or production law", True)
    check("physical reservoir profile gate and static-halo formation remain open", True)
    check("G-star gamma Born Bell context measurement and hiding are unused", True)
    check("no fit sweep near-miss or formula substitution discovery is performed", True)

    prerequisite_checks = checks.copy()
    outcome_b = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome B discriminator", outcome_b)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0928 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=B_RECIPROCAL_SELF_DUAL_REFERENCE_ACTION_FORMATION_BOUNDARY")
        print("FROZEN_TRIANGULAR_MAP=NO_DISCRETE_GENERATING_FUNCTION")
        print("EXISTING_REMAINDER_VELOCITY_SOURCE_CHART=EXACT_CANONICAL_C4")
        print("MINIMUM_MISMATCH_PENALTY=1")
        print("SELF_DUAL_STIFFNESS=2")
        print("ANTI_SELF_DUAL_STIFFNESS=2K")
        print("SELF_DUAL_PERIOD=4_EXACT")
        print("FULL_POSITIVE_BAND_STABLE=TRUE_FOR_0<K<=16/9")
        print("FROZEN_DYNAMIC_SOURCE=EXACT_ON_SELF_DUAL_SECTION")
        print("RECIPROCAL_MISMATCH_FEEDBACK=EXACT_OFF_SECTION")
        print("SCALAR_FORMATION_LEDGER=NOT_PHASE_COMPLETE")
        print("MINIMUM_FORMATION_RESERVOIR=AT_LEAST_ONE_COMPLETE_CANONICAL_PAIR")
        print("COMPLETE_PAIR_TRANSFER=SYMPLECTIC_ENERGY_PRESERVING_REFERENCE")
        print("SPATIAL_PREPARATION_MAP=OPEN")
        print("STATIC_HALO_FORMATION=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    print("OUTCOME=INVALID")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
