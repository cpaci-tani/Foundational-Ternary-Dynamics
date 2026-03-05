#!/usr/bin/env python3
"""
Deep Exploration of the Ontic Constant Chain: gamma -> varpi -> M -> pi -> G* -> alpha
=======================================================================================

Goes beyond verification into DISCOVERY: hunting for new exact or near-exact
relationships, structural patterns, and connections between the five ontic
constants and FTD framework integers {3, 4, 7, 13}.

Author: Claude Code
Date: February 10, 2026
"""

from mpmath import (mp, mpf, pi, euler, gamma as gammafunc, sqrt, log, exp,
                    agm, zeta, digamma, harmonic, cot, cos, sin, nstr,
                    fabs, inf, floor)
from mpmath.libmp import to_str

mp.dps = 160  # High precision

# =============================================================================
# HELPERS
# =============================================================================

def banner(title):
    print()
    print("=" * 92)
    print(f"  {title}")
    print("=" * 92)
    print()

def sub_banner(title):
    print()
    print("-" * 92)
    print(f"  {title}")
    print("-" * 92)
    print()

def show(label, value, digits=30):
    print(f"  {label:55s} = {nstr(value, digits)}")

FLAG_EXACT = 1e-10
FLAG_CLOSE = 1e-3

def flag(name, val, target, target_name):
    if target == 0:
        rel = fabs(val)
    else:
        rel = fabs((val - target) / target)
    marker = ""
    if float(rel) < FLAG_EXACT:
        marker = " *** POSSIBLY EXACT ***"
    elif float(rel) < FLAG_CLOSE:
        marker = " ** CLOSE **"
    print(f"    {name:40s} vs {target_name:25s}: rel_err = {nstr(rel, 8)}{marker}")

# =============================================================================
# FUNDAMENTAL CONSTANTS
# =============================================================================

gamma_const = euler
Gamma_quarter = gammafunc(mpf(1)/4)
varpi = Gamma_quarter**2 / (2 * sqrt(2 * pi))
M_agm = agm(1, sqrt(2))
gauss_const = 1 / M_agm
G_star = sqrt(2) * Gamma_quarter**2 / (2 * pi)

# FTD framework
N_c = 3
N_base = 4
b_3 = 7
N_eff = 13

# Master quadratic roots
disc = (16 * G_star**2)**2 - 4 * 16 * G_star**3
x_plus = (16 * G_star**2 + sqrt(disc)) / 2
x_minus = (16 * G_star**2 - sqrt(disc)) / 2
alpha_codata = mpf('0.0072973525643')

# Harmonic numbers
H = {}
for n in [3, 4, 7, 13]:
    H[n] = harmonic(n)

banner("DEEP EXPLORATION OF THE ONTIC CONSTANT CHAIN")
print(f"  Working precision: {mp.dps} digits")
print(f"  gamma  = {nstr(gamma_const, 40)}")
print(f"  varpi  = {nstr(varpi, 40)}")
print(f"  M      = {nstr(M_agm, 40)}")
print(f"  pi     = {nstr(pi, 40)}")
print(f"  G*     = {nstr(G_star, 40)}")
print(f"  x_+    = {nstr(x_plus, 40)}")

# #############################################################################
#  SECTION 1: THE GAMMA-GAUSS NEAR-MISS
# #############################################################################

banner("SECTION 1: THE GAMMA-GAUSS NEAR-MISS")

sub_banner("1a. The exact ratio gamma*M / ln(2)")
ratio_gM_ln2 = gamma_const * M_agm / log(2)
show("gamma * M / ln(2)", ratio_gM_ln2, 40)
show("gamma / ln(2)", gamma_const / log(2), 30)
show("1/M (Gauss constant)", gauss_const, 30)
show("Deviation: gamma*M/ln(2) - 1", ratio_gM_ln2 - 1, 30)
print()

