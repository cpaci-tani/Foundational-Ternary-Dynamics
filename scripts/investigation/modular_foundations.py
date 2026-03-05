#!/usr/bin/env python3
"""
modular_foundations.py  --  Script 1 of 5
=========================================
Modular Forms Investigation: Bridging FTD and RFT through Q(i, sqrt(15))

Computes Hilbert class polynomials, Dedekind eta values, Jacobi theta
functions, Weber class invariants, and factorizations of 137 across
the relevant number fields.

Precision: 50 decimal digits via mpmath.
"""

import sys
import os

# -- Windows encoding fix --
if sys.platform == "win32":
    os.environ["PYTHONIOENCODING"] = "utf-8"
    sys.stdout.reconfigure(encoding="utf-8")

import mpmath
from itertools import combinations

mpmath.mp.dps = 50  # 50 decimal digits of precision

# ======================================================================
#  UTILITY FUNCTIONS
# ======================================================================

PI = mpmath.pi
I = mpmath.mpc(0, 1)


def section(title):
    w = 72
    print()
    print("=" * w)
    print("  " + title)
    print("=" * w)


def subsection(title):
    print()
    trail = "-" * max(0, 68 - len(title))
    print("--- " + title + " " + trail)


def val(label, v, digits=25):
    padded = label.ljust(40)
    print("  " + padded + " = " + mpmath.nstr(v, digits))


def flag_physics(label, v):
    targets = {
        "1/alpha (137.036)": mpmath.mpf("137.035999177"),
        "alpha (0.007297)": mpmath.mpf("0.0072973525643"),
        "1728": mpmath.mpf("1728"),
        "2*sqrt(2) (Bell)": 2 * mpmath.sqrt(2),
        "varpi (2.6221)": mpmath.mpf("2.622057554292119810"),
        "pi": mpmath.pi,
        "e": mpmath.e,
        "phi (1.618)": (1 + mpmath.sqrt(5)) / 2,
    }
    try:
        rv = mpmath.re(v)
        if rv == 0:
            return
        for name, t in targets.items():
            if t == 0:
                continue
            rel = abs((rv - t) / t)
            if rel < 0.001:
                pct = "{:.4f}".format(float(rel) * 100)
                print("  *** FLAG: " + label + " is within " + pct + "% of " + name + " ***")
    except Exception:
        pass


def compute_j_invariant(tau, terms=300):
    """Compute j(tau) using Eisenstein series q-expansion."""
    q = mpmath.exp(2 * PI * I * tau)

    def sigma_k(n, k):
        s = mpmath.mpf(0)
        for d in range(1, n + 1):
            if n % d == 0:
                s += mpmath.power(d, k)
        return s

    E4 = mpmath.mpf(1)
    E6 = mpmath.mpf(1)
    qn = mpmath.mpf(1)
    for n in range(1, terms + 1):
        qn *= q
        s3 = sigma_k(n, 3)
        s5 = sigma_k(n, 5)
        E4 += 240 * s3 * qn
        E6 -= 504 * s5 * qn

    j = 1728 * E4 ** 3 / (E4 ** 3 - E6 ** 2)
    return j, E4, E6


def compute_j_via_kleinj(tau):
    """mpmath.kleinj returns j/1728, so multiply by 1728 to get j(tau)."""
    return 1728 * mpmath.kleinj(tau)


def dedekind_eta(tau, terms=500):
    """Dedekind eta via product formula."""
    q = mpmath.exp(2 * PI * I * tau)
    prefix = mpmath.exp(PI * I * tau / 12)
    prod = mpmath.mpf(1)
    qn = mpmath.mpf(1)
    for n in range(1, terms + 1):
        qn *= q
        prod *= (1 - qn)
    return prefix * prod


def weber_f(tau, terms=500):
    prefix = mpmath.exp(-PI * I / 24)
    eta_shifted = dedekind_eta((tau + 1) / 2, terms=terms)
    eta_base = dedekind_eta(tau, terms=terms)
    return prefix * eta_shifted / eta_base


def weber_f1(tau, terms=500):
    return dedekind_eta(tau / 2, terms=terms) / dedekind_eta(tau, terms=terms)


def weber_f2(tau, terms=500):
    return mpmath.sqrt(2) * dedekind_eta(2 * tau, terms=terms) / dedekind_eta(tau, terms=terms)


# ======================================================================
#  PART 1: HILBERT CLASS POLYNOMIALS
# ======================================================================

