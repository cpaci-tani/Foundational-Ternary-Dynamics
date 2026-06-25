# AUDIT — α Operator-Forcing: Route-Invariance of the MC-T4.3 Boundary

**Date:** 2026-06-01
**Status:** `[STRONGLY MOTIVATED CONJECTURE no-go]` — sharpens, promotes nothing.
**Scope:** MC-T4.3 (the operational α-readout obstruction), EM sector.
**Method:** four-route adversarial workflow (force → adversarial-refute → synthesize), run
`alpha-operator-forcing` (`wf_a82f3af9-536`, 9 agents, ftd-lead-physicist type).
**Net epistemic effect:** **zero promotions, zero demotions.** `x₊ = 1/α` (FTD-0013) stays
`[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. The spine is untouched.
**LEDGER id:** FTD-0242 (assigned 2026-06-01; next-free verified — 0238–0241 spoken-for). Re-check for collision on merge-to-main per the concurrent-session id hazard.

---

## 1. The question

The master quadratic is `x² − 16G*²x + 16G*³ = 0`, with dominant root `x₊ = 137.036` matching
`1/α` to 1.26 ppm (`G* = Γ(1/4)/Γ(3/4) ≈ 2.95868`, **not** the lemniscate constant ϖ ≈ 2.622).
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
hash-locked `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1` (§5) marks exactly this as an ** OPEN OBLIGATION
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
substitution-identity facade, `docs/theory/04_coupling/archive/retracted/DERIV_ALPHA_READOUT_RESOLUTION.md` (archived 2026-06-02), retracted same
session). `[CLOSED NEGATIVE]` applies only to the bare-parity route (FTD-0233, scoped) and the eleven prior
α-derivation routes; the operator-assembly boundary remains a `[STRONGLY MOTIVATED CONJECTURE]` no-go.

## 7. The two surviving exits (per the Number-One Goal: this maps the boundary)

1. A **6th-postulate-class** input supplying the operator-assembly W (logically independent of P1–P5).
   **Sharpened 2026-06-25 (FTD-0318, `FOUND_MCT43_NATIVE_Z2_PERMANENCE.md`):** this input cannot be a
   native substrate symmetry. No FTD-native ℤ/2 (i-conjugation, ±ω wave orientation, matter/antimatter,
   lattice parity, time-reversal) can supply the `δ`-selection — all act by `ℚ`-entry operators that fix
   `ℚ(G*)`, while `δ`'s ℤ/2 is the Galois orbit `Gal(ℚ(G*)(δ)/ℚ(G*))`, realized by no substrate operator.
   Exit-(i) is therefore necessarily a **declaration** (proposed FC-4, drafted un-minted), not a derivation.
2. The **engine-native ARC-D** measurement — but ARC-D1 already returned `[CLOSED NEGATIVE]` (2026-05-30,
   `DERIV_ALPHA_READOUT_EMPIRICAL.md`: 0 macroscopic cluster fissions across 2000 seeds; the lattice is
   topologically rigid, and a count of 0 is precision-independent).

This audit establishes that **machine numerical precision is decoupled from MC-T4.3**: the obstruction
lives in the operator-assembly (math) and in topological cluster rigidity (engine combinatorics), neither
of which is the conservation-law floor. Tightening the Gauss projection (§1 of the 2026-06-01 physics
grade) reopens no route here.

---

##  §8 [CONDITIONAL — Postulate 6 is an INPUT, not a theorem; this is NOT a derivation of α]

> The chain below assumes **one additional postulate that is logically independent of the five FTD
> postulates** — it **IS** the W-CRIT-2 operator-assembly (FTD-0235 /
> `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md`), restated as a composition rule. Granting it
> reproduces the master quadratic and hence the known 1.26-ppm match. `x₊ = 1/α` (FTD-0013) **stays
> `[STRONGLY MOTIVATED CONJECTURE]`; this section promotes nothing.** Read it as **"FTD + one
> assembly-input ⟹ α-match," never "FTD derives α."** The entire content of §8 is that it *prices*
> the MC-T4.3 gap at exactly one minimal named postulate — it does **not** discharge it.

The point of §8 is bookkeeping, not derivation. §3 established that the boundary is route-invariant: the
two halves of the readout (even trace, odd source) are each forward-forced, but their *assembly* into one
2×2 operator is not. Here we make that single missing step **explicit and atomic**, so the cost of α is
visible as one named input rather than a diffuse "gap."

**The chain (numerics VERIFIED in-session at 40 dp; `G*` kept symbolic in the structure):**

1. **FORCED trace** `[DERIVED]`. `T = 16 G*²`. The `16 = |Aut(E)|²` for `E: y²=x³−x` (FTD-0006); the
   `G*² = 2π·G_BCC(0)` is the Watson BCC self-energy (`G_BCC(0) = G*²/2π`, FTD-0002). Numerically
   `T = 140.0601353744…`.
2. **FORCED odd source** `[DERIVED]`. `g = G*`. This is the J-twisted ζ-determinant ratio
   `det_ζ(D_{3/4}) / det_ζ(D_{1/4}) = Γ(1/4)/Γ(3/4) = G*` (FTD-0234): degree 1, the `√(2π)` cancels so
   there is no forbidden `√π` prefactor. This genuinely *lifts the bare-parity no-go* (FTD-0233): an odd
   clean source exists. Numerically `g = 2.9586751192…`.
3. **POSTULATE 6 — the only input** `[IMPOSED — this IS the unforced assembly W, = W-CRIT-2]`. State the
   composition rule: **the readout determinant carries exactly ONE factor of the odd source beyond the
   even trace** (one chiral half-turn):
   $$\det \;=\; T\cdot g \;=\; 16G^{*2}\cdot G^* \;=\; 16G^{*3} \;=\; 414.3924377…$$
   This is the entire logical debt. For a 2×2 operator `Tr` and `Det` are independent invariants
   (§3, §4); fixing `T` leaves `Det` free, and nothing forward-forces that `g` lands in the determinant
   slot rather than anywhere else. Postulate 6 *names* that choice. It is logically independent of P1–P5.
4. **⟹ the master quadratic.** `x² − Tx + det = x² − 16G*²x + 16G*³ = 0` has
   $$x_+ = 137.0361714582…\;,\qquad x_- = 3.0239639163…$$
   and `x₊` matches CODATA-2022 `α⁻¹ = 137.035999177` to **1.257 ppm**.

**Smuggle audit (mechanical, against `SPEC_ALPHA_READOUT_CONTRACT.md` §3 Hard Exclusion Rules).** No
`α`, no `137`, no `x₊`, no `g_c`, and no FQCR transfer matrix `M_N(t)` (which is *defined* to have the
master quadratic as its characteristic polynomial — circular, banned) is inserted anywhere. `T` and `g`
are forward-forced upstream of any physical constant; **Postulate 6 is the sole input, and it is exactly
the unforced (Tr, Det) assembly** that §3/§4 already isolated as the boundary. The value of §8 is that it
prices the MC-T4.3 gap at **one minimal named postulate** — *not* that it derives α. Adding Postulate 6
to FTD's axiom list would convert `x₊ = 1/α` from `[STRONGLY MOTIVATED CONJECTURE]` to `[DERIVED, modulo
Postulate 6]`; **whether Postulate 6 is itself FTD-native is exactly the live RSI Leg-3 question (§4, §6),
which remains `[OPEN]`.** This section does not assume Leg 3 either way.

## §9 [pointers — where the realizer of Postulate 6 lives, and why it is not unit-clean]

Postulate 6 asks for a single clean factor of `G*` (degree 1) — equivalently a clean `√G*` (degree ½) that
the readout can square into the determinant slot. Two structural facts, recorded in the home docs below,
explain why such an object **exists natively but cannot be made unit-clean**, which is *why* the assembly
is a free choice rather than a forced one:

- **The weight-½ realizer is the theta-null `θ₃(0, i)`.** At the self-dual point `τ = i`,
  `θ₃(0,i) = π^{1/4}/Γ(3/4) = √G*/(2π)^{1/4} ≈ 1.08643` is a genuine FORCED square root
  (`θ = √(θ²)`, and `θ₃(e^{−π})² = G*/√(2π)` is the catalogued degree-1 object). It is real, native, and
  weight-½ — but **measure-dressed by the archimedean `(2π)^{1/4}`**; it is not a clean `√G*`. See
  `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` (theta-null spin triplet at `τ=i`) and
  `DERIV_SPIN_STATISTICS_BRIDGE.md` §5 (the weight-½ reading).
- **The half-power cannot be unit-clean for an arithmetic reason.** Every native degree-½ object is
  prime- or measure-dressed: the J-twisted `det_ζ(D_{3/4}) = 2^{1/4}√G* ≈ 2.04553` is dressed by the
  *ramified prime* `2^{1/4}` (non-archimedean), while `θ₃(0,i)` is dressed by the *measure* `(2π)^{1/4}`
  (archimedean). The readout's needed trace is the *unit-dressed* `4√G*` (`4 = |μ₄|`). The structural
  reason a clean `√G*` is unavailable in one CM field — the ramified prime at 2, local–global, and the
  ontic/epistemic seam — is the synthesis in
  `docs/theory/09_mathematical/number_theory/EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md`.

Net: Postulate 6 has a *real* candidate realizer (the weight-½ theta-null), and the shortfall between it
and a forcing of the determinant is *exactly* one archimedean (or one ramified-prime) measure factor —
not a missing object, but a missing way to strip the dressing. That shortfall is the MC-T4.3 obstruction,
seen from the realizer side.

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
- §8/§9 (Postulate-6 pricing + realizer pointers): `MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` (theta-null spin
  triplet at `τ=i`); `DERIV_SPIN_STATISTICS_BRIDGE.md` §5 (weight-½ reading);
  `EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md` (ramified-prime / local–global / ontic–epistemic synthesis).
  §8 numerics VERIFIED in-session at 40 dp: `T = 140.0601353744…`, `g = G* = 2.9586751192…`,
  `det = 16G*³ = 414.3924377…`, `x₊ = 137.0361714582…` (1.257 ppm vs CODATA-2022), `x₋ = 3.0239639163…`.