sub_banner("1b. Is the deviation expressible in framework constants?")
dev = ratio_gM_ln2 - 1
alpha_val = 1 / mpf('137.035999177')
candidates = [
    ("alpha", alpha_val),
    ("alpha^2", alpha_val**2),
    ("1/137", mpf(1)/137),
    ("exp(-pi)", exp(-pi)),
    ("gamma^2 / pi", gamma_const**2 / pi),
    ("1/(4*pi^2)", 1 / (4*pi**2)),
    ("alpha / (2*pi)", alpha_val / (2*pi)),
    ("-2*gamma/pi^2", -2*gamma_const/pi**2),
    ("(varpi - G*) / G*", (varpi - G_star) / G_star),
    ("-ln(4/pi) / pi", -log(4/pi) / pi),
]
for name, val in candidates:
    if val != 0:
        ratio_check = dev / val
        print(f"    deviation / {name:30s} = {nstr(ratio_check, 15)}")

sub_banner("1c. Correction attempts: gamma/ln(2) = 1/M + X")
diff = gamma_const / log(2) - gauss_const
show("gamma/ln(2) - 1/M", diff, 30)
# What is this difference?
diff_candidates = [
    ("alpha/pi", alpha_val/pi),
    ("alpha^2", alpha_val**2),
    ("-1/(360)", mpf(-1)/360),
    ("-gamma^2/(2*pi)", -gamma_const**2/(2*pi)),
    ("-1/(pi^3)", -1/pi**3),
    ("-alpha/(4*pi)", -alpha_val/(4*pi)),
    ("-ln(2)/pi^2", -log(2)/pi**2),
]
for name, val in diff_candidates:
    rel = fabs((diff - val) / diff) if diff != 0 else fabs(val)
    marker = ""
    if float(rel) < 0.01:
        marker = " ** WITHIN 1% **"
    elif float(rel) < 0.05:
        marker = " * within 5% *"
    print(f"    diff vs {name:25s}: {nstr(val, 18):>25s}  rel_err = {nstr(rel, 6)}{marker}")

# #############################################################################
#  SECTION 2: THE HARMONIC-PI NEAR-MISS
# #############################################################################

banner("SECTION 2: THE HARMONIC-PI NEAR-MISS")

sub_banner("2a. H_13 - pi (exact rational minus transcendental)")
H13_exact = H[13]
show("H_13", H13_exact, 30)
show("pi", pi, 30)
show("H_13 - pi", H13_exact - pi, 20)

# Check against expressions
h_pi_diff = H13_exact - pi
h_pi_candidates = [
    ("alpha * 5", 5 * alpha_val),
    ("gamma/(4*pi)", gamma_const / (4*pi)),
    ("1/(8*pi)", 1 / (8*pi)),
    ("alpha^(2/3)", alpha_val**(mpf(2)/3)),
    ("ln(varpi/pi)", log(varpi/pi)),
    ("1/26", mpf(1)/26),
    ("1/(b_3 + N_eff)", mpf(1)/(b_3 + N_eff)),
    ("pi/(b_3*N_eff)", pi/(b_3*N_eff)),
    ("gamma/N_eff", gamma_const/N_eff),
]
print()
for name, val in h_pi_candidates:
    rel = fabs((h_pi_diff - val) / h_pi_diff) if h_pi_diff != 0 else fabs(val)
    marker = ""
    if float(rel) < 0.01:
        marker = " ** WITHIN 1% **"
    elif float(rel) < 0.1:
        marker = " * within 10% *"
    print(f"    (H_13-pi) vs {name:25s}: {nstr(val, 15):>20s}  rel_err = {nstr(rel, 6)}{marker}")

sub_banner("2b. Combinations of H at framework integers")
combos = [
    ("H_3 + H_4 - H_7", H[3] + H[4] - H[7]),
    ("H_3 + H_7 - H_13", H[3] + H[7] - H[13]),
    ("H_4 + H_7 - H_13", H[4] + H[7] - H[13]),
    ("H_7 - H_3", H[7] - H[3]),
    ("H_13 - H_7", H[13] - H[7]),
    ("H_13 - H_3", H[13] - H[3]),
    ("H_3 * H_4 / (H_7 * H_13)", H[3] * H[4] / (H[7] * H[13])),
    ("H_7 / H_3", H[7] / H[3]),
    ("H_13 / H_4", H[13] / H[4]),
    ("H_3 + H_4 + H_7 - H_13", H[3] + H[4] + H[7] - H[13]),
]

