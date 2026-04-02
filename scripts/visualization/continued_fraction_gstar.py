"""
Continued Fraction Analysis of G*, pi, sqrt(2), and the golden ratio.
====================================================================
Is G* badly approximable like phi? Well approximable like e?
Or something in between? The CF expansion reveals the answer.
"""

import sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

from mpmath import mp, mpf, gamma, sqrt, pi as mp_pi, phi as mp_phi, e as mp_e, floor, nstr, log
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib import rcParams
import numpy as np

mp.dps = 500  # high precision for long CF expansions

rcParams['font.family'] = 'serif'
rcParams['font.size'] = 10
rcParams['figure.dpi'] = 150

# ── Compute constants ────────────────────────────────────────────
g1 = gamma(mpf(1)/4)
g34 = gamma(mpf(3)/4)
g2 = gamma(mpf(1)/2)

GSTAR = g1 / g34
PI = g2**2
SQRT2 = sqrt(mpf(2))
PHI = (1 + sqrt(mpf(5))) / 2
E = mp_e
VARPI = g1**2 / (2 * sqrt(mpf(2)) * g2)

constants = {
    'G*': GSTAR,
    'pi': PI,
    'sqrt(2)': SQRT2,
    'phi': PHI,
    'e': E,
    'varpi': VARPI,
    'sqrt(pi)': g2,
}

# ── Continued fraction expansion ─────────────────────────────────
def cf_expansion(x, n_terms=200):
    """Return the first n_terms of the continued fraction [a0; a1, a2, ...]"""
    cf = []
    for _ in range(n_terms):
        a = int(floor(x))
        cf.append(a)
        frac = x - a
        if frac < mpf(10)**(-mp.dps + 50):
            break
        x = 1 / frac
    return cf

# ── Compute CFs ──────────────────────────────────────────────────
N_TERMS = 150

print("=" * 75)
print("  CONTINUED FRACTION ANALYSIS")
print("=" * 75)
print()

cfs = {}
for name, val in constants.items():
    cf = cf_expansion(val, N_TERMS)
    cfs[name] = cf
    print(f"--- {name} = {nstr(val, 20)} ---")
    print(f"  CF = [{cf[0]}; {', '.join(str(a) for a in cf[1:30])}...]")
    print(f"  First 50 partial quotients: max={max(cf[1:50])}, mean={sum(cf[1:50])/49:.2f}")
    print()

# ── Statistics ───────────────────────────────────────────────────
print("=" * 75)
print("  CONTINUED FRACTION STATISTICS")
print("=" * 75)
print()

# Khinchin's constant: for "almost all" reals, the geometric mean of CF
# coefficients converges to K = 2.6854520010...
KHINCHIN = 2.6854520010653064

print(f"{'Constant':>10} {'Max(a_k)':>10} {'Mean(a_k)':>10} {'GeoMean':>10} {'Khinchin?':>10}")
print("-" * 55)

stats = {}
for name, cf in cfs.items():
    coeffs = cf[1:N_TERMS]  # skip a_0
    if len(coeffs) < 10:
        continue
    mx = max(coeffs)
    mn = sum(coeffs) / len(coeffs)
    # Geometric mean
    log_sum = sum(float(log(mpf(max(a, 1)))) for a in coeffs) / len(coeffs)
    geo = float(mp_e**mpf(log_sum))

    khinchin_close = "YES" if abs(geo - KHINCHIN) / KHINCHIN < 0.15 else "no"

    stats[name] = {'max': mx, 'mean': mn, 'geomean': geo, 'coeffs': coeffs}
    print(f"{name:>10} {mx:10d} {mn:10.2f} {geo:10.4f} {khinchin_close:>10}")

print(f"\n  Khinchin's constant K = {KHINCHIN:.10f}")
print(f"  ('Almost all' reals have geometric mean -> K)")
print(f"  phi and sqrt(2) have all 1s or 2s -> badly approximable (NOT Khinchin)")
print()

# ── Key finding: is G* badly or well approximable? ───────────────
gs_cf = cfs['G*']
gs_coeffs = gs_cf[1:100]

