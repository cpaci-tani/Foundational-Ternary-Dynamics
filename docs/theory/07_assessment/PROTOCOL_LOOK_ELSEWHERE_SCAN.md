# PROTOCOL — Pre-registered look-elsewhere scan for the FTD claim base

**Tag:** [PROTOCOL]
**Date:** 2026-04-26
**LEDGER row:** FTD-0097
**Implements:** Bayesian-chair recommendation from the 2026-04-25 roundtable; cross-validation instrument for FTD-0094 (L2 candidate identity) and any future ppm-level candidate identities.
**Status:** Specification finalized; runner script (`tools/scan_look_elsewhere.py`) and SHA256 hash to be committed before scan execution.
**Output target:** `AUDIT_LOOK_ELSEWHERE_RESULTS.md` (D7).

---

## 0 · Why this exists

The FTD project's epistemic ground rules (CLAUDE.md, Constraint 11) state
that the [CONJECTURE] tag does not close the underlying methodological
question of catalog over-richness. With ~129 [PARAMETRIC] relations already
documented and a candidate algebraic atom set including framework integers,
G* powers, α powers, and π, the prior probability of finding a 10⁻⁴-level
match somewhere by chance is non-trivial.

This protocol pre-registers a **blinded, hash-locked, deterministic scan**
over polynomial combinations of the FTD atom set against a fixed list of
well-known dimensionless physics ratios. The scan tests the null hypothesis
that hits at the 10⁻⁴ tolerance occur at the rate predicted by chance
under uniform random values.

If the null is rejected (significantly fewer hits than predicted, OR hits
cluster on FTD-privileged ratios), the scan is *circumstantial evidence*
that the FTD atom set encodes structure. If the null is not rejected, the
[CONJECTURE]-tagged identities (including L2) cannot be promoted purely on
ppm-match strength.

Constraint 11 enforced: this scan **does not close** the methodological
question on its own. Even a strong null rejection still leaves room for a
specific identity to be coincidental. But it changes the *prior*, which is
what the current epistemic state requires.

## 1 · Scan specification

### 1.1 Catalog atoms

```
INTEGERS = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16, 17, 27,
            47, 55, 59, 64, 141}
G_STAR = 2.95867511918863889    # Γ(1/4)/Γ(3/4), 30 digits
G_POWERS = {G_STAR, G_STAR**2, G_STAR**3, 1/G_STAR, 1/G_STAR**2}
ALPHA_POWERS = {alpha, alpha**2, alpha**11, alpha**20}
                # CODATA 2022 alpha = 1/137.035999084
TRANSCENDENTALS = {pi, pi**2, 1/pi, 2*pi, sqrt(2*pi), sqrt(pi), e}
LATTICE_KINEMATIC = {1/sqrt(3)}    # c_lat = 1/√3 (FTD axiom)

ATOMS = INTEGERS ∪ G_POWERS ∪ ALPHA_POWERS ∪ TRANSCENDENTALS ∪ LATTICE_KINEMATIC
# total: ~38 atoms
```

### 1.2 Polynomial combinations

```
DEGREES = {1, 2, 3, 4}
COEFFICIENT_INTEGERS = {-3, -2, -1, 1, 2, 3}      # exclude 0 to avoid trivial cancellations
```

For each degree d ∈ DEGREES, generate all monomials of the form
`c · a_1 · a_2 · ... · a_d` where each `a_i ∈ ATOMS` and `c ∈ COEFFICIENT_INTEGERS`.
Up to combinatorial cancellation, this generates ~(38^4 × 6) ≈ 1.25 × 10⁷
candidate polynomial values. After de-duplication (sorted-tuple key on
prime-factorized atom representation), the effective unique-value count is
expected ~10⁶–10⁷ within a few decades of unity.

### 1.3 Target ratios (fixed, locked in this document)

20 dimensionless physics quantities, sorted by experimental precision:

```
TARGETS = [
    ("alpha_inv",        137.035999084,    1.5e-10),    # 1/α
    ("m_e_in_MeV",       0.51099895069,    3e-10),
    ("m_p_over_m_e",     1836.15267343,    6e-11),
    ("m_n_over_m_e",     1838.68366173,    9e-10),
    ("m_mu_over_m_e",    206.7682830,      1.6e-8),
    ("m_tau_over_m_e",   3477.23,          5e-5),
    ("m_p_over_m_n",     0.99862347796,    7e-10),
    ("g_e_minus_2",      0.00231930437,    8e-13),    # (g-2)/2 electron, ~ a_e
    ("a_mu",             0.0011659184,     6e-10),    # muon anomaly
    ("alpha_s_MZ",       0.1179,           1e-3),
    ("sin2_theta_W",     0.22290,          3e-5),
    ("Vud_squared",      0.94888,          5e-5),
    ("m_W_over_m_Z",     0.88147,          2e-5),
    ("m_b_over_m_c",     4.18,             0.05),
    ("m_t_over_v_higgs", 0.991,            0.001),    # top-Higgs Yukawa
    ("Omega_b",          0.0493,           0.0006),
    ("Omega_dm",         0.265,            0.007),
    ("h_Hubble",         0.674,            0.005),
    ("Theta_13",         0.150,            0.001),    # PMNS
    ("delta_CP",         1.36,             0.17),     # PMNS, in radians
]
```

The three diagnostic targets (Bayesian-chair recommendation):
- `alpha · m_mu_over_m_e`  — composite, not in FTD's existing claim base
- `sin2_theta_W`           — already-demoted PARAMETRIC (FTD-0018)
- `m_tau_over_m_e`         — derived in FTD per LEDGER, but check independently

### 1.4 Tolerance

A "hit" at tolerance ε is any (target, polynomial) pair where
`|polynomial − target| / |target| < ε`. The scan reports hits at
ε ∈ {10⁻³, 10⁻⁴, 10⁻⁵, 10⁻⁶}, with 10⁻⁴ as the headline tolerance
(matches the order of the L2 residual at 68.77 ppm = 6.88 × 10⁻⁵).

## 2 · Pre-commit hash + runner

The runner script lives at `tools/scan_look_elsewhere.py` (to be authored
in a separate commit before the scan executes). It is required to be
deterministic — `random.seed` not used; iteration order over `ATOMS`,
`TARGETS`, `COEFFICIENT_INTEGERS` is the fixed lexicographic / declared
order in this document.

**Hash commit ritual (mandatory before scan):**

```
git add tools/scan_look_elsewhere.py
git commit -m "tooling: look-elsewhere scan runner (FTD-0097)"
sha256sum tools/scan_look_elsewhere.py    # paste output below
git tag preregister-look-elsewhere-scan-v1
```

SHA256 of approved runner: `6d9f0f5aebe924023b09003cd13448eb87fc7d036e7bac48cb8e442bb82d628f`
(committed at git rev `ebc5178`, tagged `preregister-look-elsewhere-scan-v1` 2026-04-27.)

The scan is run only AFTER the tag is in place. Any modification to the
runner after tagging requires re-tagging and re-running from scratch — no
partial / amended scans.

## 3 · Three diagnostic targets

These are pre-declared. If the scan reports a hit on any of them at
ε ≤ 10⁻⁴ with a low-degree polynomial (d ≤ 3), this is a control signal
that the catalog is over-rich enough to absorb arbitrary physics ratios.

| Target | FTD prior status | Outcome interpretation if hit at 10⁻⁴ |
|---|---|---|
| `α · m_mu/m_e` | not in FTD claim base | strongest signal of catalog over-richness |
| `sin²θ_W`     | PARAMETRIC (FTD-0018, 3.5% off as `3/13`) | a tighter fit at 10⁻⁴ would re-open the question |
| `m_tau/m_e`    | DERIVED in FTD | confirms or contradicts the existing derivation |

## 4 · Null hypothesis

Under the null (FTD atom set has no structural privilege), polynomial
values populate the search range uniformly. The expected number of hits
per target at tolerance ε, given N_polynomials values:

  E[hits per target | null] = N_polynomials × 2ε / (RANGE)

with RANGE the search range over which polynomial values are distributed
(taking values within {0.01, 100} as a reasonable order-of-magnitude
window for the targets above).

For N_polynomials ≈ 10⁷ and ε = 10⁻⁴ over RANGE = 10⁴ decades:

  E[hits per target | null] ≈ 10⁷ × 2 × 10⁻⁴ / 10⁴ = 0.2

For 20 targets: E[total hits | null] ≈ 4. Variance ~ Poisson(λ=4).

