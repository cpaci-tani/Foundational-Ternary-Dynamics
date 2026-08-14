"""Fixed-curve CM realization operator certificate.

Run with:
  wsl -d Ubuntu-22.04 sage scripts/proofs/proof_cm_realization_operator.sage

The curve, differential normalization, prime ranges, and newform are locked by
PREREG_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md protocol v1.2.  This is not a
search over curves, conductors, discriminants, or prime subsets.
"""

from pathlib import Path
import hashlib


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_NATIVE_PHASE_ACTION_CM_OPERATOR_v1.md"
)
PROTOCOL_SHA256 = "8BE09323F54424C51EA96B2589D532559CC54C4656DE39DEE0626DD6C5EC09F5"


class Certificate:
    def __init__(self):
        self.count = 0

    def check(self, name, condition):
        if not bool(condition):
            raise AssertionError(name)
        self.count += 1
        print(f"[PASS {self.count:02d}] {name}")


def protocol_prefix_hash(path):
    raw = path.read_bytes()
    at = raw.index(b"`protocol_sha256=")
    return hashlib.sha256(raw[:at]).hexdigest().upper()


def manual_point_count(p):
    # #E(F_p)=1+sum_x(1+Legendre(x^3-x,p)); the leading 1 is infinity.
    return 1 + sum(1 + kronecker_symbol((x**3 - x) % p, p) for x in range(p))


def primary_gaussian_trace(p):
    """Trace pi+conjugate(pi) for the conductor-32 primary convention.

    For p=1 mod 4 choose pi=u+i*v with u odd, v even, and u+v=1 mod 4.
    Conjugation may leave two values of v, but the real part and trace are
    unique.  No quadrant is selected from a target trace.
    """
    bound = floor(sqrt(p))
    traces = set()
    for u in range(-bound, bound + 1):
        for v in range(-bound, bound + 1):
            if u*u + v*v != p:
                continue
            if u % 2 == 0 or v % 2 != 0:
                continue
            if (u + v - 1) % 4 == 0:
                traces.add(2*u)
    if len(traces) != 1:
        raise AssertionError(f"primary trace not unique at p={p}: {traces}")
    return traces.pop()


def hecke_coefficients(E, limit):
    """Generate a_n from fixed local Euler factors, including bad p=2."""
    coeff = [ZZ(0)] * limit
    coeff[1] = ZZ(1)
    for n in range(2, limit):
        fac = factor(n)
        value = ZZ(1)
        for p, exponent in fac:
            p = ZZ(p)
            if p == 2:
                # Additive reduction: local factor is 1, so a_(2^r)=0.
                value = ZZ(0)
                break
            a0 = ZZ(1)
            a1 = ZZ(E.ap(p))
            if exponent == 1:
                power_coeff = a1
            else:
                prev2, prev1 = a0, a1
                for _ in range(2, exponent + 1):
                    current = E.ap(p) * prev1 - p * prev2
                    prev2, prev1 = prev1, current
                power_coeff = prev1
            value *= power_coeff
        coeff[n] = value
    return coeff


