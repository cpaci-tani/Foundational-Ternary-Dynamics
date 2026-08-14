#!/usr/bin/env python3
"""Exact certificate for FTD-0943.

This script audits the isolated, undamped production C18 relative-field map.
It uses exact rational/symbolic algebra only.  It performs no numerical
near-miss search, parameter fit, or production mutation.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import permutations, product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROGRAMME = ROOT / "docs/theory/10_eft_program"
PREREG = PROGRAMME / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md"
)

EXPECTED_HASHES = {
    PREREG: "0B6F8C0B3C8EC1BA1E65E1FD31E78887BC5FBB6498BEF5C0C807A7EF11179104",
    ROOT / "engine/include/ftd/field_operators.h": (
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48"
    ),
    ROOT / "engine/include/ftd/ontic/gauge_couplings.h": (
        "BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": (
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": (
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4"
    ),
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md"
    ): "06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53",
    ROOT / "scripts/proofs/proof_native_event_activation_characteristic_boundary_v2.py": (
        "E2A6D22946E0E3BD9A5CE208EB7C440567AA72B97C28F507C099F06E93740204"
    ),
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md"
    ): "CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285",
    ROOT / "scripts/proofs/proof_native_c4_modal_circulation_compact_support_obstruction.py": (
        "C1C312E1B5FA9F9EB90DFD1A2B71B38736BC7F8AEE93DFDBA56B88A5133031EA"
    ),
    PROGRAMME / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_EXISTING_LR_AGGREGATE_CARRIER_AND_OCCUPANCY_HISTORY_REALIZATION_BOUNDARY_v1.md"
    ): "D287ED5B5E6FCD15352E191D272A9B1A83D2952A009C1A9BEA5E0CAA985A0697",
    ROOT / "scripts/proofs/proof_existing_lr_occupancy_history_carrier_classifier.py": (
        "54AFAA09E6588A04B702A0F7368874ECA25AC21810E8532E8F04FB550E8C4808"
    ),
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def file_hash(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def hessian(expr: sp.Expr, variables: tuple[sp.Symbol, ...]) -> sp.Matrix:
    return sp.Matrix(
        [[sp.diff(expr, a, b) for b in variables] for a in variables]
    )


def origin_value(expr: sp.Expr, variables: tuple[sp.Symbol, ...]) -> sp.Expr:
    return sp.simplify(expr.subs({v: 0 for v in variables}))


def signed_permutation_substitutions(
    variables: tuple[sp.Symbol, ...],
) -> list[dict[sp.Symbol, sp.Expr]]:
    substitutions: list[dict[sp.Symbol, sp.Expr]] = []
    for perm in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            substitutions.append(
                {
                    variables[i]: (
                        variables[perm[i]]
                        if signs[i] == 1
                        else 1 / variables[perm[i]]
                    )
                    for i in range(3)
                }
            )
    return substitutions


def monomial(
    variables: tuple[sp.Symbol, ...], displacement: tuple[int, int, int]
) -> sp.Expr:
    return sp.prod(v**d for v, d in zip(variables, displacement))


def orbit_witness(displacement: tuple[int, int, int]) -> tuple[int, int, int]:
    """Construct the general signed-permutation witness used in the proof."""
    nonzero = [i for i, value in enumerate(displacement) if value]
    if len(nonzero) == 1:
        source = nonzero[0]
        target = (source + 1) % 3
        values = list(displacement)
        values[source], values[target] = values[target], values[source]
        return tuple(values)  # type: ignore[return-value]
    values = list(displacement)
    values[nonzero[0]] *= -1
    return tuple(values)  # type: ignore[return-value]


def matrix_power(matrix: sp.Matrix, exponent: int) -> sp.Matrix:
    out = sp.eye(matrix.rows)
    factor = matrix
    power = exponent
    while power:
        if power & 1:
            out = out * factor
        factor = factor * factor
        power //= 2
    return out.applyfunc(sp.expand)


def source_contract_checks() -> None:
    for path, expected in EXPECTED_HASHES.items():
        check(f"source hash: {path.relative_to(ROOT)}", file_hash(path) == expected)

    field_text = (ROOT / "engine/include/ftd/field_operators.h").read_text(
        encoding="utf-8"
    )
    gauge_text = (
        ROOT / "engine/include/ftd/ontic/gauge_couplings.h"
    ).read_text(encoding="utf-8")
    read_text = (
        ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
    ).read_text(encoding="utf-8")
    write_text = (
        ROOT / "engine/src/render_bridge_phases/phase_write.cpp"
    ).read_text(encoding="utf-8")

    markers = {
        "field face weight": "* (1.0/3.0)",
        "field edge weight": "* (1.0/6.0)",
        "field center weight": "*F * 4.0",
    }
    for name, marker in markers.items():
        check(name, marker in field_text)
    check(
        "selected production wave coefficient",
        "inline constexpr double C_WAVE = 0.57735026918962576451" in gauge_text,
    )
    check("read L uses C18 operator", "laplacian_field<&Voxel::flux_L>" in read_text)
    check("read R uses C18 operator", "laplacian_field<&Voxel::flux_R>" in read_text)
    check("read L stiffness coefficient", "rb.delta_j_L_[i] = lap_L * cw2;" in read_text)
    check("read R stiffness coefficient", "rb.delta_j_R_[i] = lap_R * cw2;" in read_text)
    check("default L kick", "v.wave_vel_L += rb.delta_j_L_[i];" in write_text)
    check("default R kick", "v.wave_vel_R += rb.delta_j_R_[i];" in write_text)
    check("default L drift", "v.flux_L += v.wave_vel_L;" in write_text)
    check("default R drift", "v.flux_R += v.wave_vel_R;" in write_text)


def exact_symbol_checks() -> tuple[sp.Expr, sp.Matrix]:
    zx, zy, zz = sp.symbols("z_x z_y z_z", nonzero=True)
    zvars = (zx, zy, zz)
    q = sp.Rational

    face_sum = zx + 1 / zx + zy + 1 / zy + zz + 1 / zz
    edge_sum = (
        zx * zy
        + zx / zy
        + zy / zx
        + 1 / (zx * zy)
        + zy * zz
        + zy / zz
        + zz / zy
        + 1 / (zy * zz)
        + zz * zx
        + zz / zx
        + zx / zz
        + 1 / (zz * zx)
    )
    K = sp.factor(q(1, 3) * (4 - q(1, 3) * face_sum - q(1, 6) * edge_sum))

    check("face shell has six monomials", len(sp.Add.make_args(face_sum)) == 6)
    check("edge shell has twelve monomials", len(sp.Add.make_args(edge_sum)) == 12)
    check("vacuum stiffness vanishes", sp.simplify(K.subs({zx: 1, zy: 1, zz: 1})) == 0)
    check("stiffness is nonconstant", sp.diff(K, zx) != 0)

    cx, cy, cz = sp.symbols("c_x c_y c_z")
    K_torus = q(4, 3) - q(2, 9) * (
        cx + cy + cz + cx * cy + cy * cz + cz * cx
    )
    torus_from_shells = sp.expand(
        q(1, 3)
        * (
            4
            - q(1, 3) * 2 * (cx + cy + cz)
            - q(1, 6) * 4 * (cx * cy + cy * cz + cz * cx)
        )
    )
    check("torus reduction exact", sp.expand(torus_from_shells - K_torus) == 0)

    kx, ky, kz = sp.symbols("k_x k_y k_z", real=True)
    kvals = (kx, ky, kz)
    K_local = K_torus.subs(
        {cx: sp.cos(kx), cy: sp.cos(ky), cz: sp.cos(kz)}
    )
    check("local stiffness value zero", origin_value(K_local, kvals) == 0)
    for variable in kvals:
        check(
            f"local stiffness gradient zero: {variable}",
            origin_value(sp.diff(K_local, variable), kvals) == 0,
        )
    HK = hessian(K_local, kvals).subs({v: 0 for v in kvals})
    check("stiffness Hessian exact", HK == sp.diag(q(2, 3), q(2, 3), q(2, 3)))
    check("stiffness Hessian rank three", HK.rank() == 3)
    check("stiffness quadratic coefficient exact", HK / 2 == sp.eye(3) * q(1, 3))

    # If b^2=K and K(0)=0, then b(0)=0 and Hess(K)=2 grad(b)grad(b)^T,
    # whose rank is at most one.  Rank(HK)=3 is the exact contradiction.
    a, b, c = sp.symbols("a b c")
    generic_square_hessian = 2 * sp.Matrix([a, b, c]) * sp.Matrix([[a, b, c]])
    check("generic scalar-square Hessian rank at most one", generic_square_hessian.rank() == 1)
    check("K scalar Laurent-square obstruction", HK.rank() > generic_square_hessian.rank())

    U = sp.Matrix([[1 - K, 1], [-K, 1]])
    check("kick-drift determinant one", sp.factor(U.det()) == 1)
    check("kick-drift trace exact", sp.factor(sp.trace(U) - (2 - K)) == 0)
    Delta = sp.factor(sp.trace(U) ** 2 - 4 * U.det())
    check("discriminant exact", sp.factor(Delta - K * (K - 4)) == 0)

    Delta_local = sp.expand(K_local * (K_local - 4))
    check("discriminant vacuum value zero", origin_value(Delta_local, kvals) == 0)
    for variable in kvals:
        check(
            f"discriminant gradient zero: {variable}",
            origin_value(sp.diff(Delta_local, variable), kvals) == 0,
        )
    HD = hessian(Delta_local, kvals).subs({v: 0 for v in kvals})
    check("discriminant Hessian exact", HD == sp.diag(q(-8, 3), q(-8, 3), q(-8, 3)))
    check("discriminant Hessian rank three", HD.rank() == 3)
    check("Delta scalar Laurent-square obstruction", HD.rank() > generic_square_hessian.rank())

    substitutions = signed_permutation_substitutions(zvars)
    check("full signed cubic group has 48 maps", len(substitutions) == 48)
    for index, substitution in enumerate(substitutions):
        check(
            f"C18 stiffness cubic invariant {index + 1:02d}",
            sp.factor(K.xreplace(substitution) - K) == 0,
        )

    return K, U


def chebyshev_and_translation_checks(K: sp.Expr, U: sp.Matrix) -> None:
    zx, zy, zz = sp.symbols("z_x z_y z_z", nonzero=True)
    zvars = (zx, zy, zz)
    t = sp.symbols("t")
    kappa = sp.symbols("kappa")
    U_abstract = sp.Matrix([[1 - kappa, 1], [-kappa, 1]])

    check(
        "abstract kick-drift specializes to production symbol",
        U_abstract.subs(kappa, K) == U,
    )

    traces = [sp.Integer(2), t]
    for m in range(2, 13):
        traces.append(sp.expand(t * traces[-1] - traces[-2]))
    for m, trace_poly in enumerate(traces):
        check(
            f"Chebyshev trace recurrence m={m}",
            sp.expand(trace_poly - 2 * sp.chebyshevt(m, t / 2)) == 0,
        )

    # Direct matrix confirmation fixes the convention U^m, while the recurrence
    # above supplies the general Cayley-Hamilton induction.
    for m in range(1, 9):
        Um = matrix_power(U_abstract, m)
        expected_trace = 2 * sp.chebyshevt(m, 1 - kappa / 2)
        check(
            f"matrix/Chebyshev trace identity m={m}",
            sp.factor(sp.trace(Um) - expected_trace) == 0,
        )
        recurrence_det = sp.factor(2 - sp.trace(Um))
        check(f"recurrence determinant nonzero m={m}", recurrence_det != 0)
        check(
            f"recurrence leading coefficient m^2 m={m}",
            sp.simplify(sp.diff(2 - 2 * sp.chebyshevt(m, 1 - t / 2), t).subs(t, 0))
            == m * m,
        )

    # T_m'(1)=m^2 is additionally checked exactly over a broad finite range;
    # the theorem derives it generally from T_m(cos theta)=cos(m theta).
    x = sp.symbols("x")
    for m in range(1, 21):
        check(
            f"Chebyshev endpoint derivative m={m}",
            sp.diff(sp.chebyshevt(m, x), x).subs(x, 1) == m * m,
        )

    # Exercise the constructive signed-permutation lemma on every small
    # displacement.  The proof is the two exhaustive support cases in
    # orbit_witness(), not an inference from the bounded enumeration.
    for displacement in product(range(-2, 3), repeat=3):
        if displacement == (0, 0, 0):
            continue
        witness = orbit_witness(displacement)
        support_size = sum(value != 0 for value in displacement)
        check(
            f"orbit witness differs from +/-d: {displacement}",
            witness != displacement
            and witness != tuple(-value for value in displacement),
        )
        check(
            f"orbit witness support branch valid: {displacement}",
            (support_size == 1 and sum(value != 0 for value in witness) == 1)
            or (support_size >= 2 and sum(value != 0 for value in witness) == support_size),
        )

    representative_displacements = [
        (1, 0, 0),
        (1, 1, 0),
        (1, -2, 0),
        (1, 1, 1),
        (1, -2, 3),
    ]
    for displacement in representative_displacements:
        witness = orbit_witness(displacement)
        lhs = monomial(zvars, displacement) + monomial(
            zvars, tuple(-value for value in displacement)
        )
        lhs_witness = monomial(zvars, witness) + monomial(
            zvars, tuple(-value for value in witness)
        )
        check(
            f"translation character not cubic invariant: {displacement}",
            sp.factor(lhs - lhs_witness) != 0,
        )

    # Direct determinant witnesses for several m and displacement classes.
    # The all-m/all-d result follows from cubic invariance plus the Laurent
    # integral-domain adjugate argument frozen in the preregistration.
    for m in range(1, 7):
        Um = matrix_power(U_abstract, m)
        for displacement in representative_displacements:
            zd = monomial(zvars, displacement)
            det_expr = sp.factor((Um - zd * sp.eye(2)).det())
            expected = sp.factor(zd**2 - sp.trace(Um) * zd + 1)
            check(
                f"translator determinant identity m={m}, d={displacement}",
                sp.factor(det_expr - expected) == 0,
            )
            check(
                f"translator determinant nonzero m={m}, d={displacement}",
                det_expr != 0,
            )

    # The adjugate identity is checked with a generic 2x2 matrix.  In the
    # Laurent integral domain, det(A)X=0 and det(A)!=0 force X=0 componentwise.
    a, b, c, d = sp.symbols("a b c d")
    A = sp.Matrix([[a, b], [c, d]])
    adjA = sp.Matrix([[d, -b], [-c, a]])
    check("generic left adjugate identity", adjA * A == A.det() * sp.eye(2))
    check("generic right adjugate identity", A * adjA == A.det() * sp.eye(2))


def scope_firewall_checks() -> None:
    prereg_text = PREREG.read_text(encoding="utf-8")
    required = [
        "infinite-support Bloch or normal modes",
        "nonlocal Fourier characteristic projectors",
        "approximate, dispersive, or exponentially tailed packets",
        "externally driven or maintained localized structures",
        "event-mediated nonlinear and",
        "separately selected oriented ports",
        "cannot prove that a new primitive type is",
        "no numerical search",
        "No tolerance, fit, numerical near-miss",
    ]
    for marker in required:
        check(f"scope firewall: {marker}", marker in prereg_text)


def main() -> None:
    source_contract_checks()
    K, U = exact_symbol_checks()
    chebyshev_and_translation_checks(K, U)
    scope_firewall_checks()

    failed = [name for name, passed in checks if not passed]
    for name, passed in checks:
        print(f"[{'PASS' if passed else 'FAIL'}] {name}")
    print()
    print(f"FTD-0943 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
    if failed:
        print("OUTCOME D — invalid certificate")
        for name in failed:
            print(f"  - {name}")
        raise SystemExit(1)
    print("OUTCOME B — isolated linear C18 has no scalar finite-range exact")
    print("characteristic factor and no nonzero finite-support exact translator")
    print("or periodic complete state at any positive tick count.")
    print("Global/nonlocal, approximate, maintained, and event-mediated nonlinear")
    print("routes remain open; no new primitive type is forced.")


if __name__ == "__main__":
    main()
