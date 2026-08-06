# PRE-REGISTRATION — FQCR Quotient Uniqueness Scan v1

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-06 (committed before any (4,6;3,2) uniqueness measurement)
**Hash-lock target tag:** `preregister-fqcr-quotient-uniqueness-v1`
**LEDGER row reservation:** FTD-0143
**Supersedes:** none — first uniqueness scan of the FQCR Model IV ansatz exponent quadruple.
**Author:** FTD-EFT program, FQCR sub-arc.
**Companion docs:** [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md), [`DERIV_GSTAR_QUARTER_CONJUGACY.md`](../03_derivations/DERIV_GSTAR_QUARTER_CONJUGACY.md), [`AUDIT_LOOK_ELSEWHERE_RESULTS.md`](../07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) (FTD-0097 spine-level look-elsewhere precedent), [`tools/scan_look_elsewhere.py`](../../../tools/scan_look_elsewhere.py) (target-list source).

> **Pre-registration discipline.** Every search-space boundary, target list, tolerance, acceptance criterion, and outcome-classifier below is committed *before* any (4,6;3,2) uniqueness claim is made. After commit, this document gets the SHA256 hash recorded in `REF_PREREGISTER_MANIFEST.md` and the git tag `preregister-fqcr-quotient-uniqueness-v1` is applied. Any post-hoc edit to thresholds, parameters, or acceptance criteria invalidates the pre-registration; a v2 must be issued before further measurement.

---

## §1 — Why this pre-registration

`SPEC_FQCR.md` Model IV introduces the projection-anomaly factor

$$ \Psi_N(t) := \prod_{n=1}^{N} \frac{(1 - Q^{4n})^6}{(1 - Q^{3n})^2}, \qquad Q := e^{-2\pi t}, $$

with the exponent quadruple $(k, d; \ell, m) = (4, 6; 3, 2)$. The quadruple is **not derived from FTD axioms** — it is a [SELECTION] choice. The scan tests whether $(4, 6; 3, 2)$ is privileged among nearby alternatives.

**Why pre-register.** The 2026-04-27 FTD-0097 spine-level look-elsewhere scan demonstrated that monomial-level fits at FTD's targets are **over-rich** — 62 raw / 11 dedup hits at $\varepsilon = 10^{-4}$ vs Poisson null $\lambda = 4$, $\chi^2(df=19) = 470$ raw / $38$ dedup. The same risk applies here: a $7^4 = 2401$-quadruple search space against $\sim 20$ FTD targets at four tolerances has ~$2 \times 10^5$ candidate-target pairs. Without pre-registered acceptance criteria, *any* quadruple that produces a near-miss can be retroactively elevated to "selected" — exactly the over-claim pattern FTD's epistemic discipline exists to prevent.

The scan answers one binary question:

> **Is $(4, 6; 3, 2)$ uniquely selected by low-complexity logic and numerical behaviour, or is it one of many near-misses?**

---

## §2 — Pre-registered scan space

**Search space.** $(k, d; \ell, m) \in \{2, 3, 4, 5, 6, 7, 8\}^4 = 2401$ quadruples.

**Why this range.** Lower bound $k, d, \ell, m \ge 2$: $1$ would collapse the product $(1 - Q^n)^1$ into the Dedekind $\eta$-function which is a different structural object. Upper bound $\le 8$: doubles the canonical $(4, 6; 3, 2)$ without introducing parameter-counting issues. Larger ranges would need a separate pre-reg.

**For each quadruple $(k, d; \ell, m)$, the scan computes:**

1. The finite anomaly $\Psi_N^{(k,d;\ell,m)}(t) := \prod_{n=1}^{N} (1 - Q^{kn})^d / (1 - Q^{\ell n})^m$ at $N = 4096$ and $t = 1$.
2. The combined renormalisation factor $R_N^{(k,d;\ell,m)}(1) = 1 + \lambda_N(4i) + A_N^{(k,d;\ell,m)}(1)$ where $A_N$ uses the corresponding (k, d; ℓ, m) instead of (4, 6; 3, 2).
3. The resulting transfer-matrix dominant eigenvalue $\lambda_\text{max}(M_N(1))$ via the master-quadratic formula.
4. The match score against each of the 20 targets.