section("PART 1: HILBERT CLASS POLYNOMIALS")

# 1a. Discriminant -4
subsection("Discriminant D = -4  (class number h = 1)")
print("  H_{-4}(x) = x - 1728")
print("  Unique reduced form: (1, 0, 1)   =>  tau = i")

tau_i = I
j_i_qexp, E4_i, E6_i = compute_j_invariant(tau_i, terms=200)
j_i_kleinj = compute_j_via_kleinj(tau_i)

val("j(i)  [q-expansion, 200 terms]", j_i_qexp)
val("j(i)  [mpmath.kleinj]", j_i_kleinj)
val("j(i) - 1728", j_i_qexp - 1728)
val("E4(i)", E4_i)
val("E6(i)", E6_i)
e6abs = "{:.2e}".format(float(abs(E6_i)))
print("  E6(i) should be 0:  |E6(i)| = " + e6abs)
flag_physics("j(i)", j_i_qexp)

# 1b. Discriminant -15
subsection("Discriminant D = -15  (class number h = 2)")
print("  Reduced forms: (1,1,4) and (2,1,2)")

sqrt15 = mpmath.sqrt(15)
tau1 = (-1 + I * sqrt15) / 2
tau2 = (-1 + I * sqrt15) / 4

val("tau1 = (-1 + i*sqrt(15))/2", tau1)
val("tau2 = (-1 + i*sqrt(15))/4", tau2)

j_tau1_qexp, E4_tau1, E6_tau1 = compute_j_invariant(tau1, terms=200)
j_tau1_kleinj = compute_j_via_kleinj(tau1)
val("j(tau1) [q-expansion]", j_tau1_qexp)
val("j(tau1) [mpmath.kleinj]", j_tau1_kleinj)
flag_physics("j(tau1)", j_tau1_qexp)

j_tau2_qexp, E4_tau2, E6_tau2 = compute_j_invariant(tau2, terms=300)
j_tau2_kleinj = compute_j_via_kleinj(tau2)
val("j(tau2) [q-expansion]", j_tau2_qexp)
val("j(tau2) [mpmath.kleinj]", j_tau2_kleinj)
flag_physics("j(tau2)", j_tau2_qexp)

j1 = j_tau1_kleinj
j2 = j_tau2_kleinj

sum_j = j1 + j2
prod_j = j1 * j2
val("j1 + j2  (should be -191025)", sum_j)
val("j1 * j2  (should be -121287375)", prod_j)
val("j1 + j2 + 191025", sum_j + 191025)
val("j1*j2 + 121287375", prod_j + 121287375)
print()
print("  H_{-15}(x) = x^2 + 191025*x - 121287375")

# 1c. Discriminant -60
subsection("Discriminant D = -60  (biquadratic connection)")

D60 = -60
print("  Finding all reduced binary quadratic forms for D = " + str(D60))
print("  Conditions: b^2 - 4ac = " + str(D60) + ", |b| <= a <= c")

forms_60 = []
for a in range(1, 20):
    for b in range(-a, a + 1):
        num = b * b + 60
        if num % (4 * a) != 0:
            continue
        c = num // (4 * a)
        if c < a:
            continue
        if abs(b) == a and b < 0:
            continue
        if a == c and b < 0:
            continue
        forms_60.append((a, b, c))

print("  Found " + str(len(forms_60)) + " reduced forms:")
for f in forms_60:
    a, b, c = f
    bstr = "{:+d}".format(b)
    print("    (" + str(a) + ", " + bstr + ", " + str(c) + ")   disc = " + str(b * b - 4 * a * c))

j_values_60 = []
print()
for a, b, c in forms_60:
    tau_cm = (-b + I * mpmath.sqrt(abs(D60))) / (2 * a)
    j_cm = compute_j_via_kleinj(tau_cm)
    j_values_60.append(j_cm)
    bstr = "{:+d}".format(b)
    lab = "j((" + str(a) + "," + bstr + "," + str(c) + "))"
    val(lab + " at tau=" + mpmath.nstr(tau_cm, 12), j_cm)
    flag_physics(lab, j_cm)

h60_class = len(j_values_60)
print("")
print("  Class number h(-60) = " + str(h60_class))