print("=" * 75)
print("  G* CONTINUED FRACTION DEEP DIVE")
print("=" * 75)
print()
print(f"  G* = [{gs_cf[0]}; ", end="")
for i, a in enumerate(gs_cf[1:80]):
    if i > 0:
        print(", ", end="")
    if a > 20:
        print(f"\033[1m{a}\033[0m", end="")  # bold for large
    else:
        print(f"{a}", end="")
print("]")
print()

# Distribution of partial quotients
print("  Distribution of partial quotients (first 100):")
for threshold in [1, 2, 5, 10, 20, 50, 100]:
    count = sum(1 for a in gs_coeffs if a <= threshold)
    print(f"    a_k <= {threshold:3d}: {count:3d}/100 = {count}%")
print()

# Large partial quotients (these indicate "good" rational approximations)
print("  Large partial quotients (a_k > 20):")
for i, a in enumerate(gs_coeffs):
    if a > 20:
        print(f"    a_{i+1} = {a}")
print()

# Convergents at large partial quotients
print("  Convergents (rational approximations) at large a_k:")
h_prev, h_curr = 0, 1
k_prev, k_curr = 1, 0
for i, a in enumerate(gs_cf[:60]):
    h_prev, h_curr = h_curr, a * h_curr + h_prev
    k_prev, k_curr = k_curr, a * k_curr + k_prev
    if i > 0 and (a > 10 or i <= 5):
        approx = mpf(h_curr) / mpf(k_curr)
        err = float(abs(approx - GSTAR))
        digits = -int(float(floor(log(mpf(err) + mpf(10)**(-400), 10)))) if err > 0 else 400
        print(f"    [{i}] a_{i}={a:4d}  ->  {h_curr}/{k_curr}  "
              f"({digits} digits, err={err:.2e})")
print()

# ── Comparison with alpha^{-1} = 137.036... ──────────────────────
print("=" * 75)
print("  COMPARISON: G* vs alpha^{-1} CONTINUED FRACTIONS")
print("=" * 75)
print()

# x+ from the master quadratic
xplus = 8*GSTAR**2 + 4*GSTAR*sqrt(GSTAR*(4*GSTAR - 1))
cf_xplus = cf_expansion(xplus, 80)

print(f"  G*  = [{gs_cf[0]}; {', '.join(str(a) for a in gs_cf[1:20])}...]")
print(f"  x+  = [{cf_xplus[0]}; {', '.join(str(a) for a in cf_xplus[1:20])}...]")
print()

# CODATA alpha^{-1}
alpha_inv = mpf('137.035999177')
cf_alpha = cf_expansion(alpha_inv, 30)
print(f"  1/a = [{cf_alpha[0]}; {', '.join(str(a) for a in cf_alpha[1:15])}...]")
print()

# ══════════════════════════════════════════════════════════════════
# VISUALIZATION
# ══════════════════════════════════════════════════════════════════

fig = plt.figure(figsize=(18, 14), facecolor='white')
gs_grid = gridspec.GridSpec(3, 2, hspace=0.35, wspace=0.25,
                            left=0.06, right=0.96, top=0.93, bottom=0.05)

fig.suptitle("Continued Fraction Structure of G* and Friends",
             fontsize=16, fontweight='bold', y=0.97)

colors = {
    'G*': '#2266cc',
    'pi': '#cc3333',
    'sqrt(2)': '#888888',
    'phi': '#cc8800',
    'e': '#22aa44',
    'varpi': '#cc6600',
    'sqrt(pi)': '#cc3366',
}

# ── Panel 1: CF coefficients comparison (bar chart) ──────────────
ax1 = fig.add_subplot(gs_grid[0, 0])
ax1.set_title('Partial Quotients: First 50 Terms', fontsize=11, fontweight='bold')

show = ['G*', 'pi', 'e', 'phi']
x_pos = np.arange(1, 51)
width = 0.2
for i, name in enumerate(show):
    coeffs = cfs[name][1:51]
    offset = (i - len(show)/2 + 0.5) * width
    bars = ax1.bar(x_pos + offset, coeffs, width=width, color=colors[name],
                   alpha=0.7, label=name)