**Targets (locked).** The 20 FTD-0097 dimensionless targets, copied from `tools/scan_look_elsewhere.py:135-156`. The list includes $\alpha^{-1}$ (137.0360), $m_p/m_e$ (1836.15), $\sin^2 \theta_W$ (0.23121), $\sin^2 \theta_{13}$ (0.0224), $m_\mu/m_e$ (206.768), $m_\tau/m_e$ (3477.23), $m_\text{e in MeV}$ (0.51100), $\alpha_s$ at $M_Z$ (0.1179), and 12 others. The list is **not modified** for this scan; using the same 20 targets as FTD-0097 keeps the look-elsewhere statistics directly comparable.

**Tolerances (locked).** $\varepsilon \in \{10^{-3}, 10^{-4}, 10^{-5}, 10^{-6}\}$. Each (quadruple, target) pair gets a hit count at each tolerance.

---

## §3 — Pre-registered acceptance criteria

The privileged-choice claim for $(4, 6; 3, 2)$ requires **all three** of the following:

### §3.1 — Top-3 across multiple tolerances

$(4, 6; 3, 2)$ must score in the **top 3 quadruples** (ranked by total hits across all 20 targets) at $\ge 3$ of the 4 tolerances $\{10^{-3}, 10^{-4}, 10^{-5}, 10^{-6}\}$.

Rationale: a single-tolerance top-3 is fragile to bin choice; requiring stability across $\ge 3$ tolerances guards against the look-elsewhere artefact where a quadruple wins at one $\varepsilon$ by coincidence.

### §3.2 — No competing quadruple matches more than 1 target at $\le 10^{-4}$

For all $(k, d; \ell, m) \ne (4, 6; 3, 2)$: the count of targets matched at $\varepsilon \le 10^{-4}$ is $\le 1$.

Rationale: if some other quadruple matches 2+ targets at $10^{-4}$, then $(4, 6; 3, 2)$ is one of multiple over-rich quadruples and the privileged-choice claim is unsupportable.

### §3.3 — α-target at $\le 10^{-5}$ exclusively

$(4, 6; 3, 2)$ must be the **unique quadruple** matching the $\alpha^{-1}$ target at $\varepsilon \le 10^{-5}$.

Rationale: the FTD-0013 master-quadratic root match at 1.26 ppm ($\sim 10^{-6}$) is the load-bearing physical claim. If multiple quadruples reproduce $\alpha^{-1}$ at $10^{-5}$, the SMC claim is generic, not specific to $(4, 6; 3, 2)$.

---

## §4 — Pre-registered outcomes

### Outcome A — uniqueness confirmed

All three §3 criteria met. Then:

- Model IV upgrades from [SELECTION] → **[SELECTION with uniqueness backing]**.
- LEDGER FTD-0143 closes [PASS, uniqueness confirmed].
- `SPEC_FQCR.md` §3.1 + §4 status table updated to reflect the uniqueness backing.
- This is publishable as a structurally-positive scan.

### Outcome B — uniqueness rejected

One or more §3 criteria fail. Then:

- Model IV stays [SELECTION] — **no uniqueness backing**.
- LEDGER FTD-0143 closes **[CLOSED NEGATIVE]**.
- $(4, 6; 3, 2)$ is reported as one of $N$ near-equally-valid quadruples; the FQCR α-readout becomes a chance-level fit at the look-elsewhere-corrected level.
- `SPEC_FQCR.md` §3.1 + §4 updated to reflect the negative finding. Model V's physical [SMC] claim is **not weakened by this** — it inherits its tag from FTD-0013 which has independent provenance via the master quadratic; the scan only tests Model IV's privileged-choice claim, not the master quadratic itself.

### Outcome C — partial / inconclusive

Some §3 criteria met, others fail. Then:

- LEDGER FTD-0143 closes [PARTIAL].
- An honest narrative analysis lands in `ANALYSIS_FQCR_QUOTIENT_UNIQUENESS.md` documenting which criteria failed.
- Model IV stays [SELECTION] without uniqueness backing.