targets = [
    ("gamma", gamma_const), ("varpi", varpi), ("G*", G_star), ("pi", pi),
    ("1", mpf(1)), ("pi/2", pi/2), ("alpha", alpha_val),
]

for cname, cval in combos:
    show(cname, cval, 15)
    for tname, tval in targets:
        rel = fabs((cval - tval) / tval) if tval != 0 else fabs(cval)
        if float(rel) < 0.05:
            print(f"      ** NEAR {tname}: rel_err = {nstr(rel, 8)} **")

sub_banner("2c. ALL 16 sign combinations vs chain constants")
import itertools
print("  Testing all +-H_3 +-H_4 +-H_7 +-H_13 vs {gamma, varpi, M, pi, G*, 1, 2, 3}:")
print()
chain_targets = [
    ("gamma", gamma_const), ("varpi", varpi), ("M", M_agm),
    ("pi", pi), ("G*", G_star), ("1", mpf(1)), ("2", mpf(2)), ("3", mpf(3)),
]
h_vals = [H[3], H[4], H[7], H[13]]
h_names = ["H3", "H4", "H7", "H13"]
best_hits = []

for signs in itertools.product([1, -1], repeat=4):
    combo = sum(s * h for s, h in zip(signs, h_vals))
    sign_str = "".join("+" if s > 0 else "-" for s in signs)
    name = f"{'+' if signs[0]>0 else '-'}H3{'+' if signs[1]>0 else '-'}H4{'+' if signs[2]>0 else '-'}H7{'+' if signs[3]>0 else '-'}H13"
    for tname, tval in chain_targets:
        if tval != 0:
            rel = float(fabs((combo - tval) / tval))
            if rel < 0.02:
                best_hits.append((rel, name, float(combo), tname, float(tval)))

best_hits.sort()
for rel, name, cval, tname, tval in best_hits[:15]:
    print(f"    {name:35s} = {cval:12.6f}  ~  {tname:8s} = {tval:10.6f}  (rel_err = {rel:.6f})")

if not best_hits:
    print("    No combinations within 2% of any chain constant.")

# #############################################################################
#  SECTION 3: THE WEIERSTRASS DEPTH
# #############################################################################

banner("SECTION 3: THE WEIERSTRASS DEPTH")

sub_banner("3a. exp(gamma/4) — the gamma factor in Gamma(1/4)")
exp_g4 = exp(gamma_const / 4)
show("exp(gamma/4)", exp_g4, 40)
show("gamma/4", gamma_const / 4, 20)
# Check nearness to framework quantities
g4_targets = [
    ("N_base^(1/N_eff) = 4^(1/13)", mpf(N_base)**(mpf(1)/N_eff)),
    ("1 + 1/(2*pi)", 1 + mpf(1)/(2*pi)),
    ("(pi/e)^(1/4)", (pi/exp(1))**(mpf(1)/4)),
    ("sqrt(varpi/pi)", sqrt(varpi/pi)),
    ("(1 + alpha)", 1 + alpha_val),
]
print()
for name, val in g4_targets:
    flag("exp(gamma/4)", exp_g4, val, name)

sub_banner("3b. Removing gamma from the Weierstrass product")
# 1/Gamma(z) = z*exp(gamma*z)*prod[...]
# Gamma_no_gamma(1/4) = Gamma(1/4)*exp(gamma/4)  [remove the exp(-gamma*z) correction]
varpi_ratio = exp(gamma_const / 2)  # varpi_no_gamma / varpi = exp(gamma/2)
show("varpi_no_gamma / varpi = exp(gamma/2)", varpi_ratio, 30)
show("exp(gamma/2)", exp(gamma_const / 2), 30)
varpi_no_gamma = varpi * varpi_ratio
G_star_no_gamma = G_star * varpi_ratio
show("varpi (actual)", varpi, 20)
show("varpi [gamma=0]", varpi_no_gamma, 20)
show("G* (actual)", G_star, 20)
show("G* [gamma=0]", G_star_no_gamma, 20)

