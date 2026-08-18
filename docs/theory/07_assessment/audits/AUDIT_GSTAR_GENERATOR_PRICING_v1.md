# AUDIT — Is the G\* generator priced? The ℤ[i] reading and the BCC-layer readout v1

**Status:** `[AUDIT — PRICING RECONCILIATION]` + `[CORRECTION — TWO REFUTED FORCING GROUNDS]` + `[OPEN — OWNER RATIFICATION OF ANY LEDGER CHANGE]`
**Date:** 2026-08-16
**Draft row:** none booked. Proposed id **FTD-1013** (`scripts/audit/check_registry.py`); this document books nothing by itself.
**Moves no tag.** No theorem, no identity, and no epistemic grade is changed here. `G*` is untouched. The master quadratic is untouched. Watson 1939 is untouched.
**Precedence:** LEDGER > constitution > this audit > other prose.

---

## 0. The question

`SPEC_IMPORT_LEDGER.md` (FTD-0371) books **1 adopted bit**, **4 selected types**, 4 named results, 3 calibrations. Its self-set column credits **SS-2 — FC-0, the ℤ[i] reading** — as *self-set*, `frontier_half: "modulus"`, i.e. the half the ledger defines as **forced / magnitude-determined / unique** rather than **chosen / branch-selecting**.

Every known route to `G*` factors through that reading. This audit asks one question:

> **Is the ℤ[i] reading actually free, and is the BCC-layer readout a second, unpriced import?**

The question is a *pricing* question. It is not a challenge to any mathematics.

---

## 1. Method

Two independent corpus censuses of routes to `G*` (theory, scripts, engine, papers) plus direct recomputation of the load-bearing numerics. Every numerical claim below was computed for this audit, not quoted:

| quantity | computed | agrees with |
|---|---|---|
| `G* = Γ(1/4)/Γ(3/4)` | `2.95867511918863889231082135773` | `2ϖ/√π`, exact to 30 dps |
| `Γ(1/4)Γ(3/4)` | `4.44288293815836624701588099006` | `π√2`, exact |
| `W_BCC` | `1.39320392968567685918424626033` | `Γ(1/4)⁴/(4π³) = G*²/(2π) = π/Γ(3/4)⁴`, exact to 30 dps |
| `₂F₁(¼,¼;1;1)²` | `1.393203929685676859184246` | `₃F₂(½,½,½;1,1;1)` — Clausen, exact to 25 dps |
| `W_SC` (Bessel form) | `1.5163860591519768…` | disc −24 Chowla–Selberg form, 15 digits |
| `x₊ = 4G*(2G*+δ)` | `137.036171458155…` | CODATA `1/α` at **1.257 ppm** |
| O_h elements with `J² = −I₃` | **0 of 48** | determinant obstruction |

---

## 2. Finding A — the ℤ[i] reading needs a plane, and the plane is not priced

**FC-0 as declared** (`SPEC_FTD_FRAMEWORK_V1.md` §1.2): *"The cubic lattice's order-4 **planar** symmetry is read as the arithmetic of the Gaussian integers ℤ[i]."* Tagged `[AXIOM]`-class, explicitly a modelling choice: *"Declaring it does not derive it."*

FC-0 is thus honest about being **planar**, and the odd-dimension obstruction does not refute it. It relocates the cost:

**The obstruction (elementary, exceptionless).** A real `J` with `J² = −I_n` forces `det(J)² = (−1)ⁿ`, so **no real complex structure exists in odd dimension**. Verified against the lattice's actual symmetry group: **0 of O_h's 48 signed-permutation elements square to −I₃**, though 12 have order 4. A C₄ rotation about an axis gives `J² = (−1,−1,+1)` — `−I` on the perpendicular plane, `+1` on the fixed axis. **ℤ[i] acts only after a 2-plane is selected**, which breaks O_h → C₄ᵥ.

**The corpus already knows this, and prices it — elsewhere.** In the α-readout programme it is machine-checked and load-bearing:

> *"there is **no O-symmetric 2-dimensional subspace** on which a complex structure (`J²=−I`, needing an even-dimensional block) can act; a definite `i` requires **breaking** `O` to a single `C₄` axis (FTD-0231), so `C₃(⟨111⟩) ∉ Stab`."*
> — `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md` Leg 1 (DISCHARGED); same result carries §II.3(a) of the construction monograph.

That fact discharges a preregistration leg there. In `SPEC_IMPORT_LEDGER.md` it is absent: the self-set section contains no mention of a plane, an axis, C₄, O_h, or dimension parity. **The same fact is a cost in one programme and a free credit in the other.**

**Three further defects in SS-2's justification.**