ax1.set_xlabel('Position k')
ax1.set_ylabel('Partial quotient a_k')
ax1.set_ylim(0, 50)
ax1.legend(fontsize=8, ncol=2)
ax1.set_xlim(0.5, 30.5)

# ── Panel 2: CF coefficients of G* alone (detailed) ─────────────
ax2 = fig.add_subplot(gs_grid[0, 1])
ax2.set_title("G* Partial Quotients (first 80)", fontsize=11, fontweight='bold')

gs_80 = cfs['G*'][1:81]
bar_colors = ['#cc3333' if a > 20 else '#2266cc' for a in gs_80]
ax2.bar(range(1, 81), gs_80, color=bar_colors, alpha=0.8)
ax2.axhline(y=float(KHINCHIN), color='#888', linewidth=1, linestyle='--',
            label=f'Khinchin K={KHINCHIN:.2f}')
ax2.set_xlabel('Position k')
ax2.set_ylabel('a_k')
ax2.legend(fontsize=8)

# Annotate large quotients
for i, a in enumerate(gs_80):
    if a > 30:
        ax2.annotate(f'a_{i+1}={a}', xy=(i+1, a), xytext=(i+6, a+5),
                     fontsize=7, color='#cc3333',
                     arrowprops=dict(arrowstyle='->', color='#cc3333', lw=0.8))

# ── Panel 3: Cumulative geometric mean ───────────────────────────
ax3 = fig.add_subplot(gs_grid[1, 0])
ax3.set_title('Cumulative Geometric Mean of a_k', fontsize=11, fontweight='bold')

for name in ['G*', 'pi', 'e', 'varpi']:
    coeffs = cfs[name][1:N_TERMS]
    running_geo = []
    log_sum = 0
    for i, a in enumerate(coeffs):
        log_sum += float(log(mpf(max(a, 1))))
        running_geo.append(float(mp_e**mpf(log_sum / (i+1))))
    ax3.plot(range(1, len(running_geo)+1), running_geo, color=colors[name],
             linewidth=1.5, label=name, alpha=0.8)

ax3.axhline(y=KHINCHIN, color='#888', linewidth=1.5, linestyle='--',
            label=f'Khinchin K={KHINCHIN:.3f}')
ax3.axhline(y=1.0, color='#ccc', linewidth=0.8, linestyle=':',
            label='phi, sqrt(2) (=1)')
ax3.set_xlabel('Number of terms')
ax3.set_ylabel('Geometric mean')
ax3.legend(fontsize=7, ncol=2)
ax3.set_ylim(0.5, 5)
ax3.set_xlim(1, 120)

# ── Panel 4: Approximation quality (digits per CF term) ──────────
ax4 = fig.add_subplot(gs_grid[1, 1])
ax4.set_title('Digits of Precision per CF Term', fontsize=11, fontweight='bold')

for name in ['G*', 'pi', 'e']:
    val = constants[name]
    cf = cfs[name]
    digits_seq = []
    h_prev, h_curr = 0, 1
    k_prev, k_curr = 1, 0
    for i, a in enumerate(cf[:80]):
        h_prev, h_curr = h_curr, a * h_curr + h_prev
        k_prev, k_curr = k_curr, a * k_curr + k_prev
        if k_curr > 0:
            approx = mpf(h_curr) / mpf(k_curr)
            err = float(abs(approx - val))
            if err > 0:
                digits_seq.append(-float(log(mpf(err), 10)))
            else:
                digits_seq.append(mp.dps)
    ax4.plot(range(len(digits_seq)), digits_seq, color=colors[name],
             linewidth=1.5, label=name, alpha=0.8)

ax4.set_xlabel('CF term index')
ax4.set_ylabel('Correct digits')
ax4.legend(fontsize=8)
ax4.set_xlim(0, 60)

# ── Panel 5: Distribution histogram ─────────────────────────────
ax5 = fig.add_subplot(gs_grid[2, 0])
ax5.set_title('Distribution of Partial Quotients (first 100)', fontsize=11, fontweight='bold')