print("")
print("  Elementary symmetric polynomials of the j-values:")
for k in range(1, h60_class + 1):
    e_k = mpmath.mpf(0)
    for combo in combinations(range(h60_class), k):
        term = mpmath.mpf(1)
        for idx in combo:
            term *= j_values_60[idx]
        e_k += term
    val("e_" + str(k) + " (sum of products of " + str(k) + ")", e_k)
    nearest_int = int(mpmath.nint(mpmath.re(e_k)))
    diff = abs(mpmath.re(e_k) - nearest_int)
    if diff < 0.01:
        dstr = "{:.2e}".format(float(diff))
        print("    -> nearest integer: " + str(nearest_int) + "  (diff = " + dstr + ")")

# ======================================================================
#  PART 2: DEDEKIND ETA VALUES
# ======================================================================

section("PART 2: DEDEKIND ETA VALUES")

subsection("Dedekind eta at CM points")

eta_i = dedekind_eta(I)
eta_tau1 = dedekind_eta(tau1)
eta_tau2 = dedekind_eta(tau2)

tau_rft = (-1 + I * sqrt15) / 8
eta_tau_rft = dedekind_eta(tau_rft)

val("eta(i)", eta_i)
val("|eta(i)|", abs(eta_i))
val("eta(tau1)", eta_tau1)
val("|eta(tau1)|", abs(eta_tau1))
val("eta(tau2)", eta_tau2)
val("|eta(tau2)|", abs(eta_tau2))
val("eta(tau_rft) [tau=(-1+i*sqrt15)/8]", eta_tau_rft)
val("|eta(tau_rft)|", abs(eta_tau_rft))

subsection("Verification: eta(i) = Gamma(1/4) / (2 * pi^(3/4))")

gamma_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
eta_i_expected = gamma_quarter / (2 * PI ** (mpmath.mpf(3) / 4))
val("Gamma(1/4)", gamma_quarter)
val("Gamma(1/4) / (2*pi^(3/4))", eta_i_expected)
val("|eta(i)| computed", abs(eta_i))
val("difference", abs(abs(eta_i) - eta_i_expected))

subsection("Eta quotients and ratios")

points = {
    "i": (I, eta_i),
    "tau1": (tau1, eta_tau1),
    "tau2": (tau2, eta_tau2),
    "tau_rft": (tau_rft, eta_tau_rft),
}

labels = list(points.keys())
for i_idx in range(len(labels)):
    for j_idx in range(i_idx + 1, len(labels)):
        la = labels[i_idx]
        lb = labels[j_idx]
        eta_a = points[la][1]
        eta_b = points[lb][1]
        ratio = eta_a / eta_b
        val("eta(" + la + ") / eta(" + lb + ")", ratio)
        val("|eta(" + la + ") / eta(" + lb + ")|", abs(ratio))
        flag_physics("|eta(" + la + ")/eta(" + lb + ")|", abs(ratio))

        ar = abs(ratio)
        for k in [2, 3, 4, 6, 8, 12, 24]:
            ark = ar ** k
            nearest = int(round(float(ark)))
            if nearest > 0 and abs(float(ark) - nearest) < 0.01:
                dstr = "{:.6f}".format(abs(float(ark) - nearest))
                print("    |eta(" + la + ")/eta(" + lb + ")|^" + str(k) + " ~ " + str(nearest) + "  (diff = " + dstr + ")")

# ======================================================================
#  PART 3: JACOBI THETA FUNCTIONS
# ======================================================================

section("PART 3: JACOBI THETA FUNCTIONS")

subsection("Theta functions at CM points")
print("  Convention: mpmath.jtheta(n, z, q) with q = exp(i*pi*tau)")

cm_points = {
    "tau = i": I,
    "tau1": tau1,
    "tau2": tau2,
}

theta_results = {}

for label in cm_points:
    tau_val = cm_points[label]
    print("")
    print("  -- " + label + "  (tau = " + mpmath.nstr(tau_val, 15) + ") --")
    q_nome = mpmath.exp(I * PI * tau_val)
    val("q = exp(i*pi*tau)", q_nome)
    val("|q|", abs(q_nome))

    th2 = mpmath.jtheta(2, 0, q_nome)
    th3 = mpmath.jtheta(3, 0, q_nome)
    th4 = mpmath.jtheta(4, 0, q_nome)

    val("theta_2(0, q)", th2)
    val("theta_3(0, q)", th3)
    val("theta_4(0, q)", th4)

    lam = (th2 / th3) ** 4
    val("lambda(tau) = (theta_2/theta_3)^4", lam)
    flag_physics("lambda(" + label + ")", lam)

    theta_results[label] = {
        "q": q_nome, "th2": th2, "th3": th3, "th4": th4, "lambda": lam
    }