1. **Provenance points at a stipulation.** SS-2 cites FTD-0244 for *"i is a native generator."* FTD-0244 **axiomatizes** `J` with `J² = −I` as generator #2 of its admissible set 𝒮; it does not derive nativeness, and its `[THEOREM]` is explicitly *"relative to the stated generator set."*
2. **That set's adequacy is itself an import.** 𝒮's representativeness is priced as **IMP-S3** `[SELECTION]`. A self-set credit rests on a stipulation inside a calculus whose sufficiency is booked in the priced column.
3. **The sibling row was already narrowed on these grounds; this one was not.** FTD-0839 corrected SS-3 — *"FC-0/`i` alone does not force the twisted circle, chiral half-line, origin, scale, operator order, or multiplicity"* — and its own row opens *"**A selected** `J` has order four."* SS-3 absorbed the correction; SS-2 did not.

**Minor defect, recorded for repair:** the PREREG's parenthetical *"the symmetric average `(J_x+J_y+J_z)/3` squares to `−I/3`"* is **not correct as written** — `S²` has diagonal −1 under all eight orientation conventions, so `(S/3)²` has diagonal −1/9 with nonzero off-diagonals, and no convention yields `−3I`. The conclusion it supports (no symmetric `i`) is nonetheless correct and independently established.

---

## 3. Finding B — the BCC-layer readout is priced nowhere, and its stated forcing ground is refuted

**The three cubic sublattices land on three different CM fields:**

| layer | Watson integral | CM discriminant |
|---|---|---|
| **SC** | `√6/(32π³)·Γ(1/24)Γ(5/24)Γ(7/24)Γ(11/24)` | **−24**, ℚ(√−6) |
| **BCC** | `Γ(1/4)⁴/(4π³) = G*²/(2π)` | **−4**, ℚ(i), j=1728 |
| **FCC** | `Γ(1/3)`-class | **−3**, j=0 |

Reading the BCC layer rather than SC or FCC is therefore **strictly load-bearing** for obtaining `G*` at all.

**It is booked in none of the seven pricing artifacts** — `SPEC_IMPORT_LEDGER.md`, `import_ledger.json`, `SPEC_UNIFIED_AXIOM_REGISTER.md`, `unified_axiom_register.json`, `SPEC_ADOPTION_PRICING_RULES.md`, `adoption_pricing.json`, `CATALOG_PARAMETRIC_INSERTIONS.md` — as a selected type, adopted bit, calibration, named result, or parametric insertion. The four selected types are D=3, the singlet, the ℭ generator-set, and the gauge-connection carrier. None is this.

> **⚠ SELF-CORRECTION (same day, before this audit was circulated).** A first draft of this section said the choice was *"priced nowhere."* **That is wrong and is withdrawn.** The choice is *absent from the seven pricing artifacts*, but it is **explicitly identified as a selection or an import in at least three committed analysis documents**:
>
> - `FOUND_ONTIC_CHAIN_v1.md` §7: *"**Step 7 — which lattice `[SELECTION]`**"*, ledger row *"| 7 | BCC vs SC vs FCC | **SELECTED** |"*, and *"`G*` | step 7 | **selected — via the choice of BCC**"* — concluding *"the chain reaches `G*` only through the choice of BCC … this shows the **Green's-function** door is also a selection, differently located."*
> - `SCOPE_DISCRETE_FEYNMAN_PROGRAM.md` §2: *"**`G*` is a BCC selection at one loop**."*
> - `ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md` §(iii): *"**Every hull-bounded class imports the d = −4 bit**."*
>
> **This changes the finding's character entirely: it is a reconciliation failure, not a discovery.** The corpus already knows. Nothing propagates it to the pricing layer.

**And the third of those documents answers this audit's own question.** `ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md` establishes that the BCC-layer restriction, the C₄ character restriction, and the ℤ[i] CM-point restriction are **not three choices but one**:

> *"These three restrictions are **one bit expressed three times**: choose the order-4 (ℤ[i], d = −4) reading of the cubic symmetry over the order-3 (ℤ[ω], d = −3) reading. **That bit is FC-0, verbatim.**"*

Its Corollary 2.1 further disposes of the surviving multiplicativity motivation: *"'restrict to the multiplicative layer' is **extensionally the same choice re-described**, and nothing in the postulates says self-energies of non-multiplicative layers are not outputs … this makes FC-0 a **motivated** `[AXIOM]`, not an earned theorem."*

**So Finding B collapses into Finding A rather than adding to it** — and in doing so sharpens it into a flat contradiction between two documents:

| document | verdict on the d = −4 bit |
|---|---|
| `ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md` §(iii) | every hull-bounded class **imports** it |
| `import_ledger.json` SS-2 | booked **self-set**, `frontier_half: "modulus"` (forced/unique) — a **credit** |