Either Outcome B or Outcome C is honest; Outcome A is the structurally-positive case.

---

## §5 — Pre-registered output artefacts

```
engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan/
├── meta.json                  — config + commit hash + git tag + sha256
├── all_quadruples.csv         — 2401 rows × (k, d, ℓ, m, total_hits at each ε, target detail)
├── ranking_eps_*.csv          — top-20 quadruples per tolerance ε ∈ {10^-3, 10^-4, 10^-5, 10^-6}
├── alpha_match_quadruples.csv — quadruples matching α^-1 target at each ε
└── ANALYSIS.md                — pass/fail per §3 criterion + Outcome A/B/C verdict
```

The scan-runner script is **not yet written** at pre-reg time; it will be authored before Test 2 launches. Anchor for the script's content hash will be added to `REF_PREREGISTER_MANIFEST.md` at runtime per FTD-0097's precedent.

Sketch: extend `tools/scan_look_elsewhere.py`'s target-list and tolerance-loop infrastructure; replace its "atom × monomial" inner loop with a "quadruple × FQCR-readout" loop. ~80 LOC of Python.

---

## §6 — Backend specification

This pre-registration is committed at HEAD (commit reservation TBD, will be filled in at git-add time). The scan does NOT require the engine to run — it's pure number-theoretic computation in Python via `mpmath`. Any commit-state from FQCR landing forward is acceptable.

---

## §7 — Risk register

| Risk | Severity | Mitigation |
|---|---|---|
| Range $\{2, ..., 8\}^4$ misses the privileged quadruple if it lies outside | Low | $(4, 6; 3, 2)$ is well within the range. If a true privileged choice is at $(k, d) > 8$, this scan would not catch it — but neither would $(4, 6; 3, 2)$, so the scan still tests the correct uniqueness claim. |
| Too many quadruples produce α-matches at $10^{-3}$ tolerance | Medium | Pre-reg requires uniqueness at $10^{-5}$, not $10^{-3}$. The look-elsewhere noise floor at $10^{-3}$ is high; the §3.3 criterion is at $10^{-5}$ specifically to filter that. |
| Numerical noise in $\Psi_N^{(k,d;\ell,m)}$ at $N = 4096$ for some quadruples | Low | mpmath at 50-digit precision; truncation error is $\ll 10^{-7}$. |
| The privileged-choice claim is dependent on $t = 1$ specifically | Acknowledged | The scan tests at $t = 1$ only; varying $t$ is a separate Test 3 concern. The pre-reg is honest about this scope. |

---

## §8 — Hash-lock

After commit:

```bash
cd /c/Users/cpaci/Desktop/ftd
git tag preregister-fqcr-quotient-uniqueness-v1 <commit-sha>
sha256sum docs/theory/10_eft_program/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md
```

The SHA256 is recorded in `REF_PREREGISTER_MANIFEST.md`. The git tag is local-only (no remote push per project policy).

When the scan launches: confirm the tag resolves to the anchor commit, and that the scan-runner's content hash is recorded against the same anchor.

---

## §9 — What this pre-reg does NOT cover

To prevent scope creep:

- **Test 3 (running behaviour)**: separate research question; a-priori interpretation of $t$ is required first.
- **Test 4 (generalisation to other coupling constants)**: separate research arc; would need its own pre-reg.
- **Master quadratic structure**: Model V's identification $\alpha^{-1} = \lambda_\text{max}(M_N(1))$ inherits FTD-0013's [SMC] tag and is **not** affected by this scan's outcome.
- **Spine theorems**: Theorem 1 ($G^*$ identity) and Theorem 2 (master quadratic) are independent of Model IV's exponent quadruple.

---

## §10 — Launch authorization

User authorized FQCR documentation integration on 2026-05-06 (plan approval). Test 2 launch is queued for a separate session — pre-reg lands the bounded-now work; the actual scan execution + analysis is a follow-up.

When launching: confirm the git tag `preregister-fqcr-quotient-uniqueness-v1` exists and points at the anchor commit. Drift between tag-time and launch-time content invalidates the pre-reg.