subsection("Verification: lambda(i) = 1/2")
lam_i = theta_results["tau = i"]["lambda"]
val("lambda(i)", lam_i)
val("lambda(i) - 1/2", lam_i - mpmath.mpf(1) / 2)

subsection("Theta identity: theta_3^4 = theta_2^4 + theta_4^4 (Jacobi)")
for label in cm_points:
    r = theta_results[label]
    lhs = r["th3"] ** 4
    rhs = r["th2"] ** 4 + r["th4"] ** 4
    val("theta_3^4 - theta_2^4 - theta_4^4 at " + label, lhs - rhs)

# ======================================================================
#  PART 4: WEBER CLASS INVARIANTS
# ======================================================================

section("PART 4: WEBER CLASS INVARIANTS")

subsection("Weber f, f1, f2 at tau = i")
wf_i = weber_f(I)
wf1_i = weber_f1(I)
wf2_i = weber_f2(I)
val("f(i)", wf_i)
val("|f(i)|", abs(wf_i))
val("f1(i)", wf1_i)
val("|f1(i)|", abs(wf1_i))
val("f2(i)", wf2_i)
val("|f2(i)|", abs(wf2_i))

val("2^(1/4)", mpmath.power(2, mpmath.mpf(1) / 4))
val("|f(i)| - 2^(1/4)", abs(abs(wf_i) - mpmath.power(2, mpmath.mpf(1) / 4)))

subsection("Weber invariants at tau1 and tau2 (disc -15)")
wf_tau1 = weber_f(tau1)
wf1_tau1 = weber_f1(tau1)
wf2_tau1 = weber_f2(tau1)
val("f(tau1)", wf_tau1)
val("|f(tau1)|", abs(wf_tau1))
val("f1(tau1)", wf1_tau1)
val("|f1(tau1)|", abs(wf1_tau1))
val("f2(tau1)", wf2_tau1)
val("|f2(tau1)|", abs(wf2_tau1))
flag_physics("|f(tau1)|", abs(wf_tau1))

wf_tau2 = weber_f(tau2)
wf1_tau2 = weber_f1(tau2)
wf2_tau2 = weber_f2(tau2)
val("f(tau2)", wf_tau2)
val("|f(tau2)|", abs(wf_tau2))
val("f1(tau2)", wf1_tau2)
val("|f1(tau2)|", abs(wf1_tau2))
val("f2(tau2)", wf2_tau2)
val("|f2(tau2)|", abs(wf2_tau2))
flag_physics("|f(tau2)|", abs(wf_tau2))

subsection("Weber f^24 values (class polynomial coefficients)")
for label_wf, wf_val in [("tau1", wf_tau1), ("tau2", wf_tau2)]:
    f24 = wf_val ** 24
    val("f(" + label_wf + ")^24", f24)
    val("|f(" + label_wf + ")^24|", abs(f24))
    rp = mpmath.re(f24)
    nr = int(round(float(rp)))
    dstr = "{:.6e}".format(abs(float(rp) - nr))
    print("    nearest integer to Re(f^24): " + str(nr) + "  (diff = " + dstr + ")")

# ======================================================================
#  PART 5: FACTORIZATIONS OF 137
# ======================================================================

section("PART 5: FACTORIZATIONS OF 137")

subsection("5a. In Z[i] (Gaussian integers)")
z1 = mpmath.mpc(4, 11)
z2 = mpmath.mpc(4, -11)
prod137 = z1 * z2
print("  137 = (4 + 11i)(4 - 11i)")
val("(4+11i)(4-11i)", prod137)
val("Norm(4+11i) = 4^2 + 11^2", mpmath.mpf(4 ** 2 + 11 ** 2))
print("  Note: 4 = N_base (from FTD quadratic coefficient)")
print("  Note: 11 is prime; 4 + 11 = 15 (the RFT discriminant!)")
print("  Note: 4^2 + 11^2 = 16 + 121 = 137")

print("")
print("  137 mod 4 = " + str(137 % 4) + "  (= 1, so 137 splits in Z[i])")

subsection("5b. Representation by forms of disc -15")
print("  Form (2,1,2): f(x,y) = 2x^2 + xy + 2y^2")
print("  Form (1,1,4): f(x,y) = x^2 + xy + 4y^2")

