# DERIV — Observer-section equivalence: measurement capability is exactly non-tracial support

**Tag:** `[THEOREM]` (Theorems D–E, elementary and machine-verified) + `[SYNTHESIS]` (the FC-1 reading) + `[CONJECTURE]` (any bearing on physical measurement).
**LEDGER id:** FTD-0517 · **Date:** 2026-07-25
**Deepens:** [`DERIV_RECORD_STATE_DICHOTOMY.md`](DERIV_RECORD_STATE_DICHOTOMY.md) (FTD-0515, Theorems A–C) and [`DERIV_FOUR_WALLS_SECTION_SCHEMA.md`](DERIV_FOUR_WALLS_SECTION_SCHEMA.md) (FTD-0508). Inputs at tags of record: FTD-0499, FTD-0243, FTD-0509, FC-1/FTD-0255.
**Verification:** `scripts/proofs/proof_observer_section_equivalence.py` (4/4 PASS, 2026-07-25).

---

## 0 · Thesis

FTD-0515 characterized the states a fiber-resolving record must carry (non-tracial) and the states the projected substrate supports (tracial). This document asks the question those results frame: **what must a subsystem be, structurally, to function as a measuring device for the frozen projection?** In the registered FTD-0499 model the answer is an equivalence, not an implication: an observer can recover pre-merge microstates precisely to the extent that its retained algebra escapes the tracial (cyclic-invariant) subalgebra — that is, precisely to the extent that it can host a non-tracial state. Measurement capability and non-tracial support are the same property. The observer's measuring power is moreover metered: full recovery of `N` `m`-way merges requires capacity `N·log₂(m)` bits — the FTD-0509 extensive currency read as an instrument budget.

FC-1 is not adopted, weakened, or paid here. What this document supplies is the first native *characterization* of the FC-1 object's role: it says exactly what hosting the declined structure would cost and what it would buy, in the substrate's own units.

## 1 · Setting

Fix the frozen projection with `m`-way merges (FTD-0499 §1). A trajectory's lost fiber datum is its branch word `w ∈ {0,…,m−1}^N`; exact reversal from the output requires exactly `w` (FTD-0499 §3). An **observer** is a retention map `ρ` on words — the record a subsystem keeps of the export stream. Its observable algebra `A_ρ` is the functions factoring through `ρ`. Reference observers, in decreasing retention: full word; first digit; necklace (cyclic class); digit count; length; parity.

## 2 · Theorem D — recovery is separation; separation is escape `[THEOREM]`

**Theorem D.** (i) `A_ρ` supports an exact inverse of `N` merges iff `ρ` is injective on each length class (full recovery ⟺ separation). (ii) `A_ρ` distinguishes *any* pair inside one cyclic class — recovery beyond the cyclic shadow — iff `ρ` is non-constant on some cyclic class ("escape"), and this holds iff `A_ρ` hosts a non-tracial state.

*Proof.* (i) The word determines the preimage chain and conversely (FTD-0499 §3); an inverse defined on `ρ`-values exists iff distinct words have distinct values. (ii) The first equivalence is definitional. For the second: if `ρ` escapes at a witness pair `(w, r)` with `r` a rotation of `w`, the indicator of `ρ(w)` is a functional through `ρ` taking distinct values on one cyclic class — non-tracial by Theorem A of FTD-0515. If `ρ` does not escape, every functional through `ρ` is constant on cyclic classes, hence tracial. ∎ (Machine checks F1–F2: the full observer separates; each lossy observer exhibits a collision pair; escape and non-tracial support coincide across all six reference observers, `m ∈ {2,3}`, lengths to 4.)

**The witness at minimal size (F4):** the pair `01` / `10` — equal length, equal digit count, equal necklace, *distinct histories*. Every cyclic-invariant observer is blind to it; the full and first-digit observers split it, and each thereby hosts a non-tracial state. Two merges suffice: the smallest possible measurement is already order-measurement.

## 3 · Theorem E — the capacity meter `[THEOREM]`

**Theorem E.** An observer whose record holds `c` bits supports exact recovery of at most `⌊c / log₂(m)⌋` merges; the bound is achieved by the registered radix control and fails at the first merge past it.

*Proof.* Injectivity on length-`N` words needs `m^N` distinguishable record values, i.e. `c ≥ N·log₂(m)`; the radix stack achieves it (FTD-0499 §2–3). ∎ (Machine check F3, boundary cases at `m = 2, c = 5` and `m = 8, c = 9`.)

With FTD-0509 this closes a loop: the *declined* imports were exactly the extensive ones, and Theorem E shows why an instrument is the paradigm extensive purchaser — **measuring power is bought by the merge, at the declined lines' own rate.** An observer is, structurally, a subsystem that pays the extensive price locally which the framework declines to pay globally.

## 4 · The FC-1 reading `[SYNTHESIS]`, and the honest ceiling

Instance I4 of the schema names the FC-1 object as a fiber-distinguishing, order-sensitive pairing. Theorems D–E characterize the *host* such an object requires: any subsystem functioning as a measuring device for the frozen projection — in the precise sense of state recovery — necessarily escapes the tracial subalgebra, necessarily supports non-tracial states, and necessarily pays `log₂(m)` bits per resolved merge. Conversely a subsystem that stays tracial measures at most the cyclic shadow (FTD-0515 Theorem C's shelf) no matter its size.

Three restrictions bind. First, "observer" and "measuring device" here mean state-recovery functionality in the registered model — nothing about consciousness, collapse, or outcome statistics; the Born rule is untouched and unreachable from this material. Second, the equivalence characterizes what hosting the FC-1 signature *requires*; it does not construct M, does not derive quantum mechanics' specific non-commutative structure (no CCR, no ℏ), and moves FC-1 not at all — the framework still declines the import and still predicts the substrate where they differ. Third, whether any physical measuring apparatus is usefully modeled as a `ρ`-observer on an export stream is `[CONJECTURE]`; the theorems are about the model, and the model's fidelity to laboratory measurement is exactly the kind of identification the discipline forbids assuming.

## 5 · Status line

Theorems D–E `[THEOREM — elementary, machine-verified 4/4]`. FC-1 reading `[SYNTHESIS]`; physical-measurement bearing `[CONJECTURE]`. Nothing promoted, nothing adopted: FC-1/FC-2 stay declined, FC-W stays adopted, `x₊ = 1/α` stays `[SMC]`, MC-T4.3 stays `[OPEN — SCOPED NO-GO PACKAGES]`, the four-walls forcing theorem stays `[OPEN]`. What is new: the FC-1 wall now has a complete native characterization of its host — measurement capability ⟺ non-tracial support, priced per merge in the FTD-0509 currency.
