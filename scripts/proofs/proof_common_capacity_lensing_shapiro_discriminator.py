#!/usr/bin/env python3
"""Exact weak common-capacity lensing and Shapiro discriminator.

The current FTD engine has one sourced latency well that affects material
clocks and selected slow-body motion but not the vacuum wave stencil.  This
certificate defines the minimum blind response tuple for a future common
action:

  a_m : slow-body response to the static capacity depth U,
  a_t : material-clock proper-time response,
  a_0 : temporal/lapse response of the wave principal symbol,
  a_s : spatial/Hodge response of the wave principal symbol.

For U=mu/r, exact weak-ray integration gives a normalized deflection and
Shapiro coefficient (a_0+a_s)/a_m.  The unknown source strength cancels.  The
classes 0, clock-medium/1911, and full equal temporal+spatial response are
therefore 0, 1, and 2 respectively.  Sharing one named capacity field does not
fix these operator coefficients; they must come from the common action.

This is a source-response target and obstruction theorem, not a lensing
derivation, a production refractive-index insertion, or a GR claim.
"""

from __future__ import annotations

from sympy import (
    Matrix,
    asinh,
    diff,
    limit,
    oo,
    simplify,
    sqrt,
    symbols,
)


def main() -> None:
    checks = 0
    u, ell = symbols("u ell")
    a_m, a_t, a_0, a_s = symbols("a_m a_t a_0 a_s", nonzero=True)
    mu, b, z = symbols("mu b z", positive=True)

    # FTD's registered clock-medium yardstick uses U=L^2/2.
    clock_index = 1 / sqrt(1 - ell**2)
    # Check the load-bearing weak coefficient without depending on a printed
    # series representation containing an Order term.
    assert limit((clock_index - 1) / (ell**2 / 2), ell, 0) == 1
    checks += 1

    proper_time_rate = sqrt(1 - 2 * a_t * u)
    assert limit((1 - proper_time_rate) / u, u, 0) == a_t
    clock_body_ratio = a_t / a_m
    checks += 1

    # Most general isotropic weak temporal/spatial optical response at first
    # order in the positive capacity depth U.
    coordinate_speed = sqrt((1 - 2 * a_0 * u) / (1 + 2 * a_s * u))
    refractive_index = 1 / coordinate_speed
    assert limit((refractive_index - 1) / u, u, 0) == a_0 + a_s
    checks += 1

    response = a_0 + a_s
    radius = sqrt(b**2 + z**2)
    weak_index = 1 + response * mu / radius
    transverse_gradient = diff(weak_index, b)
    assert simplify(
        transverse_gradient + response * mu * b / radius**3
    ) == 0
    checks += 1

    # Exact complete-ray antiderivative and deflection.
    bending_primitive = z / (b * radius)
    assert simplify(diff(bending_primitive, z) - b / radius**3) == 0
    bending_integral = limit(bending_primitive, z, oo) - limit(
        bending_primitive, z, -oo
    )
    assert bending_integral == 2 / b
    theta = -response * mu * bending_integral
    assert simplify(theta + 2 * response * mu / b) == 0
    checks += 3

    # Slow-body dynamics independently measures mu_m=a_m*mu, so the source
    # normalization cancels from the blind optical/dynamical ratio.
    mu_m = a_m * mu
    deflection_ratio = simplify(-b * theta / (2 * mu_m))
    assert deflection_ratio == (a_0 + a_s) / a_m
    checks += 1

    # Finite-endpoint one-way Shapiro excess has the same blind coefficient.
    z_left, z_right = symbols("z_left z_right", positive=True)
    shapiro_primitive = asinh(z / b)
    assert simplify(diff(shapiro_primitive, z) - 1 / radius) == 0
    geometric_log = asinh(z_left / b) + asinh(z_right / b)
    shapiro_delay = response * mu * geometric_log
    shapiro_ratio = simplify(shapiro_delay / (mu_m * geometric_log))
    assert shapiro_ratio == (a_0 + a_s) / a_m
    checks += 3

    # The three frozen-well response classes are exact substitutions, not a
    # scan or fit.  a_m is kept symbolic and cancels.
    classes = {
        "class_0": {a_t: a_m, a_0: 0, a_s: 0},
        "clock_medium_1911": {a_t: a_m, a_0: a_m, a_s: 0},
        "equal_temporal_spatial": {a_t: a_m, a_0: a_m, a_s: a_m},
    }
    expected = {"class_0": 0, "clock_medium_1911": 1, "equal_temporal_spatial": 2}
    for name, substitution in classes.items():
        assert simplify(deflection_ratio.subs(substitution)) == expected[name]
        assert simplify(shapiro_ratio.subs(substitution)) == expected[name]
        assert simplify(clock_body_ratio.subs(substitution)) == 1
        checks += 3

    # A common rescaling of the source depth cannot determine the ratio.
    scale = symbols("scale", positive=True)
    scaled_theta = theta.subs(mu, scale * mu)
    scaled_mu_m = mu_m.subs(mu, scale * mu)
    assert simplify(-b * scaled_theta / (2 * scaled_mu_m)) == deflection_ratio
    checks += 1

    # The response tuple contains independent symmetry-allowed coefficients.
    # Equal sourcing or one named U does not algebraically identify them.
    coefficient_jacobian = Matrix(
        [a_m * u, a_t * u, a_0 * u, a_s * u]
    ).jacobian(Matrix([a_m, a_t, a_0, a_s]))
    assert coefficient_jacobian.rank() == 4
    checks += 1

    print("weak capacity metric: n=1+(a_0+a_s)U+O(U^2)")
    print("point-depth ray: theta=-2(a_0+a_s)mu/b")
    print("blind deflection=blind Shapiro=(a_0+a_s)/a_m")
    print("clock/fall equivalence a_t/a_m is independent of wave response")
    print("response classes: unread=0, clock-medium=1, equal temporal+spatial=2")
    print("common source normalization cancels; shared field name does not fix ratios")
    print(
        "PASS: common-capacity lensing/Shapiro discriminator "
        f"({checks} exact checks)"
    )
    print(
        "Open: action-derived a_0/a_m and a_s/a_m, finite Maxwell-capacity "
        "operator, static solution, lensing fixture, and nonlinear gravity"
    )


if __name__ == "__main__":
    main()
