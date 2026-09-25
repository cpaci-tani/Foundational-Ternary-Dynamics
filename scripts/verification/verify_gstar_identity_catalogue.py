"""Canonical 52-row audit; numerical evidence is not an analytic proof.

Edit this tracked file and gstar_identity_catalogue.py. Invocation:
    python scripts/verification/verify_gstar_identity_catalogue.py
    python scripts/verification/verify_gstar_identity_catalogue.py --output-dir PATH
The canonical invocation deterministically generates portable identity_catalogue.py
and verify_catalogue.py from these two sources before writing ATLAS.md and the
report. The generated verifier runs standalone with the reviewed paper at
paper/PAPER_GSTAR_IDENTITIES.tex (or beside the verifier).

Run with Python + mpmath + sympy. No fitted parameters or near-miss search.
The displayed TeX is a manually reviewed transcription, not machine-parsed math.
The reviewed paper's expected digest is locked in identity_catalogue.py; changing
the source without renewing that review fails this check. A standalone extracted
package can contain PAPER_GSTAR_IDENTITIES.tex beside this script.
"""

from __future__ import annotations

from collections import Counter
from datetime import datetime, timezone
import argparse
import hashlib
import json
from pathlib import Path
import re

import mpmath as mp
import sympy as sp

try:
    from gstar_identity_catalogue import CATALOGUE, FAMILIES, EXPECTED_PAPER_SHA256, render_atlas
except ModuleNotFoundError as error:
    if error.name != "gstar_identity_catalogue":
        raise
    from identity_catalogue import CATALOGUE, FAMILIES, EXPECTED_PAPER_SHA256, render_atlas

PRECISION = 110
mp.mp.dps = PRECISION
TOL = mp.mpf("1e-90")
HERE = Path(__file__).resolve().parent
IS_CANONICAL = Path(__file__).name == "verify_gstar_identity_catalogue.py"
REPO_ROOT = HERE.parents[1] if IS_CANONICAL else next(
    (parent for parent in HERE.parents if (parent / "docs/papers/src/PAPER_GSTAR_IDENTITIES.tex").is_file()), None)
DEFAULT_OUTPUT = REPO_ROOT / "dissemination/interactive/gstar-introduction" if IS_CANONICAL else HERE


def sync_portable(output: Path) -> None:
    """Generate portable copies from canonical bytes; never reverse-sync edits."""
    if not IS_CANONICAL:
        return
    for source_name, destination_name in (
        ("gstar_identity_catalogue.py", "identity_catalogue.py"),
        ("verify_gstar_identity_catalogue.py", "verify_catalogue.py"),
    ):
        header = ("# GENERATED from scripts/verification/" + source_name + "; edit the canonical source.\n").encode("utf-8")
        (output / destination_name).write_bytes(header + (HERE / source_name).read_bytes())
    # Keep --output-dir runnable outside the repository: this is a byte-for-byte
    # source attachment, never a second editable version of the paper.
    paper_output = output / "paper/PAPER_GSTAR_IDENTITIES.tex"
    paper_output.parent.mkdir(parents=True, exist_ok=True)
    paper_output.write_bytes((REPO_ROOT / "docs/papers/src/PAPER_GSTAR_IDENTITIES.tex").read_bytes())


def number(value: object, digits: int = 18) -> str:
    return mp.nstr(value, digits)