**Null rejected** at 95% confidence if the observed total hit count is
either:
- > 11 (upper-tail; *too many* matches → catalog is highly over-rich,
  evidence AGAINST FTD's structural claims), OR
- < 1 (lower-tail; *too few* matches → catalog is structurally selective,
  evidence FOR FTD), OR
- the hit distribution is *non-uniformly* clustered on FTD-privileged
  targets (m_e, α, N_c, etc.) versus diagnostic targets — a chi-squared
  test on the per-target hit count.

The third criterion is the load-bearing one. The total-count test is too
sensitive to the choice of search range; the *clustering* test is robust
to that choice.

## 5 · Output format

Three artifacts after the scan:

### 5.1 Histogram of residuals
PNG and CSV: `log10(|p − t|/|t|)` distribution across all (polynomial,
target) pairs. For the null, this should be approximately uniform on
log-scale. Deviation indicates structure.

### 5.2 Diagnostic scatter
PNG: x-axis = polynomial degree, y-axis = log10 residual. Per-target
clusters. Reveals whether low-degree polynomials hit disproportionately
on FTD-privileged targets.

### 5.3 Hit table
JSON / CSV: every (target, polynomial, residual) triple with residual ≤ 10⁻³.
Columns: target_name, polynomial_string, polynomial_value, target_value,
residual, degree, atom_count.

## 6 · Author isolation

Two paths:

(a) **Non-author run (preferred).** The runner script is hash-locked.
    A non-author with no FTD knowledge runs `python tools/scan_look_elsewhere.py`
    on a clean checkout of the pre-registration tag and submits the output.
    They do not see this protocol document until after the run.

(b) **Self-run with deterministic isolation (fallback for solo development).**
    The author runs the scan themselves, but only AFTER the SHA256 + git tag
    are pushed and the protocol document is timestamp-frozen. The
    deterministic runner produces identical output regardless of who runs it,
    so the only manipulation channel is post-hoc cherry-picking — which is
    closed by requiring D7 to enumerate ALL hits at ε ≤ 10⁻³ rather than
    selecting interesting ones.

## 7 · Verdict assembly (input to D7)

Three outcomes:

| Total hits at 10⁻⁴ | Cluster pattern | Verdict |
|---|---|---|
| 0–1 (within Poisson lower tail) | (any) | NULL REJECTED — catalog is structurally selective; strengthens [CONJECTURE]-tagged identities |
| 2–10 (within null prediction) | uniform on targets | NULL HOLDS — catalog matches chance; [CONJECTURE] tags do NOT promote on ppm strength alone |
| 2–10 | strongly clustered on FTD-privileged targets | NULL CONDITIONALLY HOLDS — privileged-target hits could be either real signal or chance with selection bias; chi-squared test required |
| > 10 (upper tail) | (any) | NULL REJECTED upward — catalog is highly over-rich; weakens ALL [CONJECTURE] identities |

The L2 identity (FTD-0094) status post-scan:
- NULL REJECTED downward → L2 has marginally stronger structural standing
- NULL HOLDS → L2 is exactly the kind of finding the catalog produces by chance; tag stays [CONJECTURE] regardless of further measurement
- NULL REJECTED upward → L2 demoted to [PARAMETRIC]; closes the chain definitively from the look-elsewhere side

## 8 · Cross-references

- `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` — the
  ~129 PARAMETRIC entries the scan is implicitly testing against.
- `docs/theory/07_assessment/LEDGER.md` — claim base.
- `docs/theory/10_eft_program/archive/closed_negative/DERIV_MECHANISM_C_GC_BCC_BRIDGE.md` (FTD-0093) — Mechanism C derivation.
- `docs/theory/10_eft_program/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md` — the
  D2 BCC measurement, whose results combine with this scan's outcome
  per the §7 verdict matrix.
- `CLAUDE.md` Constraint 11 — the methodological commitment this scan
  operationalizes.

## 9 · What this protocol does NOT do

- It does NOT scan continuously for new candidate identities. It is a
  *one-time* validation of the existing catalog's structural status.
- It does NOT close the methodological question by itself. A NULL REJECTED
  downward is *circumstantial* evidence; the conjecture still needs an
  independent structural derivation to promote past [SELECTION].
- It does NOT replace the structural / model-theoretic tests in D1, D4.
  Those are different epistemic moves (logical / structural) and stand
  independently.
- It does NOT propose additional algebraic identities to test. The atom
  set and target set are FROZEN in this document.
