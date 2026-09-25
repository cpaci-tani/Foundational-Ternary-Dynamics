"""Canonical, manually audited transcription of the paper's 52 labelled entries.

Edit this tracked source, not the generated portable identity_catalogue.py.
Run `python scripts/verification/verify_gstar_identity_catalogue.py` to refresh
the portable modules, atlas and numerical report deterministically.

LaTeX is presentation data, not parsed executable mathematics. verify_catalogue.py
checks ID coverage and records the exact source digest alongside independent
evaluations. Changes to the paper require renewed human mathematical review.
"""

from __future__ import annotations

# Locked after manual comparison with the repaired paper. The verifier refuses
# source changes until this review binding is deliberately renewed.
EXPECTED_PAPER_SHA256 = "7d4be0b42c6b28d7bc1341e17c8c3e0e0144ea711b485557035570530729358e"

FAMILIES = {
    "A": "Gamma and beta functions",
    "A-prime": "Wallis products",
    "B": "The lemniscate",
    "C": "Elliptic integrals",
    "D": "Arithmetic-geometric mean",
    "E": "Jacobi theta functions",
    "E-prime": "Dedekind eta function",
    "F": "BCC lattice Green function",
    "G": "The curve y² = x³ − x",
    "H": "Hypergeometric and arithmetic series",
    "I": "Structural algebraic relations",
    "J": "Modular forms",
}

NOTATION = r"""Throughout, \(g_1=\Gamma(1/4)\), \(g_2=\sqrt\pi\),
\(q=e^{-\pi}\), \(k_0=1/\sqrt2\), \(K=K(k_0)\), \(E_0=E(k_0)\),
and \(\varpi=2\int_0^1(1-x^4)^{-1/2}\,dx\).
The elliptic-integral argument is the modulus; Wolfram Language and mpmath
instead use its square, the parameter. Theta uses the nome \(q=e^{\pi i\tau}\),
whereas eta uses \(e^{2\pi i\tau}\). For the curve \(y^2=x^3-x\),
\(\Omega_+=\varpi\) is the least positive real period of \(dx/(2y)\);
the integral of its absolute value over both real components is \(2\Omega_+\).
\(W_{\rm BCC}=(2\pi)^{-3}\int_{[-\pi,\pi]^3}
(1-\cos u\cos v\cos w)^{-1}\,du\,dv\,dw\).
"""


def row(id: str, family: str, label: str, lhs: str, rhs: str,
        kind: str, target: str, explanation: str, dependencies: list[str],
        wolfram_rhs: str) -> dict[str, object]:
    return dict(id=id, family=family, label=label, lhs=lhs, rhs=rhs, kind=kind,
                target=target, explanation=explanation, dependencies=dependencies,
                wolfram_rhs=wolfram_rhs)