def coefficients(count: int) -> tuple[list[int], list[int]]:
    """Exact eta coefficients and independent finite-field point counts.

    eta(4tau)^2 eta(8tau)^2 = q prod(1-q^(4m))²(1-q^(8m))².
    Euler coefficients of y²=x³-x use a_p=p+1-#E(F_p), a_2=0,
    a_(p^r)=a_p*a_(p^(r-1))-p*a_(p^(r-2)), and multiplicativity.
    """
    product = [1] + [0] * (count-1)
    for stride in (4, 8):
        for exponent in range(stride, count, stride):
            for _ in range(2):
                for n in range(count-1, exponent-1, -1):
                    product[n] -= product[n-exponent]
    eta_coeff = [0] + product
    curve_coeff = [0] * (count+1)
    curve_coeff[1] = 1
    prime_values: dict[int, int] = {2: 0}
    for prime in sp.primerange(3, count+1):
        # A zero RHS has one solution; residues two; nonresidues none.
        character_sum = 0
        for x in range(prime):
            residue = (x*x*x-x) % prime
            symbol = pow(residue, (prime-1)//2, prime)
            character_sum += -1 if symbol == prime-1 else symbol
        prime_values[prime] = -character_sum
    for n in range(2, count+1):
        value = 1
        for prime, exponent in sp.factorint(n).items():
            if prime == 2:
                value = 0
                break
            old, term = 1, prime_values[prime]
            for _ in range(2, exponent+1):
                old, term = term, prime_values[prime]*term-prime*old
            value *= term
        curve_coeff[n] = value
    return eta_coeff, curve_coeff


def main(output_dir: Path | None = None) -> int:
    output = (output_dir or DEFAULT_OUTPUT).resolve()
    output.mkdir(parents=True, exist_ok=True)
    sync_portable(output)
    symbolic: list[dict[str, object]] = []
    auxiliary: list[dict[str, object]] = []
    results: list[dict[str, object]] = []

    def exact(name: str, passed: object, method: str) -> None:
        symbolic.append(dict(name=name, passed=bool(passed), method=method))

    def auxiliary_close(name: str, actual: object, expected: object, method: str,
                        tolerance: mp.mpf = TOL) -> None:
        error = abs(actual-expected)
        auxiliary.append(dict(name=name, passed=bool(error<tolerance),
                              residual=number(error), tolerance=number(tolerance), method=method))

    ids = [str(item["id"]) for item in CATALOGUE]
    exact("catalogue_has_52_unique_IDs", len(ids)==52 and len(set(ids))==52,
          "Count the canonical catalogue IDs, rejecting duplicates.")
    exact("catalogue_has_12_families", len(FAMILIES)==12 and set(FAMILIES)=={x["family"] for x in CATALOGUE},
          "Compare declared and used family keys.")
    portable_candidates = [HERE / "paper/PAPER_GSTAR_IDENTITIES.tex", HERE / "PAPER_GSTAR_IDENTITIES.tex"]
    repo_candidates = [REPO_ROOT / "docs/papers/src/PAPER_GSTAR_IDENTITIES.tex"] if REPO_ROOT is not None else []
    candidates = repo_candidates + portable_candidates if IS_CANONICAL else portable_candidates + repo_candidates
    paper = next((p for p in candidates if p.is_file()), candidates[-1])
    if not paper.is_file():
        raise FileNotFoundError("The reviewed paper is required for the source-binding check.")
    raw = paper.read_bytes()
    source = raw.decode("utf-8")
    source_hash = hashlib.sha256(raw).hexdigest()
    exact("reviewed_paper_SHA256", source_hash == EXPECTED_PAPER_SHA256,
          "Compare actual source bytes against the independently locked review digest; any source drift fails.")
    # Only labels and table IDs are parsed. Mathematical LaTeX is not evaluated.
    tags = re.findall(r"\\tag\{([A-J][0-9]+)\}", source)
    table = re.findall(r"^([A-J][0-9]+)\s*&", source, flags=re.MULTILINE)
    exact("paper_equation_ID_coverage", Counter(tags)==Counter(ids),
          "Every canonical ID must occur exactly once as an equation tag.")
    exact("paper_table_ID_coverage", Counter(table)==Counter(ids),
          "Every canonical ID must occur exactly once as a summary-table row.")

    quarter, half = sp.Rational(1,4), sp.Rational(1,2)
    exact("gamma_reflection", sp.simplify(sp.gamma(quarter)*sp.gamma(3*quarter)-sp.sqrt(2)*sp.pi)==0,
          "Sympy exact reflection at 1/4.")
    exact("gamma_recurrence", sp.expand_func(sp.gamma(5*quarter))-sp.gamma(quarter)/4==0,
          "Sympy exact recurrence Gamma(5/4)=Gamma(1/4)/4.")
    exact("beta_conversion", sp.simplify(sp.beta(quarter,quarter).rewrite(sp.gamma)-sp.gamma(quarter)**2/sp.sqrt(sp.pi))==0,
          "Exact beta-to-gamma formula.")
    k_sym, e_sym = sp.symbols("K E", positive=True)
    exact("Legendre_coefficient", sp.cancel(8*k_sym**2/(2*k_sym*(2*e_sym-k_sym))-4*k_sym/(2*e_sym-k_sym))==0,
          "Substitute pi=2K(2E-K) in (G*)²=8K²/pi.")
    exact("BCC_integral_coefficient", sp.simplify(2*sp.pi/(2*sp.pi)**3-1/(4*sp.pi**2))==0,
          "The normalized cube integral has volume (2pi)³.")

    def gamma_ratio_log_coeff(a: sp.Rational, b: sp.Rational, n: int) -> sp.Expr:
        return sp.simplify((-1)**(n+1)*(sp.bernoulli(n+1,a)-sp.bernoulli(n+1,b))/(n*(n+1)))

    exact("Wallis_pi_log_coefficients", [gamma_ratio_log_coeff(sp.Integer(1),half,n) for n in range(1,5)] == [sp.Rational(1,8),0,-sp.Rational(1,192),0],
          "Bernoulli-polynomial gamma-ratio expansion: log(Spi/sqrt(pi))=1/(8N)-1/(192N³)+O(N^-5).")
    exact("Wallis_G_log_coefficients", [gamma_ratio_log_coeff(3*quarter,quarter,n) for n in range(1,5)] == [0,sp.Rational(1,64),0,-sp.Rational(5,2048)],
          "N factors indexed 0..N-1: log(SG/G*)=1/(64N²)-5/(2048N⁴)+O(N^-6).")

    qtr = mp.mpf(1)/4
    g1, g2 = mp.gamma(qtr), mp.sqrt(mp.pi)
    g = g1/mp.gamma(3*qtr)
    # x=1-z² removes the integrable endpoint singularity exactly.
    quartic = mp.quad(lambda z: 2/mp.sqrt(4-6*z*z+4*z**4-z**6), [0,1])
    varpi = 2*quartic
    targets = dict(g=g, sqrt_g=mp.sqrt(g), inverse_g=1/g, sqrt_pi=g2,
                   varpi=varpi, g2=g*g, pi=mp.pi, g3=g**3)
    laplace_quarter = 4*mp.quad(lambda z: mp.exp(-z**4), [0,1,mp.inf])
    laplace_half = 2*mp.quad(lambda z: mp.exp(-z*z), [0,1,mp.inf])
    k = mp.quad(lambda t: 1/mp.sqrt(1-mp.sin(t)**2/2), [0,mp.pi/2])
    e = mp.quad(lambda t: mp.sqrt(1-mp.sin(t)**2/2), [0,mp.pi/2])
    ki = mp.quad(lambda t: 1/mp.sqrt(1+mp.sin(t)**2), [0,mp.pi/2])
    agm_a, agm_b, telescoping = mp.mpf(1), mp.sqrt(2), mp.mpf(1)
    for _ in range(8):
        telescoping *= 2/(1+agm_b/agm_a)
        agm_a, agm_b = (agm_a+agm_b)/2, mp.sqrt(agm_a*agm_b)
    agm = (agm_a+agm_b)/2
    # At 110 dps the eight-step gap rounds to zero. Re-evaluate only this
    # inexpensive iteration at 400 dps, rather than reporting zero truncation
    # error. This remains floating-point evidence, not interval arithmetic.
    with mp.workdps(400):
        bound_a, bound_b = mp.mpf(1), mp.sqrt(2)
        for _ in range(8):
            bound_a, bound_b = (bound_a+bound_b)/2, mp.sqrt(bound_a*bound_b)
        agm_limit_bound = 2*mp.sqrt(mp.pi)*abs(1/bound_a-1/bound_b)
    auxiliary_close("AGM_telescoping_finite_product", telescoping, 1/agm_a,
                    "For 8 steps, product 2/(1+b_n/a_n)=a0/a8=1/a8.")
    auxiliary_close("AGM_bracket_width", abs(agm_a-agm_b), mp.mpf(0),
                    "The common AGM limit lies between a8 and b8.")
    q = mp.exp(-mp.pi)
    theta_sum = 1+2*mp.fsum(q**(n*n) for n in range(1,11))
    theta_library = mp.jtheta(3,0,q)
    theta_tail = 2*q**121/(1-q**23)
    eta = mp.exp(-mp.pi/12)*mp.fprod(1-q**(2*n) for n in range(1,101))
    eta_log_tail = q**202/((1-q*q)*(1-q**202))
    exact("theta_eta_tail_bounds", mp.sqrt(2*mp.pi)*(2*theta_sum*theta_tail+theta_tail**2)<TOL and eta_log_tail<TOL,
          "Explicit omitted tails of the Gaussian theta sum and eta logarithmic product are below the numerical tolerance; floating-point rounding is separate.")

    # Integrate the BCC denominator over w, then over v using its defining
    # elliptic integral: W=(4/pi²)int_0^(pi/2) K(cos u)du. Carlson RF avoids
    # catastrophic cancellation in 1-cos²u near the logarithmic endpoint.
    watson = 4/mp.pi**2 * mp.quad(lambda u: mp.elliprf(0,mp.sin(u)**2,1), [0,mp.pi/4,mp.pi/2])
    full_bcc_integral = (2*mp.pi)**3*watson
    auxiliary_close("Watson_gamma_evaluation", watson, g1**4/(4*mp.pi**3),
                    "One-dimensional quadrature of the analytically reduced BCC triple integral versus gamma evaluation.")

    eta_coeff, curve_coeff = coefficients(300)
    exact("curve_eta_coefficients", eta_coeff==curve_coeff,
          "Compare 300 exact coefficients of eta(4tau)^2 eta(8tau)^2 against finite-field counts and Euler recurrences for y²=x³−x. Finite agreement is a consistency check; modular identification uses the cited theorem.")
    curve_q = mp.exp(-2*mp.pi/mp.sqrt(32))
    l_curve = 2*mp.fsum(mp.mpf(curve_coeff[n])*curve_q**n/n for n in range(1,301))
    l_tail = 4*curve_q**301/(1-curve_q)
    # For root number +1 at conductor 32, split Mellin integration at 1/sqrt(32).
    # Hasse/multiplicativity gives |a_n|<=d(n)sqrt(n)<=2n; hence this tail bound.
    auxiliary_close("specified_curve_L_value", l_curve, varpi/4,
                    "Accelerated functional-equation series with independently counted curve coefficients. Analytic reference: Li-Long-Tu (2018), Lemma3.1.")
    exact("curve_L_tail_below_tolerance", l_tail<TOL,
          "Omitted exponential tail <=4 r^301/(1-r), from |a_n|<=2n.")
    # x=1/t² gives int_1^infty dx/sqrt(x³-x)=2 int_0^1dt/sqrt(1-t^4).
    omega = 2*mp.quad(lambda a: 1/mp.sqrt(1+mp.sin(a)**2), [0,mp.pi/2])
    auxiliary_close("curve_positive_real_period", omega, varpi,
                    "Regularized real period integral after t=sin(a); primitive period for dx/(2y).")

    lattice_counts = [0]*101
    for a in range(-10,11):
        for b in range(-10,11):
            norm = a*a+b*b
            if norm<=100:
                lattice_counts[norm] += 1
    lattice_series = mp.fsum(lattice_counts[n]*q**n for n in range(101))
    r2_tail = 12*q**101*(102/(1-q)+q/(1-q)**2)
    lambert = 1+4*mp.fsum((1 if d%4==1 else -1)*q**d/(1-q**d) for d in range(1,102,2))
    jacobi_product = mp.fprod((1-q**(2*n))**2*(1+q**(2*n-1))**4 for n in range(1,101))
    lambert_tail = 4*q**103/((1-q*q)*(1-q**103))
    jacobi_log_tail = 2*q**202/((1-q*q)*(1-q**202))+4*q**201/(1-q*q)
    exact("arithmetic_series_tail_bounds", mp.sqrt(2*mp.pi)*max(r2_tail,lambert_tail,jacobi_product*mp.expm1(jacobi_log_tail))<TOL,
          "Geometric majorants bound the omitted two-square, Lambert and Jacobi-product tails; floating-point rounding is separate.")
    for n in range(1,101):
        expected = 4*sum(0 if d%2==0 else (1 if d%4==1 else -1) for d in sp.divisors(n))
        if lattice_counts[n] != expected:
            break
    else:
        n = 0
    exact("two_square_counts", n==0,
          "Direct integer-pair enumeration equals Jacobi's divisor formula for n=1..100; the zero coefficient is1.")

    values = {
        "A1":g1*g1/(mp.sqrt(2)*g2*g2), "A2":mp.sqrt(2)*g1*g1/(2*mp.pi),
        "A3":mp.beta(qtr,qtr)/mp.sqrt(2*mp.pi), "A4":g1*g1/(mp.sqrt(2)*mp.gamma(mp.mpf(1)/2)**2),
        "A5":(g1*g1/(g2*g2))/mp.sqrt(2), "A6":mp.gamma(qtr)/mp.gamma(3*qtr),
        "A7":g2*g2*mp.sqrt(2)/mp.gamma(3*qtr)**2, "A8":16*mp.gamma(5*qtr)**2/(mp.sqrt(2)*g2*g2),
        "A9":laplace_quarter**2/(mp.sqrt(2)*laplace_half**2), "A10":g1/(2**qtr*g2), "A11":mp.gamma(3*qtr)/g1,
        "B1":2*varpi/g2, "B2":2*varpi/mp.sqrt(mp.pi), "B3":varpi/mp.sqrt(mp.pi/4), "B4":4*quartic/mp.sqrt(mp.pi),
        "C1":2*mp.sqrt(2)*k/g2, "C2":2*mp.sqrt(2)*k/mp.sqrt(mp.pi), "C3":4*ki/mp.sqrt(mp.pi),
        "C4":2*mp.sqrt(k/(2*e-k)), "C5":4*k/(2*e-k),
        "D1":2*g2/agm, "D2":2*mp.sqrt(mp.pi)/agm,
        "D3":mp.sqrt(2)*g2/mp.agm(1,1/mp.sqrt(2)), "D4":2*g2*telescoping,
        "E1":g2*mp.sqrt(2)*theta_library**2, "E2":mp.sqrt(2*mp.pi)*theta_library**2,
        "E3":g2*mp.sqrt(2)*mp.fsum(q**(n*n) for n in range(-10,11))**2,
        "E4":g2*mp.sqrt(2)*theta_sum**2, "E5":g2*mp.sqrt(2)*(1+2*q)**2,
        "E6":2*g2*mp.sqrt(2)*eta**2, "E7":2*mp.sqrt(2*mp.pi)*eta**2,
        "F1":mp.sqrt(2*g2*g2*watson), "F2":mp.sqrt(2*mp.pi*watson), "F3":2*g2*g2*watson,
        "F4":full_bcc_integral/(4*mp.pi**2), "G1":8*l_curve/g2, "G2":8*l_curve/mp.sqrt(mp.pi),
        "G3":8*l_curve/mp.sqrt(mp.pi), "G4":2*omega/g2,
        "H1":mp.sqrt(2*mp.pi)*mp.hyp2f1(mp.mpf(1)/2,mp.mpf(1)/2,1,mp.mpf(1)/2),
        "H2":g2*mp.sqrt(2)*lattice_series, "H3":g2*mp.sqrt(2)*lambert, "H4":g2*mp.sqrt(2)*jacobi_product,
        "H5":16/(mp.sqrt(2)*g2*g2)*(laplace_quarter/4)**2,
        "I1":4*varpi**2/g**2, "I2":g*g2/2, "I3":4*varpi**2/(g*g2*g2), "I4":16*varpi**4/(g*g2**4),
        "J1":mp.sqrt(2*mp.pi)*theta_library**4/theta_library**2,
    }
    methods = {
        "A":"Gamma/beta evaluation; A9 independently integrates regularized Laplace integrals. A6 is the defining anchor, not an independent verification.",
        "B":"Regularized quadrature of int_0^1(1-x^4)^(-1/2)dx; compare with the gamma quotient.",
        "C":"Independent quadratures of the defining K and E integrals; modulus k0²=1/2 and i²=-1.",
        "D":"Eight explicit AGM iterations, with the final upper/lower bracket and exact telescoping product checked separately.",
        "E":"mpmath theta evaluation and separately truncated Gaussian sums with a recorded tail bound.",
        "E-prime":"Independent eta product with 100 factors and explicit omitted-log-tail bound.",
        "F":"BCC cube integral analytically reduced by two elementary/elliptic integrations, then independently quadrature-evaluated; not assigned its gamma closed form.",
        "G":"Finite-field point counts and Euler recurrence coefficients; 300-term functional-equation L-series with rigorous analytic tail bound. G4 uses real-period quadrature.",
        "H":"Direct hypergeometric evaluation, exact integer-pair counts, character Lambert sum, Jacobi product, or regularized quartic exponential quadrature as appropriate.",
        "I":"Algebraic restatement checked with independently integrated varpi and the defining gamma quotient.",
        "J":"Explicit quotient of theta values; this restates E2, not a second derivation.",
    }
    for item in CATALOGUE:
        id = str(item["id"])
        target = targets[item["target"]]
        record = dict(id=id, kind=item["kind"], target=item["lhs"], target_value=number(target,100))
        if id in ("A12","A13","A14"):
            probes = []
            finite_equal = True
            for n in (16,64,256):
                spi = mp.fprod(mp.mpf(2*k)/(2*k-1) for k in range(1,n+1))/mp.sqrt(n)
                sg = mp.fprod(mp.mpf(4*k+3)/(4*k+1) for k in range(n))/mp.sqrt(n)
                pi_gamma = g2*mp.gamma(n+1)/(mp.sqrt(n)*mp.gamma(n+mp.mpf(1)/2))
                g_gamma = g*mp.gamma(n+3*qtr)/(mp.sqrt(n)*mp.gamma(n+qtr))
                finite_equal &= abs(spi-pi_gamma)<TOL and abs(sg-g_gamma)<TOL
                actual = spi if id=="A12" else sg if id=="A13" else spi*sg/2
                probes.append(dict(N=n, actual=number(actual,32), residual=number(abs(actual-target)),
                                   signed_relative_error=number(actual/target-1)))
            expected_power = 2 if id=="A13" else 1
            ratios = [mp.mpf(probes[j]["residual"])/mp.mpf(probes[j+1]["residual"]) for j in range(2)]
            asymptotic_ok = all(abs(r/4**expected_power-1)<mp.mpf("0.03") for r in ratios)
            record.update(passed=bool(finite_equal and asymptotic_ok),
                          method="Direct finite products match exact gamma-ratio reductions. Bernoulli coefficients checked symbolically; fixed N probes check the stated convergence order, not equality of a finite product to its limit.",
                          finite_probes=probes, expected_error_order=f"O(N^-{expected_power})",
                          finite_product_gamma_identity_passed=bool(finite_equal),
                          error_reduction_ratios=[number(x) for x in ratios])
        else:
            actual = values[id]
            error = abs(actual-target)
            if id=="E5":
                s = 1+2*q
                tail_bound = 2*q**4/(1-q**5)
                upper = mp.sqrt(2*mp.pi)*(2*s*tail_bound+tail_bound**2)
                passed = actual<target and error<upper and error<mp.mpf("0.000038") and error/target<mp.mpf("0.000013")
                record.update(passed=bool(passed), method="For n>=2, successive q^(n²) terms have ratio at most q^5. Bound the omitted theta tail by 2q^4/(1-q^5), then propagate through the square.",
                              absolute_error_bound=number(upper), relative_error=number(error/target),
                              actual_value=number(actual,100), absolute_residual=number(error))
            else:
                record.update(passed=bool(error<TOL), actual_value=number(actual,100), absolute_residual=number(error),
                              absolute_tolerance=number(TOL), method=methods[item["family"]])
                if id=="D4":
                    record["finite_iterations"] = 8
                    record["analytical_bracket_evaluated_at_400_dps"] = number(agm_limit_bound)
                    record["bracket_qualification"] = "AGM inequalities give the analytic bound; the reported number is a floating-point evaluation, not an interval certificate. The 110-dps rounding error is controlled separately by the stated residual tolerance."
        results.append(record)

    failed = [x["id"] for x in results if not x["passed"]]
    failed_checks = [x["name"] for x in symbolic+auxiliary if not x["passed"]]
    report = dict(generated_utc=datetime.now(timezone.utc).isoformat(),
                  precision_decimal_digits=PRECISION,
                  interpretation="Numerical consistency checks and symbolic algebra, not a formal proof of the analytic identities. No numerical search or physical identification.",
                  source_binding=dict(paper=paper.name, sha256=source_hash, expected_sha256=EXPECTED_PAPER_SHA256,
                                      mathematical_transcription="Manually reviewed. Only equation and table IDs are parsed; TeX mathematics is not parsed or executed."),
                  libraries=dict(mpmath=mp.__version__,sympy=sp.__version__),
                  constants=dict(Gstar=number(g,100),varpi=number(varpi,100),W_BCC=number(watson,100),L_curve=number(l_curve,100),omega_positive=number(omega,100)),
                  truncation_bounds=dict(theta_sum_tail=number(theta_tail),eta_log_tail=number(eta_log_tail),two_square_sum_tail=number(r2_tail),lambert_sum_tail=number(lambert_tail),jacobi_log_tail=number(jacobi_log_tail),curve_L_series_tail=number(l_tail)),
                  sources=["https://dlmf.nist.gov/5.11.E13","https://dlmf.nist.gov/19.7.E2","https://dlmf.nist.gov/19.7.E1",
                           "https://dlmf.nist.gov/20.5.E3","https://dlmf.nist.gov/23.15.E9",
                           "https://arxiv.org/pdf/1803.06072 (Li, Long and Tu, Lemma 3.1)",
                           "https://www.lmfdb.org/EllipticCurve/Q/32/a/3",
                           "Watson, Three triple integrals, Q. J. Math. Oxford 10 (1939), 266-276"],
                  catalogue_results=results,symbolic_checks=symbolic,auxiliary_checks=auxiliary,
                  summary=dict(catalogue_passed=len(results)-len(failed),catalogue_total=len(results),
                               auxiliary_passed=len(symbolic+auxiliary)-len(failed_checks),auxiliary_total=len(symbolic+auxiliary),
                               failed_ids=failed,failed_checks=failed_checks))
    (output/"CATALOGUE_VERIFICATION.json").write_text(json.dumps(report,indent=2,ensure_ascii=True)+"\n",encoding="utf-8")
    (output/"ATLAS.md").write_text(render_atlas(),encoding="utf-8")
    print(f"Catalogue: {len(results)-len(failed)}/{len(results)} rows; support checks: {len(symbolic+auxiliary)-len(failed_checks)}/{len(symbolic+auxiliary)}.")
    for failure in failed+failed_checks:
        print("FAIL:",failure)
    return 1 if failed or failed_checks else 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, help="Directory for generated portable modules, ATLAS.md and CATALOGUE_VERIFICATION.json.")
    args = parser.parse_args()
    raise SystemExit(main(args.output_dir))
