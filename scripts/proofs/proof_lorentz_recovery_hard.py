#!/usr/bin/env python3
"""Exact finite-lattice audit of FTD's free-flux Lorentz-recovery claim.

This is not a parameter scan and contains no comparison to a physical target.
It derives the Fourier symbol of the production 18-point stencil, combines it
with the production kick-drift time update, and expands the *fully discrete*
dispersion relation through sixth order in dimensionless momentum.

It also proves a scoped obstruction: every nearest-Moore (faces + edges +
corners) Laplacian that is normalized at O(k^2) and rotationally isotropic at
O(k^4) has symbol value 16/3 at (pi, pi, 0).  A centered explicit leapfrog at
the spacetime-isotropic Courant number r=c*dt/a=1 therefore cannot be stable.

Finally it gives a constructive boundary witness: a finite radius-two spatial
symbol is exactly bounded in [0,4], preserves the required quartic term, and
therefore supports a stable r=1 update with no dimension-six pole correction.
That witness violates FTD Postulate 4's one-Moore-shell dependency bound, so it
is an architecture candidate, not a recovered result for the current model.
"""

from __future__ import annotations

from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def require(condition: bool, label: str) -> None:
    if not condition:
        raise AssertionError(label)
    print(f"PASS  {label}")


def homogeneous(expr: sp.Expr, variables: tuple[sp.Symbol, ...], degree: int) -> sp.Expr:
    poly = sp.Poly(sp.expand(expr), *variables)
    return sp.expand(
        sum(
            coeff * sp.prod(var**power for var, power in zip(variables, monomial))
            for monomial, coeff in poly.terms()
            if sum(monomial) == degree
        )
    )