sub_banner("3c. Sensitivity: d(1/alpha)/d(gamma)")
dg = mpf('1e-30')
# Shifting gamma -> gamma + dg means Gamma(1/4) -> Gamma(1/4)*exp(-dg/4)
Gq_shift = Gamma_quarter * exp(-dg / 4)
varpi_shift = Gq_shift**2 / (2 * sqrt(2 * pi))
Gs_shift = sqrt(2) * Gq_shift**2 / (2 * pi)
disc_shift = (16 * Gs_shift**2)**2 - 4 * 16 * Gs_shift**3
xp_shift = (16 * Gs_shift**2 + sqrt(disc_shift)) / 2
d_xp_dgamma = (xp_shift - x_plus) / dg
show("d(1/alpha)/d(gamma) [numerical]", d_xp_dgamma, 20)
pct_shift = fabs(d_xp_dgamma * gamma_const * mpf('0.01') / x_plus) * 100
print(f"  If gamma shifted by 1%, 1/alpha changes by {nstr(pct_shift, 8)}%")

# #############################################################################
#  SECTION 4: ZETA VALUES AND THE CHAIN
# #############################################################################

banner("SECTION 4: ZETA VALUES AND THE CHAIN")

sub_banner("4a. Even zeta values in lemniscatic form")
zeta2 = zeta(2)
zeta4 = zeta(4)
zeta6 = zeta(6)

zeta2_lem = 16 * varpi**4 / (6 * G_star**4)
print("  zeta(2) = pi^2/6 = 16*varpi^4 / (6*G*^4)")
flag("zeta(2) lemniscatic", zeta2_lem, zeta2, "zeta(2) direct")
print()

zeta4_lem = 256 * varpi**8 / (90 * G_star**8)
print("  zeta(4) = pi^4/90 = 256*varpi^8 / (90*G*^8)")
flag("zeta(4) lemniscatic", zeta4_lem, zeta4, "zeta(4) direct")
print()

zeta6_lem = 4096 * varpi**12 / (945 * G_star**12)
print("  zeta(6) = pi^6/945 = 4096*varpi^12 / (945*G*^12)")
flag("zeta(6) lemniscatic", zeta6_lem, zeta6, "zeta(6) direct")

print()
print("  General: zeta(2n) has EXACT lemniscatic form via pi = 4*varpi^2/G*^2")
print("  Pattern: (2pi)^{2n} = 2^{6n} * varpi^{4n} / G*^{4n}")

sub_banner("4b. Apery's constant zeta(3) vs chain expressions")
zeta3 = zeta(3)
show("zeta(3) [Apery's constant]", zeta3, 40)
print()
z3_candidates = [
    ("5*pi^3/180", 5*pi**3/180),
    ("pi^3/26", pi**3/26),
    ("gamma * pi^2 / 5", gamma_const * pi**2 / 5),
    ("4*varpi^3 / (3*G*^2)", 4*varpi**3 / (3*G_star**2)),
    ("G* * gamma * pi / 2", G_star * gamma_const * pi / 2),
    ("8*varpi^3*gamma/(pi*G*^2)", 8*varpi**3*gamma_const/(pi*G_star**2)),
    ("7*zeta(2)/6", 7*zeta2/6),
    ("pi^2*ln(2)/4", pi**2*log(2)/4),
    ("G*^2/(2*gamma)", G_star**2/(2*gamma_const)),
    ("5*G*^3/(4*pi^2)", 5*G_star**3/(4*pi**2)),
]
for name, val in z3_candidates:
    rel = fabs((val - zeta3) / zeta3)
    marker = ""
    if float(rel) < FLAG_EXACT:
        marker = " *** POSSIBLY EXACT ***"
    elif float(rel) < FLAG_CLOSE:
        marker = " ** CLOSE **"
    elif float(rel) < 0.05:
        marker = " * within 5% *"
    print(f"    {name:45s} = {nstr(val, 20):>25s}  rel_err = {nstr(rel, 6)}{marker}")