CATALOGUE = [
    row("A1", "A", "Gamma-square definition", r"G^*", r"\frac{g_1^2}{\sqrt2\,g_2^2}", "exact", "g", "Reflection turns a gamma quotient into a gamma square.", ["Euler reflection"], "Gamma[1/4]^2/(Sqrt[2] Pi)"),
    row("A2", "A", "Equivalent gamma square", r"G^*", r"\frac{\sqrt2\,\Gamma(1/4)^2}{2\pi}", "restatement", "g", "The same normalization with a rationalized coefficient.", ["A1"], "Sqrt[2] Gamma[1/4]^2/(2 Pi)"),
    row("A3", "A", "Beta integral", r"G^*", r"\frac{B(1/4,1/4)}{\sqrt{2\pi}}", "exact", "g", "Use B(a,b)=Gamma(a)Gamma(b)/Gamma(a+b).", ["Euler beta integral"], "Beta[1/4,1/4]/Sqrt[2 Pi]"),
    row("A4", "A", "Expanded gamma notation", r"G^*", r"\frac{\Gamma(1/4)^2}{\sqrt2\,\Gamma(1/2)^2}", "restatement", "g", "Expand g1 and g2 in A1.", ["A1"], "Gamma[1/4]^2/(Sqrt[2] Gamma[1/2]^2)"),
    row("A5", "A", "A named ratio", r"G^*", r"\frac{r}{\sqrt2},\qquad r=\frac{g_1^2}{g_2^2}", "restatement", "g", "Naming an intermediate ratio adds no independent identity.", ["A1"], "(Gamma[1/4]^2/Pi)/Sqrt[2]"),
    row("A6", "A", "The gamma quotient", r"G^*", r"\frac{\Gamma(1/4)}{\Gamma(3/4)}", "exact", "g", "The compact defining expression; reflection gives A1.", ["Definition"], "Gamma[1/4]/Gamma[3/4]"),
    row("A7", "A", "The reciprocal gamma square", r"G^*", r"\frac{\sqrt2\,g_2^2}{\Gamma(3/4)^2}", "restatement", "g", "Reflection replaces Gamma(1/4) in the quotient.", ["A6", "Euler reflection"], "Sqrt[2] Pi/Gamma[3/4]^2"),
    row("A8", "A", "Gamma recurrence", r"G^*", r"\frac{16\,\Gamma(5/4)^2}{\sqrt2\,g_2^2}", "restatement", "g", "Gamma(5/4)=Gamma(1/4)/4.", ["A1", "Gamma recurrence"], "16 Gamma[5/4]^2/(Sqrt[2] Pi)"),
    row("A9", "A", "Laplace integrals", r"G^*", r"\frac{(\int_0^\infty t^{-3/4}e^{-t}\,dt)^2}{\sqrt2\,(\int_0^\infty t^{-1/2}e^{-t}\,dt)^2}", "restatement", "g", "Substitute the defining gamma integrals into A1; substitutions remove endpoint singularities in the numerical check.", ["A1", "Euler gamma integral"], "Integrate[t^(-3/4) Exp[-t],{t,0,Infinity}]^2/(Sqrt[2] Integrate[t^(-1/2) Exp[-t],{t,0,Infinity}]^2)"),
    row("A10", "A", "Square root", r"\sqrt{G^*}", r"\frac{g_1}{2^{1/4}g_2}", "restatement", "sqrt_g", "Positive square roots of A1. The target is sqrt(G*).", ["A1"], "Gamma[1/4]/(2^(1/4) Sqrt[Pi])"),
    row("A11", "A", "Reciprocal", r"1/G^*", r"\frac{\Gamma(3/4)}{\Gamma(1/4)}", "restatement", "inverse_g", "Invert the positive quotient in A6.", ["A6"], "Gamma[3/4]/Gamma[1/4]"),
    row("A12", "A-prime", "Even and odd products", r"\sqrt\pi", r"\lim_{N\to\infty}N^{-1/2}\prod_{k=1}^{N}\frac{2k}{2k-1}", "limit", "sqrt_pi", "The finite product has a gamma-ratio expression; its relative correction starts at 1/(8N).", ["Gamma recurrence", "Gamma-ratio asymptotics"], "Limit[Product[2 k/(2 k-1),{k,1,n}]/Sqrt[n],n->Infinity]"),
    row("A13", "A-prime", "Residue classes modulo four", r"G^*", r"\lim_{N\to\infty}N^{-1/2}\prod_{k=0}^{N-1}\frac{4k+3}{4k+1}", "limit", "g", "All integers in the two residue classes enter; this is not a product over primes. With N factors the leading relative correction is 1/(64N²).", ["Gamma recurrence", "Gamma-ratio asymptotics"], "Limit[Product[(4 k+3)/(4 k+1),{k,0,n-1}]/Sqrt[n],n->Infinity]"),
    row("A14", "A-prime", "The product of two limits", r"\varpi", r"\frac12\lim_{N\to\infty}N^{-1}\prod_{k=0}^{N-1}\frac{4k+3}{4k+1}\prod_{k=1}^{N}\frac{2k}{2k-1}", "limit", "varpi", "Multiplication of convergent sequences gives G*sqrt(pi)/2; no arithmetic independence is asserted.", ["A12", "A13", "B2"], "Limit[Product[(4 k+3)/(4 k+1),{k,0,n-1}] Product[2 k/(2 k-1),{k,1,n}]/(2 n),n->Infinity]"),
    row("B1", "B", "Normalized lemniscatic period", r"G^*", r"\frac{2\varpi}{g_2}", "exact", "g", "The lemniscate half-perimeter is normalized by sqrt(pi).", ["Lemniscate arc-length integral", "A6"], "2 varpi/Sqrt[Pi]"),
    row("B2", "B", "The bridge formula", r"G^*", r"\frac{2\varpi}{\sqrt\pi}", "restatement", "g", "The same period normalization with g2 expanded.", ["B1"], "2 varpi/Sqrt[Pi]"),
    row("B3", "B", "Circle-in-square normalization", r"G^*", r"\frac{\varpi}{\sqrt{\pi/4}}", "restatement", "g", "An algebraic rewriting of B2 using the circle-in-square area ratio pi/4.", ["B2"], "varpi/Sqrt[Pi/4]"),
    row("B4", "B", "Quartic integral", r"G^*", r"\frac4{\sqrt\pi}\int_0^1\frac{dx}{\sqrt{1-x^4}}", "exact", "g", "The substitution t=x^4 identifies the integral with B(1/4,1/2)/4.", ["Euler beta integral"], "4/Sqrt[Pi] Integrate[1/Sqrt[1-x^4],{x,0,1}]"),
    row("C1", "C", "Self-complementary elliptic modulus", r"G^*", r"\frac{2\sqrt2}{g_2}K(k_0)", "exact", "g", "At k0=1/sqrt(2), the complementary modulus equals the original modulus.", ["Gauss elliptic evaluation"], "2 Sqrt[2/Pi] EllipticK[1/2]"),
    row("C2", "C", "Expanded elliptic normalization", r"G^*", r"\frac{2\sqrt2}{\sqrt\pi}K(k_0)", "restatement", "g", "The coefficient is 2sqrt(2), not 4.", ["C1"], "2 Sqrt[2/Pi] EllipticK[1/2]"),
    row("C3", "C", "Imaginary modulus", r"G^*", r"\frac4{\sqrt\pi}K(i)", "exact", "g", "K(i)=K(k0)/sqrt(2); K(i) is real and positive on its defining real integration path.", ["Imaginary-modulus transformation", "C1"], "4/Sqrt[Pi] EllipticK[-1]"),
    row("C4", "C", "Eliminating pi with Legendre", r"G^*", r"2\sqrt{\frac{K}{2E_0-K}}", "exact", "g", "Legendre's relation becomes K(2E0-K)=pi/2; the positive square root is used.", ["Legendre relation", "C1"], "2 Sqrt[EllipticK[1/2]/(2 EllipticE[1/2]-EllipticK[1/2])]"),
    row("C5", "C", "The squared elliptic ratio", r"(G^*)^2", r"\frac{4K}{2E_0-K}", "restatement", "g2", "Squaring C4 changes the target to (G*)².", ["C4"], "4 EllipticK[1/2]/(2 EllipticE[1/2]-EllipticK[1/2])"),
    row("D1", "D", "Gauss AGM", r"G^*", r"\frac{2g_2}{\operatorname{agm}(1,\sqrt2)}", "exact", "g", "Arithmetic and geometric averaging have one common positive limit.", ["Gauss AGM theorem", "C1"], "2 Sqrt[Pi]/ArithmeticGeometricMean[1,Sqrt[2]]"),
    row("D2", "D", "Expanded AGM normalization", r"G^*", r"\frac{2\sqrt\pi}{\operatorname{agm}(1,\sqrt2)}", "restatement", "g", "Expand g2 in D1.", ["D1"], "2 Sqrt[Pi]/ArithmeticGeometricMean[1,Sqrt[2]]"),
    row("D3", "D", "Homogeneous AGM", r"G^*", r"\frac{\sqrt2\,g_2}{\operatorname{agm}(1,1/\sqrt2)}", "restatement", "g", "AGM homogeneity supplies the previously missing sqrt(2).", ["D1", "AGM homogeneity"], "Sqrt[2 Pi]/ArithmeticGeometricMean[1,1/Sqrt[2]]"),
    row("D4", "D", "A telescoping AGM product", r"G^*", r"2\sqrt\pi\prod_{n=0}^{\infty}\frac2{1+r_n},\qquad r_n=b_n/a_n",
        "limit", "g", "Set a0=1,b0=sqrt(2), a_next=(a+b)/2, b_next=sqrt(ab). The finite product is exactly 1/a_N and tends to 1/AGM.", ["D1", "AGM recurrence"], "2 Sqrt[Pi]/First[Nest[({Total[#]/2,Sqrt[Times@@#]}&),{1,Sqrt[2]},8]]"),
    row("E1", "E", "The square-lattice theta value", r"G^*", r"g_2\sqrt2\,\theta_3(q)^2", "exact", "g", "Theta uses q=exp(-pi), the nome of tau=i.", ["Theta-elliptic identity", "C1"], "Sqrt[2 Pi] EllipticTheta[3,0,Exp[-Pi]]^2"),
    row("E2", "E", "Expanded theta normalization", r"G^*", r"\sqrt{2\pi}\,\theta_3(q)^2", "restatement", "g", "The same special value as E1.", ["E1"], "Sqrt[2 Pi] EllipticTheta[3,0,Exp[-Pi]]^2"),
    row("E3", "E", "A two-sided Gaussian sum", r"G^*", r"g_2\sqrt2\left(\sum_{n\in\mathbb Z}e^{-\pi n^2}\right)^2", "exact", "g", "The Gaussian sum is the definition of theta3 at the square-lattice nome.", ["E1", "Theta series"], "Sqrt[2 Pi] Sum[Exp[-Pi n^2],{n,-Infinity,Infinity}]^2"),
    row("E4", "E", "One-sided Gaussian sum", r"G^*", r"g_2\sqrt2\left(1+2\sum_{n=1}^{\infty}e^{-\pi n^2}\right)^2", "restatement", "g", "Pair positive and negative integers in E3.", ["E3"], "Sqrt[2 Pi] (1+2 Sum[Exp[-Pi n^2],{n,1,Infinity}])^2"),
    row("E5", "E", "A short approximation", r"G^*", r"\sqrt{2\pi}(1+2e^{-\pi})^2", "approximation", "g", "A lower approximation, with absolute error less than 0.000038 and relative error less than 0.000013. It is not an equality.", ["E4", "Geometric tail bound"], "Sqrt[2 Pi] (1+2 Exp[-Pi])^2"),
    row("E6", "E-prime", "Dedekind eta at the square lattice", r"G^*", r"2g_2\sqrt2\,\eta(i)^2", "exact", "g", "Eta's nome is exp(-2pi); eta(i) is positive real.", ["Eta-theta identity", "E1"], "2 Sqrt[2 Pi] DedekindEta[I]^2"),
    row("E7", "E-prime", "Expanded eta normalization", r"G^*", r"2\sqrt{2\pi}\,\eta(i)^2", "restatement", "g", "Expand g2 in E6.", ["E6"], "2 Sqrt[2 Pi] DedekindEta[I]^2"),
    row("F1", "F", "BCC Watson value", r"G^*", r"\sqrt{2g_2^2 W_{\rm BCC}}", "exact", "g", "This is the body-centred cubic integral, with a product of three cosines.", ["Watson BCC evaluation"], "Sqrt[2 Pi wBCC]"),
    row("F2", "F", "Expanded Watson normalization", r"G^*", r"\sqrt{2\pi W_{\rm BCC}}", "restatement", "g", "Expand g2 in F1.", ["F1"], "Sqrt[2 Pi wBCC]"),
    row("F3", "F", "Squared Watson value", r"(G^*)^2", r"2g_2^2W_{\rm BCC}", "restatement", "g2", "Square F1; the target is (G*)².", ["F1"], "2 Pi wBCC"),
    row("F4", "F", "The BCC Brillouin-zone integral", r"(G^*)^2", r"\frac1{4\pi^2}\iiint_{[-\pi,\pi]^3}\frac{du\,dv\,dw}{1-\cos u\cos v\cos w}", "exact", "g2", "An improper but integrable three-dimensional BCC integral. It is not the FCC denominator 3-cos(u)cos(v)-cos(v)cos(w)-cos(w)cos(u).", ["F3", "BCC integral definition"], "2 Pi wBCC"),
    row("G1", "G", "The specified CM curve", r"G^*", r"\frac{8L(E,1)}{g_2}", "exact", "g", "E is y²=x³−x of conductor 32. Its L-value is varpi/4; this is a statement about this curve, not all CM twists.", ["Modular form eta(4tau)^2 eta(8tau)^2", "Exact special L-value"], "8 lCurve/Sqrt[Pi]"),
    row("G2", "G", "Expanded L-value normalization", r"G^*", r"\frac{8L(E,1)}{\sqrt\pi}", "restatement", "g", "The L-value and differential conventions are fixed in G1.", ["G1"], "8 lCurve/Sqrt[Pi]"),
    row("G3", "G", "Equivalent L-value coefficient", r"G^*", r"\frac8{\sqrt\pi}\,L(E,1)", "restatement", "g", "This duplicates the correct G2 normalization; the old coefficient 4sqrt(2/pi) was too small by sqrt(2).", ["G1"], "8 lCurve/Sqrt[Pi]"),
    row("G4", "G", "A real primitive period", r"G^*", r"\frac{2\Omega_+}{g_2}", "exact", "g", "Omega+ is the primitive positive real period of dx/(2y), equal to varpi; the total real period is twice as large.", ["Real period integral", "B1"], "2 omegaPlus/Sqrt[Pi]"),
    row("H1", "H", "Gauss hypergeometric series", r"G^*", r"\sqrt{2\pi}\,{}_2F_1(1/2,1/2;1;1/2)", "exact", "g", "K(k)=(pi/2) 2F1(1/2,1/2;1;k²).", ["Hypergeometric elliptic integral", "C1"], "Sqrt[2 Pi] Hypergeometric2F1[1/2,1/2,1,1/2]"),
    row("H2", "H", "Sums of two squares", r"G^*", r"g_2\sqrt2\sum_{n=0}^{\infty}r_2(n)q^n", "exact", "g", "r2(n) counts ordered signed integer pairs with a²+b²=n; r2(0)=1.", ["E3", "Gaussian lattice convolution"], "Sqrt[2 Pi] Sum[SquaresR[2,n] Exp[-Pi n],{n,0,Infinity}]"),
    row("H3", "H", "A Dirichlet-character Lambert series", r"G^*", r"g_2\sqrt2\left(1+4\sum_{\substack{d\ge1\\d\ {\rm odd}}}\frac{\chi_{-4}(d)q^d}{1-q^d}\right)", "exact", "g", "Jacobi's r2(n)=4 sum_{d|n}chi_-4(d) gives the Lambert expansion; chi=+1 at 1 mod4 and -1 at 3 mod4.", ["H2", "Jacobi two-square theorem"], "Sqrt[2 Pi] (1+4 Sum[(-1)^j Exp[-Pi (2 j+1)]/(1-Exp[-Pi (2 j+1)]),{j,0,Infinity}])"),
    row("H4", "H", "Jacobi's product", r"G^*", r"g_2\sqrt2\prod_{n=1}^{\infty}(1-q^{2n})^2(1+q^{2n-1})^4", "exact", "g", "Square the Jacobi product for theta3.", ["E1", "Jacobi triple product"], "Sqrt[2 Pi] Product[(1-Exp[-2 Pi n])^2 (1+Exp[-Pi (2 n-1)])^4,{n,1,Infinity}]"),
    row("H5", "H", "The quartic exponential integral", r"G^*", r"\frac{16}{\sqrt2\,g_2^2}\left(\int_0^\infty e^{-t^4}\,dt\right)^2", "exact", "g", "The substitution u=t^4 gives Gamma(1/4)/4.", ["A8", "Gamma integral substitution"], "16/(Sqrt[2] Pi) Integrate[Exp[-t^4],{t,0,Infinity}]^2"),
    row("I1", "I", "Solving for pi", r"\pi", r"\frac{4\varpi^2}{(G^*)^2}", "restatement", "pi", "An algebraic rearrangement of B2, not an independent construction of pi.", ["B2"], "4 varpi^2/gStar^2"),
    row("I2", "I", "Solving for varpi", r"\varpi", r"\frac{G^*g_2}{2}", "restatement", "varpi", "Rearrange the bridge formula.", ["B1"], "gStar Sqrt[Pi]/2"),
    row("I3", "I", "An implicit restatement", r"G^*", r"\frac{4\varpi^2}{G^*g_2^2}", "restatement", "g", "G* occurs on both sides; this provides no independent algorithm.", ["I1"], "4 varpi^2/(gStar Pi)"),
    row("I4", "I", "A cubic restatement", r"(G^*)^3", r"\frac{16\varpi^4}{G^*g_2^4}", "restatement", "g3", "The target is (G*)³; this follows by squaring I1 and rearranging.", ["I1", "I3"], "16 varpi^4/(gStar Pi^2)"),
    row("J1", "J", "The modular-form viewpoint", r"G^*", r"\sqrt{2\pi}\,\frac{\theta_3(q)^4}{\theta_3(q)^2}=\sqrt{2\pi}\,\theta_3(q)^2", "restatement", "g", "F(tau)=theta3(exp(pi i tau))² has weight one with its theta multiplier on the appropriate subgroup. G*=sqrt(2pi)F(i); G* is a scalar value, while S(tau)=-1/tau is an operator.", ["E2", "Theta transformation law"], "Sqrt[2 Pi] EllipticTheta[3,0,Exp[-Pi]]^4/EllipticTheta[3,0,Exp[-Pi]]^2"),
]


