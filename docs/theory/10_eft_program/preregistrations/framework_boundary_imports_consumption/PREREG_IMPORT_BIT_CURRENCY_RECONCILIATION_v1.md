# PREREG — Import bit-currency reconciliation v1

**Tag:** `[PRE-REGISTRATION — FROZEN]` · **LEDGER id (on execution):** FTD-0509 · **Date frozen:** 2026-07-25
**Git tag:** `preregister-import-bit-currency-v1` (this file committed and tagged before the runner executes).
**Runner:** `scripts/proofs/proof_import_bit_currency.py` (written after this file is tagged; its checks implement the gates below and nothing else).

## 1 · Question

The priced-import ledger (FTD-0371, `import_ledger.json`) declares its structural currency in *adopted bits*: IMP-B1 (FC-W, the δ branch) is priced at exactly 1 bit for a ℤ/2 branch selection. Independently, FTD-0499 derives inside the engine formalism that exact resolution of an `m`-way merge event costs `log2(m)` bits per event, with `N·log2(m)` the information lower bound over `N` merges. Are these two currencies the same unit — is the ledger's declared "adopted bit" the `m = 2` case of the substrate's own derived branch-selection cost — and if so, do the adopted and declined import lines differ in *scaling class* (one-time versus per-event)?

**Design-time disclosure.** The declared JSON fields (IMP-B1 `price: 1, unit: bit`; DEC-1/DEC-2 declined, unpriced) were inspected while designing these gates. The gates below concern derived quantities not yet computed at freeze time: the independent capacity recomputation (G1), the conversion-constant test (G2), and the scaling classification over a trajectory grid (G3).

## 2 · Frozen fixtures

- Ledger data of record: `docs/theory/01_reference/import_ledger.json` at the tree state of this commit.
- Derived-cost source of record: FTD-0499 (`AUDIT_FINITE_MEMORY_REVERSIBLE_LIFT.md`), radix control `h' = m·h + b`.
- Merge grid: `m ∈ {2, 8}`; trajectory lengths `N ∈ {1, …, 63}` for `m = 2` and `N ∈ {1, …, 21}` for `m = 8` (the FTD-0499 payload envelope).

## 3 · Gates (frozen)

- **G0 (guard).** No epistemic tag moves. FC-1 and FC-2 remain declined; IMP-B1's price is not edited; the verdict is confined to the outcome cells of §4. A run that suggests any tag change fails G0 and the result is discarded.
- **G1 (rate recomputation).** The minimal integer payload capacity that exactly reverses `N` independent `m`-way merges via the registered radix control equals `ceil(N·log2(m))` bits, exactly, across the full frozen grid. PASS iff no grid point deviates.
- **G2 (unit commensurability).** The declared IMP-B1 price for its ℤ/2 branch equals the derived per-fiber selection cost `log2(2)` with conversion constant exactly 1 (dimensionless). PASS iff equal with no fitted factor.
- **G3 (scaling classification).** For each import object, compute the price function `P(N)` over the frozen grid under the derived-cost model: IMP-B1 as a section of a single global 2-element fiber (the Galois orbit, event-independent); the DEC-2 object as the fiber-resolving section over the event stream (FTD-0499 §2 lower bound); the DEC-1 object's record-capacity floor per the same bound (signature level, per FTD-0508 Corollary 3). Classify each as *intensive* (`P(N)` constant) or *extensive* (`P(N)` unbounded, slope `log2(m)`).

## 4 · Outcome cells (frozen)

- **Outcome A — uniformly intensive.** All three objects classify intensive. Reading: the extensivity distinction fails; the ledger currency and the derived cost coincide trivially; no structural content beyond G2.
- **Outcome B — commensurable, split scaling.** G1 and G2 PASS; IMP-B1 classifies intensive and DEC-1/DEC-2 classify extensive. Reading of record (frozen): the declared adopted-bit and the derived merge-bit are the same unit; the adopted import is a finite one-time purchase while the declined imports are unbounded per-event purchases. This quantifies, in one currency, why FC-W was adoptable and FC-1/FC-2 were declined. The *identification* of the two currencies as one remains `[CONJECTURE — recorded]`; the arithmetic is `[NUMERICAL FACT]` over the frozen grid; no ledger line is repriced.
- **Outcome C — incommensurable or gate failure.** G1 or G2 fails, or a price is not expressible as bits of fiber selection. Reading: the reconciliation is refuted or underdetermined; record honestly and stop.

## 5 · No-flex clause

The grid, gates, and outcome cells above are frozen at tag time. No post-hoc gate, tolerance, or reinterpretation may be added. Whatever cell obtains is the verdict of record and is registered as FTD-0509 with this file cited.