One document prices FC-0 as an import; the ledger credits it as free. That is this audit's core finding, and it was latent in the corpus before this pass.

**Its stated ground was that it is forced, and that ground fails.** `DERIV_WATSON_GSTAR_IDENTITY.md` §7.4 argued *"The curve is not chosen — it is forced by the lattice axiom"* from Z₄ planar symmetry, and adjudicated SP1a as `[THEOREM]`. **That document's own §7.3 table lists SC with planar symmetry Z₄ and no Γ(1/4) closed form.** A property both SC and BCC possess cannot select between them. Corrected in both files this session; tags flagged, not moved.

**What actually discriminates** is *multiplicativity*: the BCC eigenvalue is a triple product `1 − cos k₁cos k₂cos k₃`, SC's is a sum. By **Clausen's identity** the triple product is the square of a ₂F₁ with quarter parameters — `₂F₁(¼,¼;1;1)² = ₃F₂(½,½,½;1,1;1) = Γ(1/4)⁴/(4π³)`, verified to 25 dps. The 4 enters through multiplicativity, **not** through a four-fold axis. Note the BCC sublattice is spanned by the eight ⟨111⟩ body diagonals, whose axes are *three*-fold.

---

## 4. Finding C — the systemic one: the pricing layer is not reconciled with the LEDGER

Finding B is an instance, not an isolated miss. At least three BCC-related selections are tagged or flagged in the claim layer and appear in **none** of the seven pricing artifacts:

| row | what it prices | tag of record | in pricing layer? |
|---|---|---|---|
| **FTD-0313** | routing the EM kinetic operator onto BCC | `[SELECTION + THEOREM-NEGATIVE — route-invariant; closed]` | **no** |
| **FTD-0819** | sublattice ontology (site-set vs 3-cochain) | `[OPEN — TWO PRICED, NEITHER CHOSEN]` | **no** |
| **FTD-0029** | BCC multiplicative structure | `SELECTION` | **no** |

A grep for `FTD-0313`, `FTD-0819`, `FTD-0029` across all seven pricing files returns **zero hits**.

> **The import ledger's counts are a record of what was entered, not a reconciliation against the LEDGER's own `[SELECTION]` rows.** Nothing in the toolchain closes that loop: `proof_import_ledger.py` is an internal-consistency verifier (8/8), not an adjudicator, and it enforces falsifiers only on imports — the self-set column carries no falsifier requirement at all.

This is the finding with the widest reach, because it means the marquee figure ("1 adopted bit, 4 selected types") is not underwritten by any process that would catch an omission.

---

## 5. What this would cost, if ratified

