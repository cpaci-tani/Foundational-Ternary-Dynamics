# AUDIT — Rigidity of the load-bearing rational identifications (FTD-0310)

**Tag:** `[MEASURED — rigidity audit]`
**Runner (SHA256-locked):** `scripts/exploration/rigidity_audit_loadbearing.py` — `3c8c42f43983b7daa61bceef3696ecd7aaf2eaac7bdcf6f2dc35967f84acd23f`
**Lock honesty:** this is a **same-session frozen-logic lock, not a blind multi-session pre-registration.** The integrity guarantees are: (a) the verdict LOGIC + thresholds (`Q_MAX=120, W=0.30, K=200000, P_THRESH=0.05, seed=20260622`) are principled-standard and were frozen in the runner **before execution** — none is tuned to any per-claim result; (b) the runner is SHA-locked + deterministic (reproducible); (c) the result runs **deflationary** (it demotes the framework's own claims), the opposite of motivated reasoning. This is the FTD-0097 / FTD-0189 look-elsewhere discipline, aimed at **debunking**, not fishing.

---

## 0 · Why (the F10 gap)

A LEDGER tag (`[PARAMETRIC]` / `[STRUCTURALLY MOTIVATED PARAMETRIC]`) **labels** a claim's status; it does **not** answer the underlying methodological question: *is the match to experiment statistically surprising, or is the space of simple rationals dense enough that some low-complexity fraction would hit the target this well by chance?* Only **x₊=1/α** has been put through such a test (FTD-0189: 0 non-G\* dual-matchers / 2.65M → rigid). This audit applies the analogous test to the three **load-bearing physics identifications** that were promoted-then-retracted in the overclaim wave and currently sit at `[STRUCTURALLY MOTIVATED PARAMETRIC]` / `[SELECTION]`.

## 1 · Method (frozen)

For each identification `(T, p₀/q₀)` with relative error `e₀ = |p₀/q₀ − T|/T`:
1. **MDL / Pareto dominance** — enumerate reduced rationals `p/q` (`q < q₀`) in `[T(1−W), T(1+W)]`; a **dominator** is any with `relerr < e₀` (strictly *simpler* AND strictly *more accurate*). A dominated claim means the framework chose a more-complex fraction to obtain its preferred integers while a simpler rational fits better — the integer story is not what does the work.
2. **Null-calibrated p-value** — draw `K` random targets `T' ~ U[T(1−W), T(1+W)]`; `p = ` fraction for which the best rational with `q ≤ q₀` achieves `relerr ≤ e₀`. This measures how routine it is to hit a random nearby target this well **at the complexity the framework spent**.
3. **Verdict (frozen):** `MDL_DOMINATED` if a strictly-simpler rational fits strictly better; else `CHANCE_LEVEL` if `p ≥ 0.05`; else `RIGID`.

Targets are the canonical `scripts/constants.py` values (computed at 40-digit precision, GTCA-F6): sin²θ_W vs PDG-effective **0.23122**; α_s vs **0.1179**; the m_e prefactor vs `k* = m_e/(m_P·√(2π)·α¹¹) = 5.343612`.

## 2 · Result (run of record)

| Identification | story | claim | rel-err | MDL dominator | null p | **verdict** |
|---|---|---|---:|---|---:|---|
| **sin²θ_W** | N_c/N_eff | 3/13 = 0.230769 | 0.195% | none (rank-3 of all q≤120; best simple rational) | **0.0594** | **CHANCE_LEVEL (borderline)** |
| **α_s** | b₃/(b₃+4·N_eff) | 7/59 = 0.118644 | 0.631% | **2/17** (simpler & better, 0.215%) | 0.903 | **MDL_DOMINATED** |
| **m_e prefactor** | N_base²/N_c | 16/3 = 5.333333 | 0.192% | none | 0.0783 | **CHANCE_LEVEL** |

**Disclosed robustness** (W ∈ {0.2, 0.3, 0.4, 0.5}; the frozen verdict uses W=0.30): sin²θ_W p = {0.049, 0.059, 0.057, 0.054} — **right on the 5% boundary** (it would read RIGID at W=0.2); α_s p = {0.92, 0.90, 0.90, 0.89} — robustly routine; m_e prefactor p = {0.086, 0.078, 0.081, 0.083} — robustly just-above-threshold.

**Honest reading:** **none of the three is RIGID at the 5% level.**
- **α_s = 7/59 is MDL-dominated** — `2/17` is both simpler and more accurate (0.215% vs 0.631%). The `b₃`-structure is *not* what makes 7/59 fit; the claim is a chance-level rational fit. *Robust.*
- **sin²θ_W = 3/13 is borderline** — it IS the best simple rational at its complexity (non-dominated, rank-3 overall), but its specialness sits exactly at the chance threshold (p ≈ 0.05). It is not statistically distinguishable from "a simple rational happened to land near 0.231."
- **m_e prefactor 16/3 is chance-level** — non-dominated but not special (p ≈ 0.08). (This tests only the prefactor. The historical claim that the exponent $\alpha^{11}$ was `[DERIVED]` from FTD-0084 was superseded by FTD-0390 and the scoped order-type theorem FTD-0397; $n=11$ remains `[SELECTION]`.)

## 3 · Demotions (applied)

Per the owner-approved "document AND run" decision, the tags are corrected to what the evidence supports (LEDGER + `CATALOG_PARAMETRIC_INSERTIONS.md`):

| Claim | Was | → Now | Reason |
|---|---|---|---|
| α_s = 7/59 | `[STRUCTURALLY MOTIVATED PARAMETRIC]` | **`[PARAMETRIC — chance-level; MDL-dominated by 2/17]`** | a simpler rational fits better |
| sin²θ_W = 3/13 | `[STRUCTURALLY MOTIVATED PARAMETRIC]` | **`[PARAMETRIC — best simple rational but not statistically special, p≈0.05]`** | borderline; structure not distinguishable from a rational fit |
| m_e prefactor 16/3 | `[SELECTION]` | **`[SELECTION — chance-level prefactor (p≈0.08)]`** | non-dominated but not special; m_e overall stays `[SMC]` (exponent-11 untested here) |

This is the F10 defense made operational: it converts "tagged" → "scan-tested" for the claims that carry the framework's gauge/mass story, and the honest result is that they are **not statistically special** — exactly the kind of self-critical finding that distinguishes the rigid core (x₊=1/α, scan-survived) from the suggestive periphery.

## 4 · Non-promotion / scope

`[MEASURED — rigidity audit]`. **Nothing promoted; the demotions are the deliverable.** The algebraic spine, N_c=3 (topological, scan-independent `[THEOREM]`), x₊=1/α (`[SMC]`, FTD-0189-rigid), MC-T4.3, FC-0/1/2 — all unchanged. The m_e *exponent* (FTD-0084) and the master quadratic are not tested here. Golden gate untouched (Python-only). This audit feeds the capstone §4 rigidity-coverage map (FTD-0311).
