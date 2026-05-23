# Measurement — (1+i)-tower level-scan falsifier blind run v1

**Status:** [MEASURED · CONFIRMATORY BLIND, locked]
**Date:** 2026-04-29
**Pre-registration:** [`PROTOCOL_TOWER_LEVEL_FALSIFIER.md`](PROTOCOL_TOWER_LEVEL_FALSIFIER.md)
**LEDGER:** FTD-0111

---

## 0 · Hash lock

| artifact | SHA-256 |
|---|---|
| `tools/scan_tower_level.py` | `e20147e2319e3830083ba8601972d4b590fcbb40c690a1774fef853bcbab2329` |
| `docs/theory/03_derivations/PROTOCOL_TOWER_LEVEL_FALSIFIER.md` | `84a07e404d6f0d602f85b5c4f7bf4e90e72585a584a8ee1a8c7d8da2eb759b08` |

Git tag: `preregister-tower-level-scan-v1` (applied at the commit landing this measurement document together with the runner and protocol).

**Determinism note.** The runner is pure mpmath (no RNG). At `mp.dps ≥ 30`, results are reproducible bit-for-bit. The hash-lock fixes the runner state for future audits; the measurement is reproducible from the runner SHA-256 + protocol SHA-256 alone.

**Honest provenance.** The protocol was drafted on 2026-04-29 after a post-hoc exploratory scan with a 16-constant catalog (`scripts/exploration/explore_tower_level_scan.py`). The locked runner uses the *expanded* 22-constant PROTOCOL §2 catalog, adding six entries (lepton/hadron mass ratios, PMNS angles, cosmological parameters) that were *not* in the exploratory scan and therefore could not have been retro-fit. The blind execution is **confirmatory** for the catalog-overlap entries (which the exploratory scan had already inspected) and **first-look** for the six expansion entries. Per FTD-0097 fishing-discipline this distinction matters: only first-look results carry full Bayesian weight.

---

## 1 · Tower observables (LOCKED levels)

| k  | x_+(k)         | x_−(k)     | 1/y_+(k)       | 1/y_−(k)        |
|---:|---------------:|-----------:|---------------:|----------------:|
|  3 | 20.203097      | 3.466303   | 0.146447       | 0.853553        |
|  4 | 137.036171     | 3.023964   | 0.021590       | 0.978410        |
|  5 | 825.81556      | 2.969313   | 3.5827e-3      | 0.996417        |
|  6 | 4901.2499      | 2.960462   | 6.0366e-4      | 0.999396        |
|  7 | 29016.971      | 2.958977   | 1.0196e-4      | 0.999898        |
|  8 | 171718.13      | 2.958726   | 1.7230e-5      | 0.999983        |
|  9 | 1016130.9      | 2.958684   | 2.9117e-6      | 0.999997        |
| 10 | 6012816.9      | 2.958677   | 4.9206e-7      | 0.99999951      |
| 11 | 35579958       | 2.958675   | 8.3156e-8      | 0.99999992      |
| 12 | 2.1054e+8      | 2.958675   | 1.4053e-8      | 0.99999999      |
| 13 | 1.2458e+9      | 2.958675   | 2.3749e-9      | 1.00000000      |
| 14 | 7.3720e+9      | 2.958675   | 4.0134e-10     | 1.00000000      |
| 15 | 4.3623e+10     | 2.958675   | 6.7824e-11     | 1.00000000      |

`G* = 2.95867511918863889231082135773` (50-digit precision; `Γ(1/4)/Γ(3/4)`).

---

## 2 · Match scan results (LOCKED tolerance 1.0%)

13 hits at the LOCKED 1% tolerance, of which **only one is independent**:

| k  | observable | value           | target                       | rel error  | flags |
|---:|:-----------|:----------------|:-----------------------------|:-----------|:------|
| 4  | x_+        | 137.036171      | 1/alpha (CODATA 2022)        | **0.00013%** | STRONG, FW-INTEGER |
| 4  | 1/y_-      | 0.978410        | cos² θ_13 (PMNS)             | 0.04187%   | STRONG, HARMONIC-AUTO, FW-INTEGER |
| 5  | 1/y_-      | 0.996417        | m_p/m_n (PDG)                | 0.22058%   | HARMONIC-AUTO |
| 6  | 1/y_-      | 0.999396        | m_p/m_n (PDG)                | 0.07774%   | STRONG, HARMONIC-AUTO |
| 7  | 1/y_-      | 0.999898        | m_p/m_n (PDG)                | 0.12798%   | HARMONIC-AUTO, FW-INTEGER |
| 8–15 | 1/y_-    | → 1             | m_p/m_n (PDG = 0.99862)      | 0.13647–0.13819% | HARMONIC-AUTO |

**Interpretation of the HARMONIC-AUTO flags.** The harmonic invariant `1/y_+(k) + 1/y_−(k) = 1` (Theorem 1) forces `1/y_−(k) → 1` as `1/y_+(k) → 0`. Since `1/y_+(k) ~ 1/(2^k G*^{k−2})` decays geometrically, `1/y_−(k)` saturates near 1 for `k ≥ 5`. Any near-unity physical constant in the catalog will therefore produce apparent matches across most large-`k` levels — but these matches are not independent of each other, and not independent of the structural fact that `1/y_+(k) ≈ 0` at large `k`. The locked runner flags them via the `harmonic_complement` predicate.

**The only INDEPENDENT match in the entire scan is `k = 4, x_+ = 137.036, ↔ 1/α (CODATA)` at 1.3 ppm.** This is the anchor and serves as control. All other hits are derivative.