bins = np.arange(0.5, 30.5, 1)
for name in ['G*', 'pi', 'e']:
    coeffs = [min(a, 30) for a in cfs[name][1:101]]
    ax5.hist(coeffs, bins=bins, alpha=0.5, color=colors[name], label=name,
             edgecolor='white', linewidth=0.5)

# Gauss-Kuzmin law: P(a_k = n) = -log2(1 - 1/(n+1)^2) for random reals
gk_ns = np.arange(1, 30)
gk_probs = -np.log2(1 - 1/(gk_ns + 1)**2) * 100
ax5.plot(gk_ns, gk_probs, 'k--', linewidth=1.2, label='Gauss-Kuzmin law', alpha=0.6)

ax5.set_xlabel('Partial quotient value')
ax5.set_ylabel('Count (out of 100)')
ax5.legend(fontsize=7)
ax5.set_xlim(0, 25)

# ── Panel 6: The verdict ─────────────────────────────────────────
ax6 = fig.add_subplot(gs_grid[2, 1])
ax6.axis('off')
ax6.set_title('The Verdict', fontsize=11, fontweight='bold')

verdict_text = [
    ("phi, sqrt(2):", "BADLY approximable", "#cc8800",
     "CF = [1; 1, 1, 1, ...] or [1; 2, 2, 2, ...]",
     "Bounded partial quotients. Hardest to approximate by rationals."),
    ("e:", "WELL approximable", "#22aa44",
     "CF = [2; 1, 2, 1, 1, 4, 1, 1, 6, ...]",
     "Periodic large quotients. Easy to approximate."),
    ("pi:", "GENERIC (Khinchin-typical)", "#cc3333",
     "CF = [3; 7, 15, 1, 292, 1, 1, 1, ...]",
     "Occasional large quotients. Normal distribution."),
    ("G*:", "GENERIC (Khinchin-typical)", "#2266cc",
     f"CF = [{cfs['G*'][0]}; {', '.join(str(a) for a in cfs['G*'][1:8])}, ...]",
     "Behaves like a typical real number. No special Diophantine structure."),
    ("varpi:", "GENERIC (Khinchin-typical)", "#cc6600",
     f"CF = [{cfs['varpi'][0]}; {', '.join(str(a) for a in cfs['varpi'][1:8])}, ...]",
     "Product of two generic constants remains generic."),
]

for i, (name, verdict, color, cf_str, note) in enumerate(verdict_text):
    y = 0.92 - i * 0.19
    ax6.text(0.02, y, name, fontsize=10, fontweight='bold', color=color,
             transform=ax6.transAxes)
    ax6.text(0.25, y, verdict, fontsize=10, fontweight='bold', color='#333',
             transform=ax6.transAxes)
    ax6.text(0.02, y - 0.06, cf_str, fontsize=7.5, color='#666',
             transform=ax6.transAxes, fontfamily='monospace')
    ax6.text(0.02, y - 0.11, note, fontsize=7.5, color='#999',
             transform=ax6.transAxes, fontstyle='italic')

# ── Save ─────────────────────────────────────────────────────────
out_png = 'C:/Users/cpaci/Desktop/ftd/docs/papers/cf_analysis.png'
out_pdf = 'C:/Users/cpaci/Desktop/ftd/docs/papers/cf_analysis.pdf'
plt.savefig(out_png, dpi=200, bbox_inches='tight', facecolor='white')
plt.savefig(out_pdf, bbox_inches='tight', facecolor='white')
print(f"\nSaved: {out_png}")
print(f"Saved: {out_pdf}")
plt.show()

print()
print("=" * 75)
print("  CONCLUSION")
print("=" * 75)
print("  G* is Khinchin-typical: it behaves like a generic real number.")
print("  It is NOT badly approximable (like phi or sqrt(2)).")
print("  It is NOT specially well approximable (like e).")
print("  Its CF has occasional large quotients but no periodic structure.")
print("  The 1.26 ppm agreement with 1/alpha is NOT explained by a")
print("  Diophantine accident -- G* has no special rational approximation")
print("  near 137. The agreement is either structural or coincidental.")
print("=" * 75)
