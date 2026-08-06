"""Exact symbolic certificate for the FTD-0657 classical pole boundary."""

import sympy as sp


def main() -> None:
    z, dt, c2, e0 = sp.symbols("z dt c2 e0", nonzero=True)
    inverse_mass = c2 / e0
    tangent = sp.Matrix([[1, dt * inverse_mass], [0, 1]])
    assert sp.factor((sp.eye(2) - z * tangent).det()) == (z - 1) ** 2
    assert tangent.eigenvals() == {sp.Integer(1): 2}

    # The additive rest constant differentiates away exactly.
    q, p, h0, stiffness = sp.symbols("q p h0 stiffness")
    hamiltonian = h0 + inverse_mass * p**2 / 2 + stiffness * q**2 / 2
    gradient = sp.Matrix([sp.diff(hamiltonian, q), sp.diff(hamiltonian, p)])
    hessian = sp.hessian(hamiltonian, (q, p))
    assert h0 not in gradient.free_symbols
    assert h0 not in hessian.free_symbols

    # The internal pole is controlled by stiffness/inertia, not h0.
    generator = sp.Matrix([[0, inverse_mass], [-stiffness, 0]])
    lam = sp.symbols("lambda")
    characteristic = sp.factor((lam * sp.eye(2) - generator).det())
    assert sp.simplify(characteristic - (lam**2 + c2 * stiffness / e0)) == 0
    assert h0 not in characteristic.free_symbols

    # A translated form factor carries only the convective phase k*v.
    k, velocity, time = sp.symbols("k velocity time", real=True)
    form_factor = sp.exp(-sp.I * k * velocity * time)
    assert sp.simplify(sp.I * sp.diff(form_factor, time) / form_factor) == k * velocity

    # A genuine classical massive field needs an explicit restoring term.
    omega, omega0, k2 = sp.symbols("omega omega0 k2")
    massive_denominator = omega**2 - c2 * k2 - omega0**2
    assert sp.solve(massive_denominator, omega**2) == [c2 * k2 + omega0**2]

    print("FTD-0657 exact certificate: PASS")


if __name__ == "__main__":
    main()
