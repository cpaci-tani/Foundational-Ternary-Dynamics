# AUDIT — B-QM-1′: Spekkens Knowledge-Balance from the Internal-Observer Restriction — PARTIAL

**Tag:** `[PARTIAL]` / `[UNDERDETERMINED]` (pre-reg §6: D1 binding derived; D2 sharpness needs an additional, located ingredient). **No spine claim moved.**
**Date:** 2026-05-29
**Result of:** `PREREG_SPEKKENS_KNOWLEDGE_BALANCE_v1.md` (B-QM-1′, FTD-0227 provisional), SHA256 `79e3b7f8c4a7e4aff5887c0cd130c45f5477778400c1da4db1cd51fcdc49f2dc` (commit deferred; SHA recorded in-session before the attempt).
**Verifier:** [`scripts/proofs/proof_spekkens_knowledge_balance.py`](../../../scripts/proofs/proof_spekkens_knowledge_balance.py) (10/10).
**Companion:** `SCOPE_DERIVE_QM_GAP.md`. **Explains:** FTD-0199 / FTD-0200 (Born CLOSED-NEGATIVE → now structurally accounted for).

---

## 0 · Executive summary

**Verdict: PARTIAL.** FTD's internal-observer restriction yields the **binding** half of the Spekkens structure (derived, non-circular) but **not** the **sharp** half — and the missing ingredient is now precisely located and partly FTD-native.

- **D1 — binding: DERIVED.** From the **classical finite-self-reference limit** (pigeonhole: a finite internal observer with `M` pointer states inside an `N>M` total cannot distinguish all `N` states — in particular, states differing only in its *own* component). This is **binding in principle** for internal observers and is **non-circular** — it does *not* invoke Breuer's QM theorem or WAY (falsifier BP1 honored). This is a genuine positive: FTD's commitment to *endogenous* observers gives a theorem-grade binding epistemic horizon.
- **D2 — sharp: NOT from binding alone.** A generic binding limit is a **fixed blind spot** (the observer always loses the *same* axis) → only **one** complementary basis → **2** epistemic states — classical. The Spekkens *sharp* set is **6** epistemic states (= the 3 complementary bases `{a, b, a⊕b}` = the qubit stabilizer set), which requires the knowledge budget to apply **symmetrically to all three bases** — the full symplectic **GL(2,𝔽₂) ≅ S₃** acting transitively on them.

So binding ≠ sharp, and the gap is exactly the **symmetric conjugate budget**.

---

## 1 · How much of the symmetry is FTD-native (honest accounting)

`S₃ = ℤ/2 ⋊ ℤ/3`, and the two factors have very different status:

| Piece | Role | FTD source | Status |
|---|---|---|---|
| **ℤ/2** (transposition) | swaps two bases | **`J²=−I`** reduces mod 2 to `[[0,1],[1,0]]` — order 2, swaps `ab`, **fixes `a⊕b`** | **FTD-native** ✓ (the quarter-conjugacy) — but only *part* of S₃ |
| **ℤ/3** (3-cycle) | cyclically permutes all three bases (gives transitivity) | candidate: the cube **body-diagonal C₃** / the **N_c = 3** axis-rotation | **FTD candidate — UNVERIFIED** |

So `J²=−I` — the structure this whole thread has circled (`i = J`, the BCC complex structure) — **is** a load-bearing ingredient of the epistemic budget, but it supplies only the ℤ/2. Sharpness additionally needs a **ℤ/3 acting on the same three bases**, whose natural FTD candidate is the same 3-fold structure that gives **N_c = 3 / SU(3)** (the body diagonal). That two of FTD's deepest structures (the quarter-conjugacy and the 3-fold colour axis) converge on the epistemic budget is striking — but the ℤ/3 action on the budget is **conjecture, not verified** (it is the next target, B-QM-1″).

---

## 2 · This structurally explains FTD-0199/0200

FTD-0199/0200 empirically found the substrate gives **Rice/Gaussian** statistics, **not Born**. B-QM-1′ explains *why*:

> The restriction is **binding but not sharp** → "binding lossy access" → **classical coarse-graining with noise** → exactly Rice/Gaussian, *not* the sharp Spekkens/Born set.

The empirical closed-negatives were not a fluke of the tested substrate — they are what a binding-but-not-sharp restriction *must* produce. And the audit tells us precisely what to add to reach the quantum set: the **full symplectic S₃ budget** (specifically the ℤ/3 the substrate does not yet manifestly supply to the epistemic budget).

---

## 3 · Falsifier checklist (pre-reg §5) — all clean

| F | Fires? | Why |
|---|---|---|
| BP1 (QM-theorem import) | no | binding from **classical** self-reference; J's contribution computed explicitly over 𝔽₂, not asserted |
| BP2 (tuned split/labeling) | no | standard Spekkens 4-state system; results are structural |
| BP3 (inserted symmetry) | **no — and this is why the verdict is PARTIAL** | the symmetry is *identified as the gap*; J's contribution is honestly measured as only ℤ/2; the ℤ/3 is flagged UNVERIFIED, not assumed |
| BP4 (`[q,p]=i`/ℏ) | no | none |
| BP5 (Born/stabilizer fitting) | no | the stabilizer set is the *comparison target*, derived from the 2-subsets, not fit |
| BP6 (CODATA) | no | none |

---

## 4 · Honest accounting

- **First non-fully-negative result of the QM arc.** B1, B-QM-1 were CLOSED-NEGATIVE; this is PARTIAL with a *derived* positive (binding) and a *located, partly-native* gap (the S₃ budget).
- **Not over-sold.** It is **PARTIAL, not FOUND.** The sharp Spekkens-class is *not* derived; only the binding half is. The ℤ/3 piece — the difference between classical-with-noise and quantum — is an **unverified** FTD candidate.
- **Bell residue still separate.** Even a full FOUND on B-QM-1″ (the sharp Spekkens-class) would give only the *classically-explainable* part of QM. Bell violations remain a distinct residue needing the CA's superdeterminism (per `SCOPE_DERIVE_QM_GAP.md` §1).
- **Spine untouched.** No tag moves. `x₊=1/α` `[SMC]`. This is epistemic-program scoping.

**Bottom line:** the ψ-epistemic reframe is *productive* — recast as "derive the knowledge-balance," the program yields a real derived result (binding, via the internal-observer limit) and converts the FTD-0199/0200 closed-negatives into a *prediction* (binding-not-sharp ⇒ Rice/Gaussian). The remaining gap is sharp and partly FTD-native: supply the full symplectic **S₃** budget — ℤ/2 from `J²=−I` (have it) ⋊ ℤ/3 from a 3-fold axis (candidate: N_c=3 body-diagonal, unverified).

---

## 5 · Next target — B-QM-1″

> Does FTD's geometry supply the **full symplectic S₃** action on the epistemic budget — the ℤ/2 from `J²=−I` (verify it acts on the *budget*, not just analogically) **and** the ℤ/3 from the body-diagonal C₃ / N_c=3 structure — sharpening the binding restriction to the Spekkens knowledge-balance?

Pre-blessed caveats for B-QM-1″: (a) the symmetries must act on the **epistemic budget** (the allowed knowledge-states), not merely on the spatial lattice by analogy (the laundering risk); (b) even FOUND leaves the **Bell residue** open; (c) a classical phase space + sharp restriction gives the **Spekkens-class**, which is `ψ`-epistemic-classical, *not* full QM.

---

## 6 · Provenance & discipline

Deferred commit; pre-reg SHA `79e3b7f8…` recorded in-session before the analysis. Verifier 10/10; no QM-theorem import, no inserted symmetry, no Born fitting, no CODATA. GTCA note: mid-execution I caught my own overstatement (that `J²=−I` supplies "the symmetry") and corrected it — over 𝔽₂ it is only a ℤ/2 transposition that *fixes one basis*; the verdict reflects the honest partial contribution, which is exactly what keeps this PARTIAL rather than an over-sold FOUND.