**Corrected costing** (the first draft over-counted; see §3's self-correction). Because `ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md` establishes the layer, character, and CM-point restrictions are **one bit**, the reclassification is *not* a new purchase — it is a **column move of an existing line**:

- **SS-2 leaves the self-set column** and becomes a priced line. Selected types **4 → 5**, or (if the owner takes the "one bit expressed three times" framing literally) the existing **adopted-bit count 1 → 2**, since FC-0 is a single binary choice — order-4 over order-3.
- **Or SS-2 narrows**, crediting *"the lattice has order-4 planar rotations"* as self-set (genuine, forced) while the *definite `i`* — which needs a plane, breaking O_h → C₄ᵥ — moves to the argument half.
- **No tag moves anywhere else.** `G*` remains an identity. `W₃ = G*²/(2π)` remains Watson 1939. The spine's seven theorem-grade results are untouched. `x₊ = 1/α` remains `[SMC]` with uniqueness support still at zero (FTD-0791/0802) — this audit neither helps nor harms it.

The honest headline change is narrow but real: **the generator through which every route to `G*` passes would move from the free column to the priced one** — and it would do so by *importing a verdict the corpus has already reached elsewhere*, not by discovering anything new.

---

## 6. How to refute this audit

Stated so it can be attacked rather than accumulated:

1. **Show the plane choice is free** — exhibit an O_h-invariant complex structure on the relevant module, or argue that FC-0's "planar" quantifies over planes without selecting one *and* that the downstream `V_complex ≅ ℤ[i]²` inherits no axis. (Leg 1 of the readout preregistration is evidence against, but it is scoped to the readout operator.)
2. **Show the BCC readout is forced** — by a ground other than Z₄ symmetry, which is refuted. The Moore-layer decomposition distinguishes BCC (it is the only layer exciting all three J-components), but no document derives from that that *its Green's function* is what the master quadratic must read.
3. **Show these are already priced** under names this audit failed to match. The seven files searched are listed in §3; a hit anywhere in them retires Finding B.
4. **Show the reconciliation exists** — a process, test, or verifier that propagates LEDGER `[SELECTION]` rows into the pricing layer. Finding C dies immediately if one is produced.

---

## 7. Draft LEDGER row — **NOT BOOKED**

> | **FTD-1013** | **Is the ℤ[i] reading that generates every route to G\* actually self-set, and is the BCC-layer readout priced?** | `[AUDIT — PRICING RECONCILIATION]` + `[CORRECTION — TWO REFUTED FORCING GROUNDS]` + `[OPEN — OWNER RATIFICATION]` | NEW 2026-08-16 — `AUDIT_GSTAR_GENERATOR_PRICING_v1.md`. **(A)** FC-0 is declared over *planar* order-4 symmetry and is `[AXIOM]`-class by its own text; a **definite** `i` additionally requires selecting a 2-plane, since no real `J` with `J²=−I` exists in odd dimension (**0 of O_h's 48 elements square to −I₃**; determinant obstruction, exceptionless). That requirement is machine-checked and load-bearing inside the α-readout programme (Leg 1, DISCHARGED: *"a definite `i` requires breaking `O` to a single `C₄` axis"*) but appears nowhere in the import ledger's self-set column. SS-2's provenance cites FTD-0244, which **stipulates** `J²=−I` as generator #2 of 𝒮 rather than deriving it, and 𝒮's representativeness is separately priced as IMP-S3 `[SELECTION]`; FTD-0839 already narrowed the sibling row SS-3 on these exact grounds. **(B)** The **BCC-over-SC/FCC readout is booked in none of the seven pricing artifacts**, though the three sublattices give CM discriminants −4/−24/−3 and only BCC yields `G*`. Its stated forcing ground (Z₄ planar symmetry ⇒ Γ(1/4)) is **refuted by the same document's own table**, which lists SC as Z₄ with no Γ(1/4) closed form; independently recomputed `W_SC = 1.5163860591519768…` = the disc-−24 Chowla–Selberg form to 15 digits. The actual discriminator is **multiplicativity via Clausen** — `₂F₁(¼,¼;1;1)² = ₃F₂(½,½,½;1,1;1) = Γ(1/4)⁴/(4π³)`, 25 dps — and the BCC sublattice is spanned by *three*-fold ⟨111⟩ axes, so the 4 is not inherited from a four-fold axis. ⚠ **(B) is a reconciliation failure, NOT a discovery — an over-strong first draft saying "priced nowhere" was withdrawn the same day.** Three committed documents already call it a selection or an import: `FOUND_ONTIC_CHAIN_v1.md` §7 (*"which lattice `[SELECTION]`"*, *"`G*` … selected — via the choice of BCC"*), `SCOPE_DISCRETE_FEYNMAN_PROGRAM.md` §2 (*"`G*` is a BCC selection at one loop"*), and `ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md` §(iii) (*"every hull-bounded class **imports** the d = −4 bit"*). That last document also **collapses (B) into (A)**: the BCC-layer, C₄-character, and ℤ[i]-CM restrictions are *"**one bit expressed three times** … **That bit is FC-0, verbatim**"*, with Cor. 2.1 adding that the multiplicativity motivation is *"extensionally the same choice re-described"*, leaving FC-0 a **motivated `[AXIOM]`, not an earned theorem**. **The sharpened core finding is a flat contradiction: `ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md` prices the d = −4 bit as an IMPORT; `import_ledger.json` SS-2 credits the same bit as SELF-SET on the forced/modulus half.** **(C) Systemic:** FTD-0313, FTD-0819, FTD-0029 all carry `[SELECTION]`-class tags and appear in **zero** of the seven pricing files; no verifier reconciles LEDGER selections or analysis-layer verdicts into the import ledger, and the self-set column carries no falsifier requirement. **(D) Stale forcing claims:** the refuted Z₄-forcing ground survives in `PAPER_1A_WATSON_LATTICE_BRIDGE.tex` (Thm `z4-forces-k`), `SCOPE_DISCRETE_FEYNMAN_PROGRAM.md:20`, and three uncorrected rows of `AUDIT_HIDDEN_SELECTIONS.md` (126/231/343); `gap_equation_layer_convergence.py:46` carries a stale comment contradicting its own corrected docstring. **Moves no tag; books nothing.** `G*`, `W₃ = G*²/(2π)`, the master quadratic, and the spine's theorem-grade results are untouched; `x₊=1/α` stays `[SMC]` with uniqueness support still zero (FTD-0791/0802). Corrections applied to `AUDIT_HIDDEN_SELECTIONS.md` §SP1a and `DERIV_WATSON_GSTAR_IDENTITY.md` §7.4 — both **flagged, not applied**: the `[THEOREM]` grades stand pending owner review, with their stated grounds recorded as failed. Four named refutation routes in §6. |

---

*This audit prices nothing by itself and ratifies nothing. Every change it proposes to `import_ledger.json` requires owner ratification and its own verifier pass.*