def render_atlas() -> str:
    lines = ["# The 52-entry atlas of G*", "", "A companion to *Fifty-Two Faces of the Lemniscatic Bridge Constant*.", "",
             "These are 52 catalogue entries across twelve families. Some give other targets (pi, varpi, a power or a reciprocal), three are Wallis limits, one is an AGM-product limit, and E5 is an approximation. Algebraic restatements are labelled explicitly. Numerical checks provide reproducible consistency evidence, not proofs of the analytic theorems.", "", NOTATION.strip(), ""]
    for family, name in FAMILIES.items():
        lines += [f"## {name}", ""]
        for item in CATALOGUE:
            if item["family"] != family:
                continue
            relation = r"\approx" if item["kind"] == "approximation" else "="
            lines += [f"### {item['id']} · {item['label']}", "", f"**{item['kind'].capitalize()}**", "",
                      f"\\[ {item['lhs']} {relation} {item['rhs']} \\]", "", str(item["explanation"]), "",
                      "Depends on: " + "; ".join(item["dependencies"]) + ".", ""]
    lines += ["## Reproducibility", "", "Run `python verify_catalogue.py` from this directory (mpmath and sympy required). The report records all 52 targets, methods, residuals and tolerances, exact symbolic checks, finite-limit checks, and the SHA-256 digest of the reviewed paper. The script checks equation/table ID coverage; it does not parse the paper's LaTeX into mathematics.", "",
              "For D4, the Wolfram expression in the structured catalogue is an eight-step numerical evaluation of the displayed limit; its finite character is intentional. The independently checked AGM bracket controls the error.", "",
              "Sources and conventions are documented in the repaired paper and the introduction's source note.", ""]
    return "\n".join(lines)


if __name__ == "__main__":
    from pathlib import Path
    if Path(__file__).name == "gstar_identity_catalogue.py":
        output = Path(__file__).resolve().parents[2] / "dissemination/interactive/gstar-introduction"
        output.mkdir(parents=True, exist_ok=True)
    else:
        output = Path(__file__).resolve().parent
    (output / "ATLAS.md").write_text(render_atlas(), encoding="utf-8")
    print(f"Wrote ATLAS.md with {len(CATALOGUE)} entries.")
