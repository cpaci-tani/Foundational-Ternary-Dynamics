# ANALYSIS — FQCR Quotient-Uniqueness Scan (FTD-0143): Uniqueness Rejected

**Tag:** [CLOSED NEGATIVE — pre-registered execution, Outcome B]
**Date:** 2026-07-12 (execution; pre-reg locked 2026-05-06, tag `preregister-fqcr-quotient-uniqueness-v1` @ `557593e4`)
**LEDGER row:** FTD-0143
**Pre-registration:** [`../preregistrations/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`](../preregistrations/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md)
**Artifacts:** `engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan/` (meta.json, all_quadruples.csv, ranking CSVs, alpha_match_quadruples.csv, in-dir ANALYSIS.md with full lock-integrity record)
**Runner:** `tools/scan_fqcr_quotient_uniqueness.py` (SHA256 registered in `REF_PREREGISTER_MANIFEST.md`)

---

## The one-line result

The FQCR Model-IV exponent quadruple **(4, 6; 3, 2) is not privileged**: all 2401 quadruples in the locked search space {2..8}⁴ reproduce α⁻¹ at the 10⁻⁵ criterion, and by α-residual the canonical quadruple ranks **1333rd of 2401** — beaten by, among others, the identity quadruple (2,2;2,2) whose anomaly term is exactly zero. The pre-registered §3.3 uniqueness criterion fails maximally; §3.1/§3.2 pass only vacuously (a 2401-way ranking tie; no quadruple can reach any non-α target). **FTD-0143 closes [CLOSED NEGATIVE]; Model IV stays [SELECTION] with no uniqueness backing.**

## What the scan showed structurally

At the locked evaluation point t = 1, the anomaly term A_N — the *only* place the quadruple enters the readout — is numerically bounded by ~1.2×10⁻⁴ across the whole space (Q = e⁻²ᵖⁱ ≈ 1.9×10⁻³ suppresses every sum), confining the entire λ_max family to a ~2.6×10⁻⁶ relative band. The α-proximity of the FQCR readout is therefore carried **entirely by the quadruple-independent core** (the master-quadratic root under R = 1 + λ_N(4i)); the Model-IV quadruple is numerically epiphenomenal at t = 1, and at its canonical values it slightly *degrades* the match relative to A = 0. This is a descriptive fact of the locked construction, not a new claim: nothing here creates or supports any "FQCR derives α" statement — the physical readout inherits FTD-0013's [STRONGLY MOTIVATED CONJECTURE] via Model V, unchanged (pre-reg §9; Theorems 1–2 of the spine are untouched by construction).

## Criterion-by-criterion (locked §3)

| | Requirement | Result | Verdict |
|---|---|---|---|
| §3.1 | top-3 by total hits at ≥3/4 tolerances | every quadruple ties at exactly 1 hit (α⁻¹) at 1e−3/1e−4/1e−5; 1969/2401 at 1e−6 | PASS — **vacuous** (tie-inclusive reading; no tie-break was locked, disclosed) |
| §3.2 | no competitor >1 target at ≤1e−4 | 0 violators — no quadruple can reach any non-α target at all | PASS — **vacuous** |
| §3.3 | unique α⁻¹ match at ≤1e−5 | **2401/2401 match**; canonical residual 2.56e−9 (rank 1333); family best 1.95e−10, median 7.4e−10, worst 2.64e−6 | **FAIL — maximal** |

## Outcome adjudication (disclosed)

Pre-reg §4's Outcome B ("one or more criteria fail") and Outcome C ("some met, others fail") **overlap by construction** — a lock defect disclosed here rather than silently resolved. The mechanical split (2 pass / 1 fail) reads C and is recorded as such in `meta.json`. The verdict of record is **B**, on the pre-registration's own text: §3.3's rationale ("if multiple quadruples reproduce α⁻¹ at 10⁻⁵, the SMC claim is generic, not specific to (4,6;3,2)") and Outcome B's own description ("one of N near-equally-valid quadruples" — here N = 2401) are verbatim the finding, and the two passes carry no discriminating content. "Partial/inconclusive" would misdescribe a conclusively negative answer to the scan's one binary question.

## Consequences (all pre-registered)

- **Model IV** (`SPEC_FQCR.md` §3.1, §4): stays **[SELECTION]** — no uniqueness backing; the doc must continue NOT to claim (4,6;3,2) is uniquely selected, now with scan-negative provenance to prevent re-attempt.
- **Model V / FTD-0013:** unaffected — the [SMC] tag has independent provenance via the master quadratic; the scan tested Model IV's privileged-choice claim only.
- **FTD-0097 comparability:** same 20 targets, same tolerances, same hit convention — this scan extends the look-elsewhere discipline's negative track record on monomial/ansatz-level fits (over-rich there; quadruple-insensitive here).
- **Not covered** (pre-reg §9): Test 3 (t-running — the natural follow-up given that all discriminating power at t = 1 is suppressed by Q ≈ 1.9e−3; a t where Q is O(1) would discriminate, but requires its own pre-reg and an a-priori interpretation of t), Test 4 (generativity), spine theorems.

## Disclosures carried from the execution record

(i) Lock integrity: tag → `557593e4` verified; post-lock changes to the pre-reg = rename + one link fix; to the target-list tool = comments only, all TARGETS/ALPHA literals byte-identical. (ii) §3.1 had no locked tie-break; both readings reported, both contentless on this data. (iii) Pre-reg §2's prose "sin²θ_W (0.23121)" conflicts with the normative locked list value 0.22290 — descriptive error in the prose, no criterion affected. (iv) G_N\* (finite-N) vs G\* offset ≈ 2.75e−9 is common-mode, an order below the discriminating window. (v) 80-dps independent recomputation of three quadruples (canonical, identity, worst-case) agrees to all displayed digits.
