# Pre-registration protocol — (1+i)-tower level-scan falsifier

**Status:** [PRE-REGISTRATION DRAFT, not yet hash-locked]
**Date drafted:** 2026-04-29 (late evening)
**Companion:** [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) §6.7
**LEDGER:** FTD-0111 (extends Theorem 8 with falsifier protocol)
**Discipline reference:** FTD-0097 look-elsewhere scan (the methodological template)

---

## 0 · Why this protocol exists

Theorem 8 (`THEOREM_HARMONIC_INVARIANT_TOWER.md`) introduces the (1+i)-tower of master quadratics `M_k(x) = x² − 2^k G*^(k−2) x + 2^k G*^(k−1)`. The level `k = 4` matches `α⁻¹` to 1.26 ppm and `N_c` to 0.80% — verified [STRONGLY MOTIVATED CONJECTURE] under FTD-0001.

A natural follow-up question is: *do other levels of the tower carry physical content?* Specifically:

- The framework integers `{N_c = 3, N_base = 4, b_3 = 7, N_eff = 13}` index four levels. `k = N_base = 4` is empirically matched. Do `k ∈ {3, 7, 13}` also carry content?
- More generally, does any level `k ≠ 4` produce a tower observable (`x_+`, `x_-`, `1/y_+`, `1/y_-`) that matches a known dimensionless physics constant?

A post-hoc exploratory scan (`scripts/exploration/explore_tower_level_scan.py`) was run on 2026-04-29 (late evening) and found no positive matches at any level `k ≠ 4` other than the `k = 4` `1/y_-` ↔ `cos²(θ_13)` complement, which is automatic from the harmonic invariant and therefore non-independent. The exploratory scan **falsified the framework-integer-as-tower-index hypothesis** at the level the scan could probe (1% tolerance, 16 candidate constants).

But: that scan was post-hoc, and per FTD-0097 / CLAUDE.md fishing-discipline, post-hoc scans cannot be cited as evidence. **A future blind run, with prior pre-registration of the catalog, level range, precision threshold, and look-elsewhere correction, is required before any positive match at `k ≠ 4` can be admitted as evidence.** This document specifies that pre-registration.

---

## 1 · Scope

The scan is over the (1+i)-tower of master quadratics defined in Theorem 8 of `SPEC_ALGEBRAIC_SPINE.md`:

$$M_k(x) := x^2 - 2^k\,G^{*\,k-2}\,x + 2^k\,G^{*\,k-1}, \qquad k \in \{3, 4, 5, \ldots, K_{\max}\}.$$

Tower observables at each level: `x_+(k)`, `x_-(k)`, `1/y_+(k) = G*/x_+(k)`, `1/y_-(k) = G*/x_-(k)`.

**Locked range**: `k ∈ {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}`. Reasoning: `k = 4` is the verified match anchor; the framework integers extend up to `N_eff = 13`; one buffer level `k = 14, 15` to absorb edge effects.

---

## 2 · Catalog of candidate physical constants (LOCKED before measurement)

The catalog is locked at **N_targets = 22 dimensionless physics constants** drawn from established sources (CODATA 2022, PDG 2024, established cosmology). Locking criterion: the constant must have an experimental determination at ≤1% precision and a name recognized by mainstream physics.

**Anchor (verified, included for control):**
1. `α⁻¹` (CODATA 2022): 137.035999177(21)

**Lepton mass ratios (PDG 2024):**
2. `m_μ/m_e`: 206.7682830
3. `m_τ/m_e`: 3477.23
4. `m_τ/m_μ`: 16.817

**Hadron mass ratios (PDG 2024):**
5. `m_p/m_e`: 1836.15267343
6. `m_n/m_e`: 1838.68366
7. `m_π/m_e`: 273.13
8. `m_K/m_e`: 974.0
9. `m_p/m_n`: 0.99862

**Electroweak (PDG 2024, on-shell scheme):**
10. `m_W/m_Z`: 1.13501
11. `m_H/m_W`: 1.553
12. `sin²θ_W`: 0.23121

**Neutrino mixing (PDG 2024 PMNS):**
13. `sin²θ_12`: 0.307
14. `sin²θ_23`: 0.546
15. `sin²θ_13`: 0.0220
16. `cos²θ_13`: 0.978

**Strong / coupling-related:**
17. `α_s(M_Z)`: 0.1180
18. `m_b/m_t`: 0.0234

**Cosmological (Planck 2018):**
19. `Ω_b h²`: 0.02237
20. `Ω_DM/Ω_b`: 5.32

**Mathematical anchor controls:**
21. `4π`: 12.566
22. `e` (Euler): 2.71828

The catalog is locked at this set of 22. Any deviation (adding, removing, or substituting constants) after this protocol's hash-lock invalidates the scan.

---

## 3 · Tolerance and look-elsewhere correction

**Tolerance threshold**: 1.0% relative error (`|q/v − 1| < 0.01`) for any (level, observable, target) triple.

**Look-elsewhere correction.** Total comparison count:
- Levels: 13 (`k = 3` through `k = 15`)
- Observables per level: 4 (`x_+`, `x_-`, `1/y_+`, `1/y_-`)
- Targets: 22

Total tests = `13 × 4 × 22 = 1144`.

**Null model**: assume each (observable, target) pair is independent, with the observable drawn uniformly from a log-distribution over the relevant numerical range (consistent with FTD-0097 methodology). Expected number of matches under the null at 1% tolerance:

`λ_null = 1144 × (2 × 0.01) / log_range = (1144 × 0.02) / log(some range)`

(Exact `λ_null` to be computed at scan time using the FTD-0097 methodology — `tools/scan_look_elsewhere.py` style with appropriate target-domain restriction. Anchored at `λ_null ≈ 5–10` matches.)

**Verdict matrix (LOCKED):**

| Outcome                                        | Verdict                          |
|------------------------------------------------|----------------------------------|
| `≤ 1` match at `k ≠ 4`                         | NULL CONSISTENT (catalog selective) |
| `2–10` matches at `k ≠ 4`                      | INCONCLUSIVE (within Poisson null) |
| `≥ 15` matches at `k ≠ 4`                      | NULL REJECTED upward (catalog over-rich) |
| Any single match at `k ∈ {3, 7, 13}` (framework-integer levels) at `≤ 0.1%` tolerance | **POSITIVE STRUCTURAL EVIDENCE** — promotes framework-integer-as-tower-index from [FALSIFIED in exploratory scan] to [STRONGLY MOTIVATED CONJECTURE] requiring confirmation |
| The `k = 4` `α⁻¹` match reproduces             | CONTROL passes                   |

**Pre-registration discipline.** Before any blind run:

1. Hash this protocol document (SHA-256) and add it to a git tag `preregister-tower-level-scan-vN`.
2. Hash-lock the runner script (`scripts/exploration/explore_tower_level_scan.py` with the catalog and verdict matrix from this document compiled in as constants).
3. Run the scan on a deterministic mpmath calculation (no RNG involved, so reproducibility is automatic).
4. Compare measured outcome against the verdict matrix.
5. Report verdict in a `MEASUREMENT_TOWER_LEVEL_SCAN.md` document linked to the tag.

The post-hoc scan run on 2026-04-29 produced 1 verified match (`k=4` `x_+` ↔ `α⁻¹`) and 1 automatic-from-harmonic match (`k=4` `1/y_-` ↔ `cos²θ_13`) and 0 matches at any `k ∈ {3, 7, 13}`. **Under this verdict matrix, that outcome would be "NULL CONSISTENT" — framework-integer-as-tower-index hypothesis structurally consistent with no signal**. But because the scan was post-hoc, its results CANNOT be reported as evidence. The blind re-run is required.

---

## 4 · What this scan can and cannot establish

**Can establish (under blind pre-registered protocol)**:
- Whether the framework-integer-as-tower-index hypothesis carries empirical content beyond the `k = 4` anchor.
- Whether any `k ∈ {3, 5, 6, 7, ..., 15}` produces a positive match at ≤0.1% precision against the locked catalog.
- Whether the (1+i)-tower's predictive content extends beyond the single level-4 instance.

**Cannot establish**:
- Why `k = 4` is selected — the structural reason (Section 6.6 of `THEOREM_HARMONIC_INVARIANT_TOWER.md`: first `G*`-non-trivial level) is independent of any scan outcome.
- The cyclotomic identity at `k = 3` — that is [THEOREM] (Section 6.5), independent of any scan.
- The harmonic invariant itself — [THEOREM] (Section 2 of the theorem doc).

**Anti-goals (locked):**
- No attempt to "tune" the catalog mid-run to find matches.
- No reformulation of the level range after observing intermediate results.
- No reporting of "near-miss" matches outside the locked tolerance.
- No extension to non-(1+i) towers (`(2+i)`, `(2+3i)`, etc.) within this protocol — that would be a separate pre-registration with its own catalog and look-elsewhere correction.

---

## 5 · Implementation status

- **Scan script**: `scripts/exploration/explore_tower_level_scan.py` (post-hoc version, exists; needs hash-lock variant for blind run).
- **Catalog**: locked above (this document, §2).
- **Verdict matrix**: locked above (§3).
- **Hash-lock**: pending — the protocol must be committed and tagged before the blind runner is invoked.
- **Runner SHA-256**: to be computed and recorded at the time of hash-lock.
- **Output target**: `engine/results/tower_level_scan_<DATE>/`.

---

## 6 · Cross-references

- `THEOREM_HARMONIC_INVARIANT_TOWER.md` §6.5–6.8 (level-3 cyclotomic, structural why-`k=4`, exploratory scan, harmonic-conjugate cover-page reformulation)
- `SPEC_ALGEBRAIC_SPINE.md` §8 (Theorem 8)
- `tools/scan_look_elsewhere.py` (FTD-0097 methodology template)
- `PROTOCOL_LOOK_ELSEWHERE_SCAN.md` (FTD-0097 protocol template)
- LEDGER FTD-0097 (look-elsewhere scan precedent)
- LEDGER FTD-0111 (this row)

---

## 7 · Single-line summary

**The (1+i)-tower level-scan falsifier locks 22 candidate dimensionless physics constants, 13 levels (`k = 3` through `k = 15`), 4 tower observables per level, 1% tolerance, and a Poisson-null verdict matrix; positive evidence for framework-integer-as-tower-index requires any `k ∈ {3, 7, 13}` match at ≤ 0.1% precision under blind hash-locked execution; null consistent if ≤ 1 match at `k ≠ 4`; null rejected upward if ≥ 15 matches; the protocol exists because the post-hoc scan run on 2026-04-29 cannot itself be cited as evidence under FTD-0097 fishing discipline, regardless of its outcome.**