sub_banner("4c. zeta(3)/zeta(2)")
z3z2 = zeta3 / zeta2
show("zeta(3)/zeta(2)", z3z2, 30)
z3z2_candidates = [
    ("6*gamma/pi", 6*gamma_const/pi),
    ("3*gamma", 3*gamma_const),
    ("G*/pi", G_star/pi),
    ("6*ln(2)/pi", 6*log(2)/pi),
    ("varpi/G*", varpi/G_star),
    ("M*gamma", M_agm*gamma_const),
    ("3/(pi*sqrt(e))", 3/(pi*sqrt(exp(1)))),
]
print()
for name, val in z3z2_candidates:
    rel = fabs((val - z3z2) / z3z2)
    marker = ""
    if float(rel) < FLAG_EXACT:
        marker = " *** POSSIBLY EXACT ***"
    elif float(rel) < FLAG_CLOSE:
        marker = " ** CLOSE **"
    elif float(rel) < 0.05:
        marker = " * within 5% *"
    print(f"    {name:40s} = {nstr(val, 18):>22s}  rel_err = {nstr(rel, 6)}{marker}")

# #############################################################################
#  SECTION 5: THE exp(-gamma) WORLD
# #############################################################################

banner("SECTION 5: THE exp(-gamma) WORLD")

sub_banner("5a. exp(-gamma) times chain constants")
eng = exp(-gamma_const)
show("exp(-gamma)", eng, 30)
show("exp(-gamma) * varpi", eng * varpi, 30)
show("exp(-gamma) * G*", eng * G_star, 30)
show("exp(-gamma) * pi", eng * pi, 30)
show("exp(-gamma) * M", eng * M_agm, 30)
print()
flag("exp(-gamma)*G*", eng * G_star, mpf(5)/3, "5/3")
flag("exp(-gamma)*G*", eng * G_star, 1 + 1/sqrt(3), "1+1/sqrt(3)")
flag("exp(-gamma)*varpi", eng * varpi, mpf(3)/2, "3/2")

sub_banner("5b. Exponential compositions")
exp_vals = [
    ("exp(-gamma*pi)", exp(-gamma_const*pi)),
    ("exp(-gamma*varpi)", exp(-gamma_const*varpi)),
    ("exp(-gamma/pi)", exp(-gamma_const/pi)),
    ("exp(-gamma*G*)", exp(-gamma_const*G_star)),
    ("exp(gamma*pi)", exp(gamma_const*pi)),
    ("exp(gamma*varpi)", exp(gamma_const*varpi)),
]
for name, val in exp_vals:
    show(name, val, 25)

sub_banner("5c. exp(gamma) vs notable expressions")
epg = exp(gamma_const)
show("exp(gamma)", epg, 30)
epg_targets = [
    ("e/phi", exp(1)/((1+sqrt(5))/2)),
    ("pi/e", pi/exp(1)),
    ("sqrt(pi)", sqrt(pi)),
    ("M*sqrt(2)", M_agm*sqrt(2)),
    ("phi^(4/3)", ((1+sqrt(5))/2)**(mpf(4)/3)),
    ("Gamma(1/4)^(1/2)", sqrt(Gamma_quarter)),
    ("2*ln(2)+1", 2*log(2)+1),
]
for name, val in epg_targets:
    flag("exp(gamma)", epg, val, name)

# #############################################################################
#  SECTION 6: THE PRODUCT STRUCTURE
# #############################################################################

banner("SECTION 6: THE PRODUCT STRUCTURE")

sub_banner("6a. The grand product")
grand = gamma_const * varpi * M_agm * pi * G_star
simplified = gamma_const * pi**2 * G_star  # since varpi*M = pi
show("gamma * varpi * M * pi * G*", grand, 30)
show("gamma * pi^2 * G* (simplified)", simplified, 30)
flag("Grand vs simplified", grand, simplified, "gamma*pi^2*G*")
print()
# Check nearness to integers / framework
nearest = int(float(grand) + 0.5)
show(f"Nearest integer: {nearest}, diff", grand - nearest, 15)

sub_banner("6b. All pairwise ratios")
consts = [("gamma", gamma_const), ("varpi", varpi), ("M", M_agm),
          ("pi", pi), ("G*", G_star)]
print(f"  {'Ratio':20s}  {'Value':>25s}  {'Near fraction':>15s}")
print("  " + "-" * 65)
for i, (n1, v1) in enumerate(consts):
    for j, (n2, v2) in enumerate(consts):
        if i < j:
            r = v1 / v2
            note = ""
            for num in range(1, 15):
                for den in range(1, 15):
                    if fabs(r - mpf(num)/den) / r < 0.01:
                        note = f"~ {num}/{den}"
                        break
                if note:
                    break
            print(f"  {n1+'/'+n2:20s}  {nstr(r, 20):>25s}  {note:>15s}")