print("")
print("  Searching for (x,y) with 2x^2 + xy + 2y^2 = 137:")
found_212 = []
for x in range(-20, 21):
    for y in range(-20, 21):
        if 2 * x * x + x * y + 2 * y * y == 137:
            found_212.append((x, y))
            v212 = 2 * x * x + x * y + 2 * y * y
            print("    Found: (" + str(x) + ", " + str(y) + ")  =>  value = " + str(v212))

print("")
print("  Specific check (3,7): 2*9 + 21 + 2*49 = " + str(2 * 9 + 21 + 2 * 49))
print("  Decomposition: 2*3^2 + 3*7 + 2*7^2 = 18 + 21 + 98 = " + str(18 + 21 + 98))

print("")
print("  Searching for (x,y) with x^2 + xy + 4y^2 = 137:")
found_114 = []
for x in range(-20, 21):
    for y in range(-20, 21):
        if x * x + x * y + 4 * y * y == 137:
            found_114.append((x, y))
            v114 = x * x + x * y + 4 * y * y
            print("    Found: (" + str(x) + ", " + str(y) + ")  =>  value = " + str(v114))

if not found_114:
    print("    NO representations found!  137 is NOT represented by (1,1,4)")
    print("    This means 137 is in the non-principal genus of disc -15")

subsection("5c. Splitting behavior of 137 in relevant fields")
print("  In Q(i):           137 = (4+11i)(4-11i)       [splits]")
print("  In Q(sqrt(-15)):   137 mod structure:")
if found_212 and not found_114:
    print("    137 is represented by non-principal form (2,1,2)")
    print("    => 137 splits, but primes above 137 are NON-principal ideals")
    print("    => This is the key number-theoretic obstruction!")

print("")
print("  Legendre symbol (-15 | 137):")
print("    -15 mod 137 = " + str((-15) % 137))
val_euler = pow(122, 68, 137)
print("    122^68 mod 137 = " + str(val_euler) + "  (1 = QR, 136 = NQR)")
if val_euler == 1:
    print("    => -15 is a quadratic residue mod 137 => 137 splits in Q(sqrt(-15))")

print("")
print("  Legendre symbol (-60 | 137):")
print("    -60 mod 137 = " + str((-60) % 137))
val_euler60 = pow((-60) % 137, 68, 137)
print("    77^68 mod 137 = " + str(val_euler60) + "  (1 = QR, 136 = NQR)")

# ======================================================================
#  PART 6: REFERENCE TABLE
# ======================================================================

section("PART 6: COMPLETE REFERENCE TABLE")

print("")
print("  All values computed to 25+ significant digits.")
print("  Use these as inputs for Scripts 2-5.")
print("")

ref_table = [
    ("SECTION", "CM Points and j-invariants", "", ""),
    ("tau_i", "i", mpmath.nstr(I, 30), "FTD CM point"),
    ("tau1", "(-1+i*sqrt(15))/2", mpmath.nstr(tau1, 30), "disc -15, form (1,1,4)"),
    ("tau2", "(-1+i*sqrt(15))/4", mpmath.nstr(tau2, 30), "disc -15, form (2,1,2)"),
    ("tau_rft", "(-1+i*sqrt(15))/8", mpmath.nstr(tau_rft, 30), "RFT scaled"),
    ("j(i)", "1728", mpmath.nstr(j_i_kleinj, 30), ""),
    ("j(tau1)", "", mpmath.nstr(j1, 30), ""),
    ("j(tau2)", "", mpmath.nstr(j2, 30), ""),
    ("j1+j2", "-191025", mpmath.nstr(j1 + j2, 30), ""),
    ("j1*j2", "-121287375", mpmath.nstr(j1 * j2, 30), ""),
]

for idx in range(len(forms_60)):
    a, b, c = forms_60[idx]
    jv = j_values_60[idx]
    bstr = "{:+d}".format(b)
    ref_table.append((
        "j_60_" + str(idx),
        "j((" + str(a) + "," + bstr + "," + str(c) + "))",
        mpmath.nstr(jv, 30),
        "disc -60 form #" + str(idx + 1)
    ))

