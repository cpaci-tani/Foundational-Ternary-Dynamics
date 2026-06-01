# AUDIT — α Operator-Forcing: Route-Invariance of the MC-T4.3 Boundary

**Date:** 2026-06-01
**Status:** `[STRONGLY MOTIVATED CONJECTURE no-go]` — sharpens, promotes nothing.
**Scope:** MC-T4.3 (the operational α-readout obstruction), EM sector.
**Method:** four-route adversarial workflow (force → adversarial-refute → synthesize), run
`alpha-operator-forcing` (`wf_a82f3af9-536`, 9 agents, ftd-lead-physicist type).
**Net epistemic effect:** **zero promotions, zero demotions.** `x₊ = 1/α` (FTD-0013) stays
`[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. The spine is untouched.
**LEDGER id:** pending assignment (deferred — do not hardcode; confirm next-free at hash-lock).

---

## 1. The question

The master quadratic is `x² − 16G*²x + 16G*³ = 0`, with dominant root `x₊ = 137.036` matching
`1/α` to 1.26 ppm (`G* = Γ(1/4)/Γ(3/4) ≈ 2.95887`, **not** the lemniscate constant ϖ ≈ 2.622).
Read as a 2×2 operator, `(Tr, Det) = (16G*², 16G*³)`. The trace is forward-derivable `[DERIVED]`
(`16 = |Aut(E)|²` for `E: y²=x³−x`, FTD-0006; `G*² = 2π·G_BCC(0)`, the Watson BCC self-energy,
FTD-0002). The determinant needs an **odd** power of G*. **Is the `(Tr, Det)` operator structure —
the assembly that makes `x₊ = 1/α` the dominant eigenvalue of one readout operator — FORCED by
FTD-native structure (⇒ α derived), or IMPOSED (⇒ boundary)?** This is W-CRIT-2.

## 2. Method — four independent FTD-native routes, each force-attempted then adversarially refuted

| Route | Channel attempted |
|---|---|
| **jtwist** | J-twisted ζ-regularized determinant operator (the candidate odd source) |
| **bcc** | BCC body-diagonal transfer/response operator (triple-cosine Watson) |
| **cm** | Lemniscatic CM arithmetic of `E: y²=x³−x` (CM by ℤ[i], Aut = μ₄) |
| **novel** | Forced variational principle / period-ring (Hodge) valuation / K-theory co-realizability |

Each route's Force agent built the strongest honest forward chain (G* kept symbolic; banned moves =
inserting α/x₊/g_c, or using the FQCR transfer matrix `M_N(t)` which is *defined* to have the master
quadratic as its characteristic polynomial — circular). Each was then independently re-derived and
adversarially refuted by a second agent hunting for smuggling or over-statement.

## 3. Verdict — BOUNDARY, route-invariant: 0 of 4 forced

All four Force-attempts self-reported **`gap`**; all four Refute-passes upheld **`boundary`**;
**`cleanForcedRoutes = []`**. No route forces the operator structure with nothing smuggled.

**What IS forward-forced** `[DERIVED]/[THEOREM]` (every route agrees, no insertion):
- the **trace** `16G*²` (even) — `16 = |μ₄|²` (FTD-0006), `G*² = 2π·G_BCC(0)` Watson (FTD-0002);
- the **existence of a clean FTD-native odd source**: `det_ζ(D_{3/4}) / det_ζ(D_{1/4}) = Γ(1/4)/Γ(3/4) = G*`,
  degree 1, the `√(2π)` cancels so there is no forbidden `√π` prefactor (FTD-0234; reverified to 40 dp).
  This **genuinely lifts the bare parity no-go** (FTD-0233): `16G*³ = 16G*²·G*` is *assemblable* —
  strictly stronger than "trace-only."

**What is NOT forced** `[OPEN — the boundary]`:
- the **operator assembly** — that the *same* 2×2 readout carrying `Tr = 16G*²` also has `det = 16G*³`.
  For a 2×2, trace and determinant are **independent invariants**; fixing the trace leaves the
  determinant free. The det_ζ ratio supplies the odd *scalar* but forces neither the gluing nor that
  the scalar lands in the determinant slot. This is the imposed master-quadratic Vieta target (W-CRIT-2).

### 3.1 The genuinely new result: route-invariance

The four channels are not redundant — they are independent FTD-native principles (spectral ζ-determinant,
lattice transfer operator, curve arithmetic, variational/valuational/K-theoretic). **They hit the same
wall in the same place.** The variational route additionally shows the lattice action, after the Gauss
constraint, collapses to the ultralocal state cost `(c²/2 + g_c)·Σsₓ²` whose only output coupling is the
`[PARAMETRIC]` `g_c` (Mechanisms A/B/C all closed-negative) — G* is absent from the action functional and
enters only through the spectral Green's function. The valuation route shows a grading fixes at most the
*difference* of invariant degrees, never the absolute (Tr,Det) slot assignment.

**Therefore the gap is structural to the discrete ontology, not an artifact of any one readout model.**
This is the substantive advance over the standing FTD-0234 / FTD-0235 audits.

## 4. Honest correction — the co-realizability no-go is over-stated; W-CRIT-2 is the load-bearer

The Force-chains all leaned on an elaborate **C₄/C₃ co-realizability** no-go (a definite complex
structure *i* needs the stabilizer broken to one C₄ axis, since `mult_O(E) = 0` on the 8-corner module;
the odd `G*³` as a C₃-symmetric three-plane product needs C₃ about ⟨111⟩ unbroken; `⟨C₄,C₃⟩ = O`, so the
two are mutually exclusive from one preparation). **Legs 1–2 of this are machine-checked theorem-grade**
(`mult_O(E)=0`; corner module `= A1⊕A2⊕T1⊕T2`; `⟨C₄,C₃⟩=O`, order 24). **Leg 3 is NOT** — the
hash-locked `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1` (§5) marks exactly this as an **⛔ OPEN OBLIGATION
(the crux)**, with the FORCED-escape (a reducible / 3-dimensional / infinite-descended single operator
co-realizing both invariants without recollapsing) **explicitly still live**.

The adversarial layer caught this over-statement. The **honest load-bearer** for the boundary is the
*cheap* argument — 2×2 Tr/Det independence (FTD-0235 / `proof_det_identity.py`: same trace `16G*²`,
determinants `64G*⁴` vs `64G*⁴ − 1`). The elaborate symmetry machinery is *sufficient-not-necessary
corroboration* and can structurally only ever reach "unforced," never "forced-clean."

A related correction: the even-vs-odd **parity dichotomy on (Tr, Det) is a coordinate artifact** — under
the product-1 normalization `x = 4G*^{3/2}z` (SPEC_FQCR), the quadratic becomes `z² − 4√G* z + 1 = 0`
with `det = 1` (even) and `trace = 4√G*` (half-integer degree); the odd content migrates to the trace.
So "parity of the determinant" is not a basis-free obstruction; the basis-free obstruction is simply that
the assembly W is a free choice.

## 5. Classification of α: DYNAMICAL, not structural `[DERIVED, from the contrast]`

- **`N_c = 3` is structural** — forced from O_h / topology by four independent routes
  (`docs/theory/03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md`), no operator-assembly choice
  required; the value falls out of the symmetry.
- **α is dynamical** — its value rides entirely on the *unforced* assembly W. Everything the ontology
  forces (the trace; the existence of the odd source) is consistent with infinitely many other (Tr, Det)
  pairs over the same scalar ring. The specific pair making `x₊ = 137.036` dominant is *selected*, not derived.
  Same status the engine already assigns to `g_c`.

**Sharpest one-line statement:** *the substrate forces every ingredient of α's defining quadratic — the
even trace `16G*²` from the Watson integral, and the existence of a clean odd `G*` from the J-twisted
ζ-determinant — but it does not force their assembly into one readout operator, so α's value is selected
by a logically independent convention, not derived; the discrete ontology determines the menu, not the dish.*

## 6. Proof status of the boundary `[STRONGLY MOTIVATED CONJECTURE no-go]`

- **The "unforced-on-present-evidence" half is solid** `[DERIVED]`: 2×2 Tr/Det independence + the
  convergent failure of four independent forward constructions. This is enough to *withhold* the
  derivation and keep `x₊ = 1/α` at `[STRONGLY MOTIVATED CONJECTURE]`.
- **The "no FTD-native W can EVER exist" half is NOT proven** `[OPEN]` — that stronger logical-independence
  claim is RSI Leg 3, whose FORCED-escape remains live. A future single operator co-realizing the
  definite-*i* trace and a C₃-symmetric odd determinant *would* flip the verdict to FORCED and re-open
  MC-T4.3 positive. The four routes here are pre-lock adversarial refutation attempts; none constructed
  the escape, none proved it impossible.

To label this `[THEOREM]` would itself violate the discipline (cf. the retracted "conformal-anomaly"
substitution-identity facade, `docs/theory/04_coupling/DERIV_ALPHA_READOUT_RESOLUTION.md`, retracted same
session). `[CLOSED NEGATIVE]` applies only to the bare-parity route (FTD-0233, scoped) and the eleven prior
α-derivation routes; the operator-assembly boundary remains a `[STRONGLY MOTIVATED CONJECTURE]` no-go.

## 7. The two surviving exits (per the Number-One Goal: this maps the boundary)

1. A **6th-postulate-class** input supplying the operator-assembly W (logically independent of P1–P5).
2. The **engine-native ARC-D** measurement — but ARC-D1 already returned `[CLOSED NEGATIVE]` (2026-05-30,
   `DERIV_ALPHA_READOUT_EMPIRICAL.md`: 0 macroscopic cluster fissions across 2000 seeds; the lattice is
   topologically rigid, and a count of 0 is precision-independent).

This audit establishes that **machine numerical precision is decoupled from MC-T4.3**: the obstruction
lives in the operator-assembly (math) and in topological cluster rigidity (engine combinatorics), neither
of which is the conservation-law floor. Tightening the Gauss projection (§1 of the 2026-06-01 physics
grade) reopens no route here.

---

### Provenance
- Workflow: `alpha-operator-forcing` run `wf_a82f3af9-536` (4 routes × force+refute + synthesis).
- Canonical anchors: `SPEC_ALPHA_READOUT_CONTRACT.md` (§3 Hard Exclusion Rules);
  `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` (FTD-0235, W-CRIT-2);
  `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` (FTD-0234, the odd source);
  `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md` (§5 Leg 3 = open obligation);
  `FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129, the 4-leg "α is dynamical" diagnostic);
  `DERIV_ALPHA_READOUT_EMPIRICAL.md` (ARC-D1 closed-negative);
  `scripts/proofs/proof_readout_multE_zero.py`, `proof_det_identity.py`, `proof_bcc_complex_structure.py`.