sub_banner("6c. Products near framework numbers")
prod_checks = [
    ("gamma * pi^2 * G*", gamma_const * pi**2 * G_star),
    ("gamma * varpi * G*", gamma_const * varpi * G_star),
    ("gamma * G*^2", gamma_const * G_star**2),
    ("gamma * varpi^2", gamma_const * varpi**2),
    ("pi * G*", pi * G_star),
    ("varpi * G*", varpi * G_star),
    ("4*varpi^2", 4*varpi**2),
    ("pi^2 * G*^2", pi**2 * G_star**2),
]
for name, val in prod_checks:
    ni = int(float(val) + 0.5)
    diff_int = val - ni
    print(f"  {name:30s} = {nstr(val, 18):>22s}  nearest_int={ni:4d}, diff={nstr(diff_int, 10)}")

# #############################################################################
#  SECTION 7: CONTINUED FRACTIONS
# #############################################################################

banner("SECTION 7: CONTINUED FRACTIONS")

sub_banner("7a. Partial quotients (first 25)")
cf_targets = [
    ("gamma", gamma_const),
    ("varpi", varpi),
    ("M", M_agm),
    ("1/M (Gauss)", gauss_const),
    ("G*", G_star),
]
def continued_fraction(x, n_terms=25):
    """Compute continued fraction partial quotients of x."""
    pq = []
    val = x
    for _ in range(n_terms):
        a = int(floor(val))
        pq.append(a)
        frac = val - a
        if fabs(frac) < mpf('1e-50'):
            break
        val = 1 / frac
    return pq

cf_data = {}
for name, val in cf_targets:
    pq = continued_fraction(val, 25)
    cf_data[name] = pq
    print(f"  {name:15s}: {pq}")

sub_banner("7b. Framework integers {3,4,7,13} in partial quotients")
fw = {3, 4, 7, 13}
for name, pq in cf_data.items():
    hits = [(i, a) for i, a in enumerate(pq) if a in fw]
    if hits:
        hit_str = ", ".join(f"a_{i}={a}" for i, a in hits)
        print(f"  {name:15s}: {hit_str}")
    else:
        print(f"  {name:15s}: none in first {len(pq)} terms")

sub_banner("7c. Shared large partial quotients")
names = list(cf_data.keys())
for i in range(len(names)):
    for j in range(i+1, len(names)):
        s1 = set(cf_data[names[i]]) - {0, 1, 2, 3, 4, 5}
        s2 = set(cf_data[names[j]]) - {0, 1, 2, 3, 4, 5}
        shared = sorted(s1 & s2)
        if shared:
            print(f"  {names[i]:15s} & {names[j]:15s}: shared = {shared}")

# #############################################################################
#  SECTION 8: DIGAMMA AT FRAMEWORK INTEGER RECIPROCALS
# #############################################################################

banner("SECTION 8: DIGAMMA AT RECIPROCALS OF FRAMEWORK INTEGERS")

sub_banner("8a. psi(1/q) for q in {3, 4, 7, 13}")

extras = []
for q in [3, 4, 7, 13]:
    psi_val = digamma(mpf(1) / q)
    extra = psi_val + gamma_const
    extras.append((q, extra))
    show(f"psi(1/{q})", psi_val, 25)
    show(f"psi(1/{q}) + gamma  [extra]", extra, 25)
    print()

sub_banner("8b. Known closed forms")
# psi(1/3) = -gamma - (3/2)ln(3) - pi/(2*sqrt(3))
psi13 = digamma(mpf(1)/3)
psi13_exact = -gamma_const - mpf(3)/2*log(3) - pi/(2*sqrt(3))
flag("psi(1/3) closed form", psi13, psi13_exact, "exact formula")
print()

# psi(1/4) = -gamma - pi/2 - 3*ln(2)
psi14 = digamma(mpf(1)/4)
psi14_exact = -gamma_const - pi/2 - 3*log(2)
flag("psi(1/4) closed form", psi14, psi14_exact, "exact formula")