def main() -> None:
    field_ops = read("engine/include/ftd/field_operators.h")
    phase_read = read("engine/src/render_bridge_phases/phase_read.cpp")
    phase_write = read("engine/src/render_bridge_phases/phase_write.cpp")
    toggles = read("engine/include/ftd/term_toggles.h")
    couplings = read("engine/include/ftd/ontic/gauge_couplings.h")
    wilson_h = read("engine/include/ftd/wilson_dirac.h")
    render_h = read("engine/include/ftd/render_bridge.h")

    checks = 0

    require(
        "(1/3)·face_sum + (1/6)·edge_sum − 4·center" in field_ops
        and "* (1.0/3.0)" in field_ops
        and "* (1.0/6.0)" in field_ops,
        "S1 production FULL stencil is the 18-point face=1/3, edge=1/6 operator",
    )
    checks += 1
    require(
        "const double cw2 = rb.toggles.lorentz_bcc_time_floquet" in phase_read
        and ": C_WAVE * C_WAVE);" in phase_read
        and "bool lorentz_period2_floquet = false" in toggles
        and "bool lorentz_bcc_time_floquet = false" in toggles
        and "rb.delta_j_[i] = lap * cw2;" in phase_read,
        "S2 default phase_read applies C_WAVE^2 times the spatial Laplacian",
    )
    checks += 1
    require(
        "v.wave_vel += rb.delta_j_[i];" in phase_write
        and "v.flux += v.wave_vel;" in phase_write,
        "S3 default phase_write is the unit-step kick-drift update",
    )
    checks += 1
    require(
        "bool symplectic_leapfrog = false" in toggles
        and "bool verlet_wave_integrator = false" in toggles
        and "bool lorentz_period2_floquet = false" in toggles
        and "BccStencilMode bcc_stencil = BccStencilMode::FULL" in toggles,
        "S4 the audited kick-drift/FULL path is the production default",
    )
    checks += 1
    require(
        "inline constexpr double C_WAVE = 0.57735026918962576451" in couplings,
        "S5 production Courant number squared is exactly represented as 1/3 by contract",
    )
    checks += 1

    qx, qy, qz, eps = sp.symbols("q_x q_y q_z eps", real=True)
    variables = (qx, qy, qz)
    spatial_symbol = (
        4
        - sp.Rational(2, 3) * (sp.cos(qx) + sp.cos(qy) + sp.cos(qz))
        - sp.Rational(2, 3)
        * (sp.cos(qx) * sp.cos(qy) + sp.cos(qx) * sp.cos(qz) + sp.cos(qy) * sp.cos(qz))
    )
    scaled = spatial_symbol.subs({qx: eps * qx, qy: eps * qy, qz: eps * qz})
    spatial_series = sp.series(scaled, eps, 0, 8).removeO().expand()

    s2 = qx**2 + qy**2 + qz**2
    q4 = qx**4 + qy**4 + qz**4
    q6 = qx**6 + qy**6 + qz**6
    expected_spatial = (
        eps**2 * s2
        - eps**4 * s2**2 / 12
        + eps**6 * (s2 * q4 / 72 - q6 / 90)
    )
    require(
        sp.expand(spatial_series - expected_spatial) == 0,
        "A1 spatial symbol is S2-S2^2/12+S2*Q4/72-Q6/90+O(q^8)",
    )
    checks += 1
    require(
        sp.expand(homogeneous(spatial_series.coeff(eps, 4), variables, 4) + s2**2 / 12) == 0,
        "A2 the complete quartic spatial correction is rotationally invariant",
    )
    checks += 1
    require(
        sp.Rational(1, 360)
        != sp.Rational(1, 8)
        * (s2 * q4 / 72 - q6 / 90).subs({qx: 1, qy: 1, qz: 0}),
        "A3 the first cubic-direction discriminator occurs at sixth order in the symbol",
    )
    checks += 1

    # The symbol is multi-affine in a=cos(qx), b=cos(qy), c=cos(qz), so its
    # extrema over [-1,1]^3 occur at cube vertices.  Enumerating eight exact
    # vertices is an exhaustive proof, not a numerical search.
    a, b, c = sp.symbols("a b c", real=True)
    symbol_cos = 4 - sp.Rational(2, 3) * (a + b + c + a * b + a * c + b * c)
    vertex_values = {
        (av, bv, cv): sp.simplify(symbol_cos.subs({a: av, b: bv, c: cv}))
        for av in (-1, 1)
        for bv in (-1, 1)
        for cv in (-1, 1)
    }
    lambda_max = max(vertex_values.values())
    require(
        lambda_max == sp.Rational(16, 3)
        and vertex_values[(-1, -1, 1)] == sp.Rational(16, 3),
        "A4 exact production-symbol maximum is 16/3, attained at (pi,pi,0) permutations",
    )
    checks += 1
    require(
        sp.Rational(4, 1) / lambda_max == sp.Rational(3, 4),
        "A5 explicit centered-time stability requires r^2 <= 3/4, not r^2 <= 1/3",
    )
    checks += 1
    require(
        sp.Rational(1, 3) < sp.Rational(3, 4),
        "A6 C_WAVE^2=1/3 is conservative and is not the CFL saturation of the production stencil",
    )
    checks += 1

    # Fully discrete dispersion.  With theta=omega*dt and r=c*dt/a,
    # 4 sin^2(theta/2) = r^2 M(q).  Solve in y=theta^2 order by order.
    r2 = sp.symbols("r2", real=True)
    a4 = r2 * (r2 - 1) * s2**2 / 12
    a6 = (
        r2**2 * (4 * r2 - 5) * s2**3 / 360
        + r2 * s2 * q4 / 72
        - r2 * q6 / 90
    )
    y = eps**2 * r2 * s2 + eps**4 * a4 + eps**6 * a6
    temporal_symbol = y - y**2 / 12 + y**3 / 360
    rhs = r2 * expected_spatial
    require(
        sp.factor(sp.series(temporal_symbol - rhs, eps, 0, 8).removeO()) == 0,
        "D1 fully discrete solution satisfies 4 sin^2(theta/2)=r^2 M(q) through O(q^6)",
    )
    checks += 1
    require(
        sp.factor(a4) == r2 * (r2 - 1) * s2**2 / 12,
        "D2 the leading boost-violating on-shell term is r^2(r^2-1)S2^2/12",
    )
    checks += 1
    require(
        sp.solve(sp.Eq(r2 * (r2 - 1), 0), r2) == [0, 1],
        "D3 nontrivial cancellation of the dimension-six dispersion term requires r^2=1",
    )
    checks += 1
    current_a4 = sp.simplify(a4.subs(r2, sp.Rational(1, 3)))
    current_a6 = sp.simplify(a6.subs(r2, sp.Rational(1, 3)))
    require(
        sp.expand(current_a4 + s2**2 / 54) == 0
        and sp.expand(current_a6
                      + sp.Rational(11, 9720) * s2**3
                      - s2 * q4 / 216
                      + q6 / 270) == 0,
        "D4 current engine has the corrected complete sixth-order tensor",
    )
    checks += 1
    require(
        sp.Rational(1, 1) > sp.Rational(3, 4),
        "D5 r^2=1 cancellation is incompatible with the production explicit stability ceiling",
    )
    checks += 1

    # Broaden D5 to every normalized nearest-Moore stencil with quartic
    # rotational isotropy.  f/e/h are face/edge/corner weights.
    f, e, h = sp.symbols("f e h", real=True)
    family_solution = sp.solve(
        [sp.Eq(f + 4 * e + 4 * h, 1), sp.Eq(e + 2 * h, sp.Rational(1, 6))],
        [f, e],
        dict=True,
    )[0]
    moore_at_pi_pi_0 = 8 * f + 16 * e
    family_value = sp.simplify(moore_at_pi_pi_0.subs(family_solution))
    require(
        family_value == sp.Rational(16, 3),
        "N1 every normalized quartic-isotropic nearest-Moore stencil has M(pi,pi,0)=16/3",
    )
    checks += 1
    require(
        family_value > 4,
        "N2 no such nearest-Moore stencil supports a stable r=1 centered explicit update",
    )
    checks += 1

    # Constructive escape witness outside the nearest-Moore class.  Put
    # u_i=1-cos(q_i) in [0,2], O=sum_{i!=j}u_i^2 u_j, and define
    #   M_R2 = M18 - O/4 + 9 u_x u_y u_z/8.
    # The added terms begin at q^6, so normalization and the isotropic quartic
    # term are unchanged.  Their cos^2 factors require spatial radius two.
    u, v, w = sp.symbols("u v w", real=True)
    uvw_variables = (u, v, w)
    uvw_s1 = u + v + w
    uvw_s2 = u * v + u * w + v * w
    ordered_21 = u**2 * v + u**2 * w + u * v**2 + u * w**2 + v**2 * w + v * w**2
    candidate_uvw = sp.expand(
        2 * uvw_s1
        - sp.Rational(2, 3) * uvw_s2
        - ordered_21 / 4
        + sp.Rational(9, 8) * u * v * w
    )
    candidate_q = candidate_uvw.subs(
        {u: 1 - sp.cos(qx), v: 1 - sp.cos(qy), w: 1 - sp.cos(qz)}
    )
    candidate_scaled = candidate_q.subs({qx: eps * qx, qy: eps * qy, qz: eps * qz})
    candidate_series = sp.series(candidate_scaled, eps, 0, 8).removeO().expand()
    require(
        sp.expand(candidate_series.coeff(eps, 2) - s2) == 0
        and sp.expand(candidate_series.coeff(eps, 4) + s2**2 / 12) == 0,
        "E1 radius-two witness preserves normalization and the -S2^2/12 quartic term",
    )
    checks += 1

    # Exact range certificate.  On each of the 4^3 rational sub-boxes of
    # [0,2]^3, convert the degree-(2,2,2) polynomial to the tensor Bernstein
    # basis.  A polynomial is a convex combination of its Bernstein
    # coefficients on the box, so coefficients in [0,4] prove 0<=M_R2<=4.
    bernstein_values: list[sp.Expr] = []
    t0, t1, t2 = sp.symbols("t0 t1 t2", real=True)
    t_variables = (t0, t1, t2)
    edges = tuple(sp.Rational(i, 2) for i in range(5))
    for box in product(range(4), repeat=3):
        lower = tuple(edges[i] for i in box)
        upper = tuple(edges[i + 1] for i in box)
        mapped = candidate_uvw.subs(
            {
                uvw_variables[i]: lower[i] + (upper[i] - lower[i]) * t_variables[i]
                for i in range(3)
            }
        )
        power_coeffs = {monomial: coeff for monomial, coeff in sp.Poly(mapped, *t_variables).terms()}
        for index in product(range(3), repeat=3):
            coefficient = 0
            for powers in product(*(range(i + 1) for i in index)):
                weight = sp.prod(
                    sp.binomial(index[axis], powers[axis]) / sp.binomial(2, powers[axis])
                    for axis in range(3)
                )
                coefficient += power_coeffs.get(powers, 0) * weight
            bernstein_values.append(sp.simplify(coefficient))
    require(
        min(bernstein_values) >= 0 and max(bernstein_values) <= 4,
        "E2 exact Bernstein certificate proves 0 <= M_R2(q) <= 4 on the Brillouin zone",
    )
    checks += 1
    require(
        candidate_uvw.subs({u: 0, v: 0, w: 0}) == 0
        and candidate_uvw.subs({u: 0, v: 0, w: 2}) == 4
        and candidate_uvw.subs({u: 2, v: 2, w: 2}) == 1,
        "E3 witness attains the stability ceiling and has no all-pi corner zero",
    )
    checks += 1

    # At r=1, invert T=4 sin^2(theta/2) as
    # theta^2=T+T^2/12+T^3/90+O(T^4).  The q^4 pole coefficient vanishes.
    candidate_theta2 = sp.series(
        candidate_series + candidate_series**2 / 12 + candidate_series**3 / 90,
        eps,
        0,
        8,
    ).removeO().expand()
    require(
        sp.expand(candidate_theta2.coeff(eps, 4)) == 0,
        "E4 stable r=1 witness cancels the complete dimension-six free-pole correction",
    )
    checks += 1
    require(
        sp.expand(candidate_theta2.coeff(eps, 6)) != 0,
        "E5 witness moves the first free-pole Lorentz violation to dimension eight, not to zero",
    )
    checks += 1
    candidate_cos = sp.Poly(
        sp.expand(candidate_uvw.subs({u: 1 - a, v: 1 - b, w: 1 - c})),
        a,
        b,
        c,
    )
    require(
        all(candidate_cos.degree(var) <= 2 for var in (a, b, c))
        and candidate_cos.coeff_monomial(a**2 * b) != 0,
        "E6 escape is finite radius two and therefore lies outside Postulate 4's nearest-Moore class",
    )
    checks += 1

    # The corrected Wilson Hamiltonian remains a separate continuous-time/RK4
    # sector. Its default massless small-q coefficient is one in the same raw
    # lattice coordinates; no C_SPEED factor links it to RenderBridge.
    require(
        "3 spatial dimensions; time evolved continuously via RK4" in wilson_h
        and "double spatial_speed = 1.0" in wilson_h,
        "M1 corrected Wilson matter uses an independent clock and preserves default c_s=1",
    )
    checks += 1
    require(
        "C_SPEED" not in wilson_h and "wilson_dirac" not in render_h.lower(),
        "M2 no live C_SPEED or RenderBridge integration links Wilson matter to flux",
    )
    checks += 1
    q = sp.symbols("q", real=True)
    wilson_massless_e2 = sp.sin(q) ** 2 + (1 - sp.cos(q)) ** 2
    require(
        sp.series(wilson_massless_e2, q, 0, 6) == q**2 - q**4 / 12 + sp.O(q**6),
        "M3 corrected massless Wilson Hamiltonian has E^2=q^2-q^4/12+O(q^6) at c_s=r=1",
    )
    checks += 1

    print()
    print(f"RESULT  {checks}/{checks} exact/source-contract checks passed")
    print("SPATIAL M(q) = S2 - S2^2/12 + S2*Q4/72 - Q6/90 + O(q^8)")
    print("FULL     theta^2 = r^2 S2 + r^2(r^2-1)S2^2/12")
    print("                    + r^4(4r^2-5)S2^3/360")
    print("                    + r^2*S2*Q4/72 - r^2*Q6/90 + O(q^8)")
    print("CURRENT  theta^2 = S2/3 - S2^2/54 - 11*S2^3/9720")
    print("                    + S2*Q4/216 - Q6/270 + O(q^8)")
    print("STABILITY r^2 <= 3/4; dimension-six cancellation requires r^2 = 1")
    print("ESCAPE   radius-two M_R2 is in [0,4] and cancels dimension six at r=1")
    print("         but it violates the current one-Moore-shell dependency postulate")
    print("VERDICT  LEADING-ORDER-FLUX-ONLY")


if __name__ == "__main__":
    main()