ref_table += [
    ("SECTION", "Dedekind eta values", "", ""),
    ("eta(i)", "", mpmath.nstr(eta_i, 30), ""),
    ("|eta(i)|", "", mpmath.nstr(abs(eta_i), 30), ""),
    ("eta(tau1)", "", mpmath.nstr(eta_tau1, 30), ""),
    ("|eta(tau1)|", "", mpmath.nstr(abs(eta_tau1), 30), ""),
    ("eta(tau2)", "", mpmath.nstr(eta_tau2, 30), ""),
    ("|eta(tau2)|", "", mpmath.nstr(abs(eta_tau2), 30), ""),
    ("eta(tau_rft)", "", mpmath.nstr(eta_tau_rft, 30), ""),
    ("|eta(tau_rft)|", "", mpmath.nstr(abs(eta_tau_rft), 30), ""),
    ("Gamma(1/4)", "", mpmath.nstr(gamma_quarter, 30), ""),
    ("Gamma(1/4)/(2*pi^(3/4))", "", mpmath.nstr(eta_i_expected, 30), "= |eta(i)|"),
]

ref_table += [
    ("SECTION", "Jacobi theta values at CM points", "", ""),
]
for label in cm_points:
    r = theta_results[label]
    ref_table.append(("theta_2(0,q) at " + label, "", mpmath.nstr(r["th2"], 30), ""))
    ref_table.append(("theta_3(0,q) at " + label, "", mpmath.nstr(r["th3"], 30), ""))
    ref_table.append(("theta_4(0,q) at " + label, "", mpmath.nstr(r["th4"], 30), ""))
    ref_table.append(("lambda(" + label + ")", "", mpmath.nstr(r["lambda"], 30), ""))

ref_table += [
    ("SECTION", "Weber class invariants", "", ""),
    ("|f(i)|", "", mpmath.nstr(abs(wf_i), 30), "should be 2^(1/4)"),
    ("|f(tau1)|", "", mpmath.nstr(abs(wf_tau1), 30), ""),
    ("|f(tau2)|", "", mpmath.nstr(abs(wf_tau2), 30), ""),
    ("f(tau1)^24", "", mpmath.nstr(wf_tau1 ** 24, 30), ""),
    ("f(tau2)^24", "", mpmath.nstr(wf_tau2 ** 24, 30), ""),
]

ref_table += [
    ("SECTION", "Key mathematical constants", "", ""),
    ("pi", "", mpmath.nstr(PI, 35), ""),
    ("sqrt(15)", "", mpmath.nstr(sqrt15, 35), ""),
    ("Gamma(1/4)", "", mpmath.nstr(gamma_quarter, 35), ""),
    ("2^(1/4)", "", mpmath.nstr(mpmath.power(2, mpmath.mpf(1) / 4), 35), ""),
    ("varpi", "",
     mpmath.nstr(mpmath.sqrt(2) * gamma_quarter ** 2 / (4 * PI), 35), "lemniscatic constant"),
    ("G* = 2*varpi", "",
     mpmath.nstr(mpmath.sqrt(2) * gamma_quarter ** 2 / (2 * PI), 35), "FTD coupling constant"),
]

header = "  " + "Symbol".ljust(40) + " " + "Exact".ljust(20) + " " + "Numerical Value".ljust(55) + " " + "Notes"
sep = "  " + "-" * 40 + " " + "-" * 20 + " " + "-" * 55 + " " + "-" * 20
print(header)
print(sep)
for row in ref_table:
    if row[0] == "SECTION":
        print("")
        print("  == " + row[1] + " ==")
        continue
    print("  " + row[0].ljust(40) + " " + row[1].ljust(20) + " " + row[2].ljust(55) + " " + row[3])

# ======================================================================
#  PART 7: CROSS-CHECKS AND PHYSICS FLAGS
# ======================================================================

section("PART 7: CROSS-CHECKS AND PHYSICS FLAGS")

subsection("7a. Varpi, G*, and the master quadratic")
# varpi = lemniscatic constant (half-period of lemniscate)
varpi = mpmath.sqrt(2) * gamma_quarter ** 2 / (4 * PI)
val("varpi (lemniscatic constant)", varpi)

# G* as defined in FTD CLAUDE.md: G* = sqrt(2)*Gamma(1/4)^2 / (2*pi) = 2*varpi
G_star_ftd = mpmath.sqrt(2) * gamma_quarter ** 2 / (2 * PI)
val("G* = 2*varpi (FTD definition)", G_star_ftd)

