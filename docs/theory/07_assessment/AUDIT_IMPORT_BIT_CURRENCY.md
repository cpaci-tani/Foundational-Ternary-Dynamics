# AUDIT — Import bit-currency reconciliation

**Date:** 2026-07-25
**Identifier:** `FTD-0509`
**Status:** `[NUMERICAL FACT — frozen-grid arithmetic]` + `[CONJECTURE — recorded: currency identification]` + `[SYNTHESIS — no line repriced]`
**Verdict:** `OUTCOME B — COMMENSURABLE_UNIT_SPLIT_SCALING`
**Pre-registration:** [`PREREG_IMPORT_BIT_CURRENCY_RECONCILIATION_v1.md`](../10_eft_program/preregistrations/PREREG_IMPORT_BIT_CURRENCY_RECONCILIATION_v1.md) (git tag `preregister-import-bit-currency-v1`, committed before execution)
**Runner:** `scripts/proofs/proof_import_bit_currency.py` — gates 4/4 PASS.

## 1 · Question and result

The priced-import ledger (FTD-0371) declares a structural currency, the adopted bit; the engine formalism independently derives a branch-selection cost, `log2(m)` bits per `m`-way merge (FTD-0499). The frozen gates asked whether the two are one unit and how the priced lines scale under the derived model. All gates passed and the run landed in the pre-registered Outcome B cell:

- **G1.** The minimal payload capacity that exactly reverses `N` merges via the registered radix control equals `ceil(N·log2(m))` bits at every point of the frozen grid (`m ∈ {2,8}`, `N` to the FTD-0499 payload envelope). No deviation.
- **G2.** The declared IMP-B1 price (1 bit for the ℤ/2 δ-branch) equals the derived per-fiber cost `log2(2)` with conversion constant exactly 1. No fitted factor.
- **G3.** Under the derived-cost model, IMP-B1 classifies *intensive* (a section of a single global 2-element fiber — the Galois orbit `{x₊, x₋}` — whose price is independent of trajectory length), while the DEC-2 object (the backward pairing over the event stream) and the DEC-1 record floor (per FTD-0508 Corollary 3) classify *extensive*, price `N·log2(m)`, unbounded.

## 2 · Reading of record (frozen in the pre-registration)

The declared adopted-bit and the derived merge-bit are the same unit. The adopted import is a finite one-time purchase; the declined imports are unbounded per-event purchases. In one currency, this quantifies why FC-W was adoptable and FC-1/FC-2 were declined: the framework's adoption history is consistent with a finite-price rule — pay O(1) bits for a timeless arithmetic section, decline O(N) bits for dynamical sections over the event stream.

Three restrictions bind this reading. The *identification* of the ledger's declared currency with the substrate-derived cost is `[CONJECTURE — recorded]`; the gates establish unit-commensurability and the scaling split, not that the ledger's authors' declaration and the engine's theorem denote one object. The arithmetic itself is `[NUMERICAL FACT]` over the frozen grid only. No ledger line is repriced, no tag moves, and FC-1/FC-2 remain declined bets exactly as the constitution states them (guard gate G0).

## 3 · What this opens

If the currency identification firms, the D5 pricing rules acquire a substrate-side anchor: "adopted bits" would no longer be a declared bookkeeping unit but the same quantity the substrate's own reversible-lift theorem counts. The natural next probe is whether the *selected-type* currency rows admit the same treatment (a selected type as a section of an exhibited finite fiber, priced `log2(#alternatives)`), which would make the whole ledger currency derivational rather than declarative. That probe is not pre-registered here and no claim about it is made.

## 4 · Reproducibility

- gates: `4/4 PASS`, outcome cell `B`;
- runner: `scripts/proofs/proof_import_bit_currency.py` (implements the frozen gates only);
- data of record: `import_ledger.json` at the tagged tree state;
- no engine, production, toggle, or default touched.