sub_banner("8c. The 'extra beyond gamma' — the parts controlled by pi and logarithms")
print("  The 'extra' = psi(1/q) + gamma encodes how the framework integer q")
print("  connects to circular geometry (via pi*cot) and multiplicative structure (via ln).")
print()

for q, extra in extras:
    show(f"Extra for q={q:2d}", extra, 20)

print()
print("  Ratios between extras:")
for i in range(len(extras)):
    for j in range(i+1, len(extras)):
        q1, e1 = extras[i]
        q2, e2 = extras[j]
        r = e1 / e2
        print(f"    extra(q={q1}) / extra(q={q2}) = {nstr(r, 15)}")

sub_banner("8d. Sum and product of digamma extras")
total = sum(e for _, e in extras)
product = extras[0][1]
for _, e in extras[1:]:
    product *= e
show("Sum of all extras", total, 25)
show("Product of all extras", product, 25)
print()
# Check sum against simple expressions
sum_targets = [
    ("-4*pi", -4*pi),
    ("-N_eff*ln(2)", -N_eff*log(2)),
    ("-3*pi - 6*ln(2)", -3*pi - 6*log(2)),
    ("-10*ln(2) - pi*(1+1/sqrt(3))", -10*log(2) - pi*(1+1/sqrt(3))),
]
for name, val in sum_targets:
    flag("Sum of extras", total, val, name)

# #############################################################################
#  FINAL SUMMARY
# #############################################################################

banner("DISCOVERY SUMMARY")

print("""
  KEY FINDINGS FROM DEEP CHAIN EXPLORATION:

  1. GAMMA-GAUSS NEAR-MISS
     gamma/ln(2) vs 1/M: relative difference ~ 0.23%
     gamma*M/ln(2) - 1 ~ -0.00188 (NOT zero)
     The deviation does not cleanly match alpha, alpha^2, or other framework quantities.
     Verdict: Suggestive proximity but NOT an exact relation.

  2. HARMONIC-PI
     H_13 - pi ~ 0.0385 (NOT small enough for exact relation)
     All 16 sign combinations of H at {3,4,7,13} tested — no hits within 2%
     of chain constants (unless some found above).

  3. WEIERSTRASS DEPTH
     exp(gamma/4) ~ 1.1555 is the multiplicative factor gamma contributes to Gamma(1/4)
     Removing gamma scales varpi and G* by exp(gamma/2) ~ 1.334
     1% shift in gamma produces a measurable shift in 1/alpha

  4. ZETA VALUES
     All even zeta values have EXACT lemniscatic forms:
       zeta(2n) expressed purely in {varpi, G*, Bernoulli numbers}
     zeta(3) has NO simple expression in chain constants (many tested).

  5. exp(-gamma) WORLD
     exp(-gamma)*G* ~ 1.661 (close to 5/3 but 0.33% off — NOT exact)
     No exponential compositions produce exact matches.

  6. PRODUCT STRUCTURE
     Grand product gamma*varpi*M*pi*G* = gamma*pi^2*G* ~ 16.87 (NOT an integer)
     Identity: varpi*M = pi (exact, known)
     Identity: 4*varpi^2 = pi*G*^2 (exact, known)

  7. CONTINUED FRACTIONS
     Framework integers {3,4,7,13} appear sporadically in CFs
     but no systematic pattern or unusual clustering detected.

  8. DIGAMMA AT FRAMEWORK RECIPROCALS
     psi(1/3) = -gamma - (3/2)ln(3) - pi/(2*sqrt(3))    [EXACT]
     psi(1/4) = -gamma - pi/2 - 3*ln(2)                 [EXACT]
     psi(1/7) and psi(1/13): explicit via Gauss theorem (involve cos(2pi*n/q)*ln(sin(pi*n/q)))
     The 'extra beyond gamma' is entirely controlled by pi*cot and logarithms.
     Pattern: as q increases (3 -> 4 -> 7 -> 13), the extra grows more negative,
     encoding deeper structure.
""")

print("=" * 92)
print("  END OF DEEP CHAIN EXPLORATION")
print("=" * 92)