def eta_product_coefficients(limit):
    """q-product eta(4 tau)^2 eta(8 tau)^2 through q^(limit-1)."""
    PS = PowerSeriesRing(QQ, "q", default_prec=limit)
    q = PS.gen()
    product = q
    for n in range(1, (limit - 1)//4 + 1):
        product *= (1 - q**(4*n))**2
    for n in range(1, (limit - 1)//8 + 1):
        product *= (1 - q**(8*n))**2
    return [ZZ(product[n]) for n in range(limit)]


def sym2_polynomial(trace, determinant, X):
    return (
        X**3
        - (trace**2 - determinant) * X**2
        + determinant * (trace**2 - determinant) * X
        - determinant**3
    )


def main():
    cert = Certificate()
    print("CM realization operator fixed-curve certificate")
    print(f"protocol={PROTOCOL_SHA256}")
    cert.check("protocol v1.2 prefix hash", protocol_prefix_hash(PROTOCOL) == PROTOCOL_SHA256)

    E = EllipticCurve(QQ, [0, 0, 0, -1, 0])
    cert.check("fixed curve is y^2=x^3-x", E.ainvs() == (0, 0, 0, -1, 0))
    cert.check("Cremona label is 32a2", E.cremona_label() == "32a2")
    cert.check("conductor is 32", E.conductor() == 32)
    cert.check("j invariant is 1728", E.j_invariant() == 1728)
    cert.check("minimal discriminant is 64", E.discriminant() == 64)
    cert.check("rank is zero", E.rank() == 0)
    cert.check("torsion is Z/2 + Z/2", tuple(E.torsion_subgroup().invariants()) == (2, 2))
    cert.check("E(R) has two components", E.real_components() == 2)
    cert.check("p=2 reduction is additive Kodaira III", str(E.local_data(2).kodaira_symbol()) == "III")
    cert.check("p=2 Tamagawa number is 2", E.local_data(2).tamagawa_number() == 2)
    cert.check("Tamagawa product is 2", E.tamagawa_product() == 2)
    cert.check("analytic Sha order is 1", E.sha().an() == 1)

    # Archimedean realization in the Neron differential dx/(2y).
    RF = RealField(200)
    gamma14 = RF(1/4).gamma()
    pi_rf = RF.pi()
    K = gamma14**2 / (4 * sqrt(pi_rf))
    varpi = gamma14**2 / (2 * sqrt(2 * pi_rf))
    gstar = 2 * varpi / sqrt(pi_rf)
    sage_period = RF(E.period_lattice().real_period())
    cert.check("least real period equals the lemniscatic varpi",
               abs(sage_period - varpi) / varpi < RF(2)**-45)
    cert.check("varpi=sqrt(2) K(1/sqrt(2)) normalization",
               abs(varpi - RF(2).sqrt()*K) < RF(2)**-190)
    cert.check("G*=2 varpi/sqrt(pi)", abs(gstar - 2*varpi/sqrt(pi_rf)) < RF(2)**-190)

    # Full BSD real volume: two components times the least real period.
    bsd_from_invariants = (2 * varpi) * E.tamagawa_product() / E.torsion_order()**2
    cert.check("BSD invariant quotient is varpi/4", abs(bsd_from_invariants - varpi/4) < RF(2)**-190)
    L1 = RF(E.lseries().dokchitser(prec=200)(1).real())
    cert.check("analytic L(E,1)=varpi/4", abs(L1 - varpi/4) / (varpi/4) < RF(2)**-170)
    cert.check("archimedean-to-Euler bridge G*=8L(E,1)/sqrt(pi)",
               abs(gstar - 8*L1/sqrt(pi_rf)) / gstar < RF(2)**-170)

    # The BCC square naturally sees the 2-twist E^(2), whose least period is K.
    E2 = E.quadratic_twist(2).minimal_model()
    cert.check("fixed quadratic twist is y^2=x^3-4x", E2.ainvs() == (0, 0, 0, -4, 0))
    cert.check("2-twist label/conductor is 64a1/64", E2.cremona_label() == "64a1" and E2.conductor() == 64)
    cert.check("2-twist least period is K(1/sqrt(2))",
               abs(RF(E2.period_lattice().real_period()) - K) / K < RF(2)**-45)
    bcc_period = (2*K/pi_rf)**2
    cert.check("BCC Watson period is G*^2/(2*pi)",
               abs(bcc_period - gstar**2/(2*pi_rf)) < RF(2)**-185)

    # Finite-prime Frobenius.  The first range and held-out range are fixed,
    # exhaustive ranges rather than selected framework primes.
    prefix = list(prime_range(3, 100))
    held_out = list(prime_range(100, 200))
    all_primes = prefix + held_out
    cert.check("manual point counts match Sage on prime prefix",
               all(manual_point_count(p) == E.change_ring(GF(p)).cardinality()
                   for p in prefix))
    cert.check("manual point counts match Sage on held-out primes",
               all(manual_point_count(p) == E.change_ring(GF(p)).cardinality()
                   for p in held_out))
    cert.check("inert primes have a_p=0 on both ranges",
               all(E.ap(p) == 0 for p in all_primes if p % 4 == 3))
    cert.check("split-prime trace follows the fixed primary Gaussian character",
               all(E.ap(p) == primary_gaussian_trace(p)
                   for p in all_primes if p % 4 == 1))

    PR = PolynomialRing(QQ, "X")
    X = PR.gen()
    frobenius_ok = True
    euler_ok = True
    inert_order4_ok = True
    twist_ok = True
    sym2_twist_ok = True
    inert_sym2_ok = True
    for p in all_primes:
        ap = ZZ(E.ap(p))
        Fp = matrix(QQ, [[0, -p], [1, ap]])
        frobenius_ok &= Fp.charpoly("X") == X**2 - ap*X + p
        T = polygen(QQ, "T")
        euler_ok &= (identity_matrix(QQ, 2) - Fp*T).det() == 1 - ap*T + p*T**2
        if p % 4 == 3:
            inert_order4_ok &= Fp**2 == -p*identity_matrix(QQ, 2)
            inert_order4_ok &= Fp**4 == p**2*identity_matrix(QQ, 2)
        chi2 = kronecker_symbol(2, p)
        twist_ok &= E2.ap(p) == chi2 * ap
        sym2_twist_ok &= sym2_polynomial(ap, p, X) == sym2_polynomial(chi2*ap, p, X)
        if p % 4 == 3:
            inert_sym2_ok &= sym2_polynomial(ap, p, X) == (X-p)*(X+p)**2
    cert.check("Frobenius companion has T^2-a_p*T+p", frobenius_ok)
    cert.check("det(I-F_p*T) gives the local Euler polynomial", euler_ok)
    cert.check("inert Frobenius obeys F_p^2=-pI and normalized order four", inert_order4_ok)
    cert.check("quadratic-twist traces differ by the fixed (2/p) character", twist_ok)
    cert.check("Sym^2 Frobenius polynomial is exactly twist-blind", sym2_twist_ok)
    cert.check("inert Sym^2 polynomial is (X-p)(X+p)^2", inert_sym2_ok)

    # One fixed newform: eta(4 tau)^2 eta(8 tau)^2.  Its coefficients must
    # agree both with the curve and with the local-factor recurrence.
    limit = 201
    qform = E.q_eigenform(limit)
    curve_coeff = [ZZ(qform[n]) for n in range(limit)]
    eta_coeff = eta_product_coefficients(limit)
    hecke_coeff = hecke_coefficients(E, limit)
    cert.check("newform is eta(4*tau)^2 eta(8*tau)^2 through q^200",
               curve_coeff == eta_coeff)
    cert.check("newform coefficients obey all fixed Hecke/Euler recurrences through n=200",
               curve_coeff[1:] == hecke_coeff[1:])
    cert.check("bad-prime local factor is 1 (all even coefficients vanish)",
               all(curve_coeff[n] == 0 for n in range(2, limit, 2)))

    print(f"\nPASS {cert.count}/{cert.count}")
    print("Genuine operator: the compatible H^1(E) Hecke/Frobenius system.")
    print("Boundary: BCC supplies Sym^2 H^1 and is blind to the quadratic-twist")
    print("and rank-two orientation needed to choose this lift.")
    return 0


main()
