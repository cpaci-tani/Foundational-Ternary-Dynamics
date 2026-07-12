# FTD-0143 — FQCR Quotient-Uniqueness Scan: Execution Analysis

**Run date:** 2026-07-12 · **Runner:** `tools/scan_fqcr_quotient_uniqueness.py` (SHA256 `719015e2…03da`, registered in `REF_PREREGISTER_MANIFEST.md`) · **Wall time:** 0.9 s · **Config:** N = 4096, t = 1, mpmath 50 dps, per the locked protocol.

**Verdict: Outcome B — uniqueness rejected. FTD-0143 closes [CLOSED NEGATIVE].**
(The mechanical criterion-split reads "C — partial"; §5 below discloses the overlap in the pre-registration's outcome definitions and the adjudication. Model IV stays [SELECTION] with **no uniqueness backing**, exactly as pre-registered for a non-A outcome.)

---

## 1 · Lock integrity (checked before execution)

- `git rev-list -n1 preregister-fqcr-quotient-uniqueness-v1` → `557593e44ff72ac300741e0f79d8345d5908f351` — matches the manifest anchor. ✓
- Pre-reg document diff since lock: **rename** into `preregistrations/` + **one relative-link fix** (`AUDIT_LOOK_ELSEWHERE_RESULTS.md` path). No threshold, parameter, target, or criterion changed. ✓
- `tools/scan_look_elsewhere.py` (the locked TARGETS/TOLERANCES source) diff since lock: **comment-only** (an audit note about FTD-0097's own broken hash-lock, and a comment-string edit "CODATA 2022"→"CODATA 2018"); the `ALPHA` literal `137.035999084` and every TARGETS tuple are byte-identical. The FTD-0097 lock-break note does **not** affect FTD-0143's lock. ✓
- The runner imports `TARGETS`/`TOLERANCES` directly from `tools.scan_look_elsewhere` — no copied lists, no drift surface.

## 2 · Pre-registered criteria — results

| Criterion | Locked requirement | Result | Pass? |
|---|---|---|---|
| §3.1 | (4,6;3,2) in top-3 quadruples by total hits at ≥3 of 4 tolerances | Every quadruple scores exactly 1 hit (α⁻¹ only) at ε ∈ {1e−3, 1e−4, 1e−5}; 1969/2401 score 1 at 1e−6. The hit-count ranking is a **2401-way tie** — "top-3" is satisfied only in the degenerate tie-inclusive sense | **PASS (vacuous)** |
| §3.2 | No competitor matches >1 target at ε ≤ 1e−4 | No quadruple can match anything but α⁻¹ (λ_max ≈ 137 for the whole family; every other target is orders of magnitude away) — 0 violators | **PASS (vacuous)** |
| §3.3 | (4,6;3,2) the **unique** quadruple matching α⁻¹ at ε ≤ 1e−5 | **2401 of 2401 quadruples** match α⁻¹ at 1e−5. The canonical quadruple is not merely non-unique — by α-residual it ranks **1333rd of 2401** (residual 2.56e−9; family best 1.95e−10 at (3,3;3,2)-type quadruples; family median 7.4e−10; family worst 2.64e−6) | **FAIL (maximal)** |

## 3 · Why: the readout is quadruple-insensitive at t = 1

At t = 1, Q = e^(−2π) ≈ 1.87e−3, so the anomaly term A_N^(k,d;ℓ,m) = (2π/3)(d·k·T_k − m·ℓ·T_ℓ) is bounded by ~1.2e−4 over the whole space (T_2 ≈ 3.5e−6 dominates; T_j for j ≥ 3 are ≤ 6.5e−9). The eigenvalue sensitivity dλ_max/dR ≈ −3.1 then confines the **entire family** of λ_max values to a band of width ~3.6e−4 absolute (~2.6e−6 relative) around the quadruple-independent value. That band sits inside the 1e−5 acceptance window (1.37e−3 absolute) — so §3.3 could never have discriminated at t = 1. The α-proximity of the FQCR readout is carried **entirely by the quadruple-independent part** (the master-quadratic root under R = 1 + λ_N(4i)): the A = 0 row (2,2;2,2) lands at residual 7.3e−10, *better* than the canonical quadruple (2.56e−9). At the evaluation point the Model-IV quadruple is numerically epiphenomenal.

This is reported as a descriptive fact of the locked construction, **not** as a new claim: no tag moves, and in particular this does not create any "FQCR predicts α to sub-ppb" claim — the readout inherits FTD-0013's [SMC] status via Model V, unchanged (pre-reg §9).

## 4 · Cross-checks and disclosures

- **80-dps independent recomputation** (different code path, direct per-term evaluation) of (4,6;3,2), (2,2;2,2), (8,8;2,8): λ_max agrees with the 50-dps artifacts to all displayed digits (e.g. canonical 137.035999435003578235995575791). ✓
- **G_N\* sensitivity (disclosure):** the locked formula uses the finite G_N\* (≈ G\* + 2.75e−9); the induced λ_max offset (~1e−7 relative) is common to all quadruples and an order below the §3.3 window — no effect on any criterion.
- **Tie-handling (lock ambiguity):** §3.1 defines no tie-break. Tie-inclusive reading used (canonical qualifies iff its hit count ≥ 3rd-highest distinct count). Under any strict reading the 2401-way tie makes "top-3" undefined; under the tie-inclusive reading it passes vacuously. Both readings agree the criterion has no discriminating content on this data.
- **Narrative-vs-lock discrepancy (disclosure):** pre-reg §2's prose mentions "sin²θ_W (0.23121)" while the normative locked tool list carries `sin2_theta_W = 0.22290` (the M_Z-scheme CODATA value). The tool list is the lock ("copied from `tools/scan_look_elsewhere.py:135-156`… not modified"); the prose parenthetical was descriptive error. No criterion touched sin²θ_W either way.

## 5 · Outcome adjudication (B vs C overlap — disclosed)

Pre-reg §4 defines Outcome B as "one or more §3 criteria fail" and Outcome C as "some §3 criteria met, others fail" — the definitions overlap (any C-case is literally also a B-case). The mechanical split (2 pass / 1 fail) would read C; the runner's meta.json records that split. Adjudication to **B** rests on the pre-registration's own text, not on judgment about thresholds: §3.3's rationale states "if multiple quadruples reproduce α⁻¹ at 10⁻⁵, the SMC claim is generic, not specific to (4,6;3,2)" — 2401/2401 is the maximal instance of that clause — and Outcome B's own description ("(4,6;3,2) is reported as one of N near-equally-valid quadruples") is verbatim the finding, with N = 2401. The two passing criteria pass only vacuously (no quadruple can reach any non-α target; the ranking is a full-space tie), so "partial/inconclusive" would misdescribe a conclusively negative result. Per pre-reg §4: "Either Outcome B or Outcome C is honest"; B is the accurate one.

**Consequences (per pre-reg §4-B):** Model IV stays [SELECTION] — no uniqueness backing; LEDGER FTD-0143 closes [CLOSED NEGATIVE]; the FQCR α-readout's quadruple choice is a chance-level fit at the look-elsewhere-corrected level; Model V's physical [SMC] claim is **not** weakened (it inherits FTD-0013's tag with independent provenance via the master quadratic — the scan tested Model IV's privileged-choice claim only).

## 6 · Artifacts

`meta.json` (config + lock record + criterion data + mechanical verdict), `all_quadruples.csv` (2401 rows), `ranking_eps_{1e-03,1e-04,1e-05,1e-06}.csv`, `alpha_match_quadruples.csv`, this file. Narrative doc: `docs/theory/10_eft_program/reports_and_audits/ANALYSIS_FQCR_QUOTIENT_UNIQUENESS.md`.