# FTD master quadratic uses c = varpi (the lemniscatic constant itself)
# x^2 - 16*c^2*x + 16*c^3 = 0  where c = varpi
print("")
print("  Master quadratic with c = varpi:")
a_coeff = mpmath.mpf(1)
b_coeff_v = -16 * varpi ** 2
c_coeff_v = 16 * varpi ** 3
disc_v = b_coeff_v ** 2 - 4 * a_coeff * c_coeff_v
xp_v = (-b_coeff_v + mpmath.sqrt(disc_v)) / 2
xm_v = (-b_coeff_v - mpmath.sqrt(disc_v)) / 2
val("  x+ (c=varpi)", xp_v)
val("  x- (c=varpi)", xm_v)

# FTD master quadratic uses c = G* = 2*varpi ~ 2.9587
# x^2 - 16*c^2*x + 16*c^3 = 0 where c = G*
print("")
print("  Master quadratic with c = G* (FTD CLAUDE.md):")
b_coeff = -16 * G_star_ftd ** 2
c_coeff = 16 * G_star_ftd ** 3

disc_quad = b_coeff ** 2 - 4 * a_coeff * c_coeff
x_plus = (-b_coeff + mpmath.sqrt(disc_quad)) / (2 * a_coeff)
x_minus = (-b_coeff - mpmath.sqrt(disc_quad)) / (2 * a_coeff)

val("  x+ (should be ~137.036)", x_plus)
val("  x- (should be ~3.024)", x_minus)
val("  x+ - 137.035999177", x_plus - mpmath.mpf("137.035999177"))
val("  x- - 3", x_minus - 3)

subsection("7b. Connections between j-values and 137")
val("j(tau1) / 1728", j1 / 1728)
val("j(tau2) / 1728", j2 / 1728)
val("j(tau1) - j(tau2)", j1 - j2)
val("|j(tau1) - j(tau2)|", abs(j1 - j2))
val("(j1 - j2)^2", (j1 - j2) ** 2)

disc_H15 = (j1 + j2) ** 2 - 4 * j1 * j2
val("disc(H_{-15}) = (j1+j2)^2 - 4*j1*j2", disc_H15)
val("sqrt(disc(H_{-15}))", mpmath.sqrt(disc_H15))

val("191025^2 + 4*121287375", mpmath.mpf(191025) ** 2 + 4 * mpmath.mpf(121287375))

subsection("7c. The number 15 = 3 * 5 and dimension connections")
print("  15 = 3 * 5")
print("  D = -15 corresponds to Q(sqrt(-15))")
print("  The biquadratic field Q(i, sqrt(15)) has discriminant product -4 * -15 = 60")
print("  Note: -60 = -4 * 15 = -4 * 3 * 5")
print("  FTD integers: {3, 4, 7, 13}")
print("  3 + 4 = 7;  3 * 4 + 1 = 13;  3 + 4 + 7 = 14;  3 * 5 = 15")

subsection("7d. All values near 137 or 1/137 found")
print("  Scanning all computed values for proximity to 137.036 or 0.007297...")
target_alpha_inv = mpmath.mpf("137.035999177")
all_vals = {
    "j(i)": j_i_kleinj,
    "j(tau1)": j1,
    "j(tau2)": j2,
    "|eta(i)|": abs(eta_i),
    "|eta(tau1)|": abs(eta_tau1),
    "|eta(tau2)|": abs(eta_tau2),
    "lambda(i)": theta_results["tau = i"]["lambda"],
    "lambda(tau1)": theta_results["tau1"]["lambda"],
    "lambda(tau2)": theta_results["tau2"]["lambda"],
    "|f(i)|": abs(wf_i),
    "|f(tau1)|": abs(wf_tau1),
    "|f(tau2)|": abs(wf_tau2),
    "x+": x_plus,
    "x-": x_minus,
    "varpi": varpi,
}

for name in all_vals:
    v = all_vals[name]
    try:
        rv = float(mpmath.re(v))
        if rv == 0:
            continue
        rel137 = abs((rv - float(target_alpha_inv)) / float(target_alpha_inv))
        if rel137 < 0.01:
            rvs = "{:.10f}".format(rv)
            rels = "{:.4f}".format(rel137 * 100)
            print("  ** " + name + " = " + rvs + "  (" + rels + "% from 1/alpha)")
    except Exception:
        pass

print("")
print("=" * 72)
print("  SCRIPT 1 COMPLETE")
print("  All values above serve as inputs for Scripts 2-5.")
print("=" * 72)