---

## 3 · Verdict (PROTOCOL §3 matrix, LOCKED)

| Outcome                                        | Verdict                          | Result |
|------------------------------------------------|----------------------------------|:------:|
| Anchor: `k = 4` `x_+` matches `1/α` to ≤ 1%    | CONTROL_PASS                     | ✅      |
| `≤ 1` independent match at `k ≠ 4`             | NULL_CONSISTENT                  | ✅      |
| `2–10` independent matches at `k ≠ 4`          | INCONCLUSIVE                     | —      |
| `≥ 15` independent matches at `k ≠ 4`          | NULL_REJECTED upward             | —      |
| Any `k ∈ {3, 7, 13}` framework-integer match at ≤ 0.1% | POSITIVE_STRUCTURAL_EVIDENCE | —      |

**Result: NULL_CONSISTENT with control passing.**

`0` independent matches at `k ≠ 4`. `0` framework-integer-level matches at strong tolerance. The framework-integer-as-tower-index hypothesis is **falsified at the locked-protocol blind level**, against the locked 22-constant catalog spanning lepton/hadron mass ratios, electroweak parameters, PMNS angles, strong/coupling parameters, and cosmological parameters.

---

## 4 · What this measurement establishes

**[MEASURED, LOCKED].** The (1+i)-tower of master quadratics, scanned blindly across `k ∈ [3, 15]` at 1% tolerance against 22 known dimensionless physics constants, produces exactly one independent match: the anchor `k = 4 x_+ ≈ 1/α` (1.3 ppm). The framework-integer-as-tower-index hypothesis (that `k ∈ {3, 7, 13} = {N_c, b_3, N_eff}` index additional physically-content-bearing levels) is falsified: zero matches at any framework-integer level beyond the verified `k = 4 = N_base`.

**Consistent with**: the structural reading (`THEOREM_HARMONIC_INVARIANT_TOWER.md` §6.6) that `k = 4` is uniquely selected as the smallest level at which the discriminant correction `A_k = 2^(k−2) G*^(k−3) − 1` contains a positive power of `G*`. Under this reading, framework integers do not index tower levels; only the structural `G*`-non-trivial criterion does, and it singles out `k = 4` alone.

**Does not establish**:
- Why `k = 4` carries physical content. The structural criterion (`G*`-non-trivial) describes; it does not derive. The empirical match `α⁻¹ ≈ x_+(4)` to 1.3 ppm remains [STRONGLY MOTIVATED CONJECTURE] (FTD-0013; the single live physics identification of the master quadratic). *(The historical "dual prediction" framing paired with FTD-0014 `x_- ↔ N_c` is **retired** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`.)*
- Uniqueness of the `(1+i)` multiplier choice. The blind scan tests one tower (with `m_k = 2^k`); rigidity against other Gaussian/rational primes (`(2+i)`, `(2+3i)`, `p = 3, 5, 7, …`) requires its own pre-registered protocol.
- Any negative result against the level-3 cyclotomic identity `1/y_∓(3) = sin²(π/8), cos²(π/8)`. That is [THEOREM] (`THEOREM_HARMONIC_INVARIANT_TOWER.md` §6.5), independent of any scan.

---

## 5 · LEDGER row

**FTD-0111 EXTENDED — blind tower-level scan v1 closes (1) framework-integer hypothesis at locked 22-constant catalog [NULL_CONSISTENT, control passes].** First confirmatory blind execution of `PROTOCOL_TOWER_LEVEL_FALSIFIER.md`. Runner SHA-256 `e20147e2…`, protocol SHA-256 `84a07e40…`, tag `preregister-tower-level-scan-v1`. Result: 0 independent matches at `k ≠ 4`; only the anchor `k = 4 x_+ ↔ 1/α` (1.3 ppm) verified. Framework-integer-as-tower-index hypothesis terminally falsified at this scan band. Output: `engine/results/tower_level_scan_2026-04-29/scan_result.json`. Surviving structural reason for `k = 4` selection (Section 6.6 of the theorem doc): first level at which `A_k` contains a positive power of `G*`. The dual physical-identification conjecture `α ↔ 1/x_+(4)`, `N_c ↔ x_-(4)` is unaffected — [STRONGLY MOTIVATED CONJECTURE], unchanged.

---

## 6 · Cross-references

- `PROTOCOL_TOWER_LEVEL_FALSIFIER.md` — locked spec (catalog, levels, tolerance, verdict matrix).
- `THEOREM_HARMONIC_INVARIANT_TOWER.md` §§6.5, 6.6, 6.7, 6.8 — structural framework.
- `tools/scan_tower_level.py` — locked deterministic runner.
- `engine/results/tower_level_scan_2026-04-29/scan_result.json` — JSON output of this run.
- `LEDGER.md` FTD-0111 — claim row.
- FTD-0097 — methodological precedent (look-elsewhere scan with hash-locked runner + git tag).

---

## 7 · Single-line summary

**Blind hash-locked execution of the (1+i)-tower level-scan falsifier (runner SHA-256 e20147e2…, protocol SHA-256 84a07e40…, tag preregister-tower-level-scan-v1) returned `NULL_CONSISTENT` with control passing: 0 independent matches at `k ≠ 4` against the locked 22-constant catalog, terminally falsifying the framework-integer-as-tower-index hypothesis at this scan band; the verified `k = 4 x_+ ↔ 1/α` anchor (1.3 ppm) is the only independent match in the entire `k ∈ [3, 15]` range, consistent with the structural reading that `k = 4` is uniquely selected as the smallest `G*`-non-trivial level.**
