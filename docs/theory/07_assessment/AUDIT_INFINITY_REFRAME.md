# AUDIT — Completed-Infinity Reframe (Portfolio Triage)

**Tag:** [AUDIT] — foundational commitment change with portfolio-wide consequences.
**Date:** 2026-04-19
**Trigger:** shift from "completed-infinity" ontology (the lattice is
an infinite totality ℤ³ as a single completed object) to
"undefined-boundary" ontology (at every specified position, adjacent
sites exist; the lattice has no defined edge and no claimed
completeness).
**Status:** triage complete; per-claim dispositions below. Further
editorial work (restating individual proofs in finitary language)
queued as a separate program.

---

## 1 · What changed

**Completed infinity** (the classical analytic view) permits:
- Global integrals `∫_ℝⁿ` and sums `Σ_{all sites}` as single completed objects
- Limits `lim_{L→∞}` with definite values
- Path integrals over ALL field configurations
- Thermodynamic limits `N → ∞` as ontologically real
- RG flow "to the UV" / "to the IR" as reaching asymptotic values

**Undefined boundary** (the finitist/constructivist view) permits:
- Arbitrarily large finite computations, explicitly bracketed
- Algebraic objects defined by closed-form expressions (computable to any
  finite precision without invoking completion)
- Properties that hold at every finite scale ("for every L, P(L)")
- Local differential equations / update rules on finite regions

This is not a minor semantic change. The framework's operational
toolkit narrows, and many existing claims need either restatement or
re-derivation.

## 2 · Triage dispositions

### 2.1 · SURVIVES (no change required)

| Claim | Why it survives |
|---|---|
| Master quadratic polynomial `x² − 16G*²x + 16G*³ = 0` | Pure algebra; G* = Γ(1/4)/Γ(3/4) computable to any finite precision; no limit invoked |
| Roots x₊ = 137.036, x₋ = 3.024 | Solution of a quadratic; algebraic |
| CM curve uniqueness (d=-4 is unique among class-number-1) | Numerical scan over 7 curves, all finite computations |
| Coefficient 16 = \|Aut(E)\|² (6 routes) | Arithmetic invariants of finite groups |
| Moore integers {N_base=4, N_eff=13, b_3=7} | Finite combinatorics of 3³ Moore neighborhood |
| Phase G emergent Coulomb: α_r(r, L) = 2·r·G_L(r) | Holds AT EVERY FINITE L with R²=1.0000 at L=384; no limit used |
| Phase H scaling: α_r(g_c) = g_c²·α_r(1) | Holds at every finite L to 0.0000% (test_phase_h_coupling) |
| Phase J partition function on L=2 | Explicit finite-L calculation, 8 voxels, 1107 configs |
| Lattice Green's function G_L(r) at any specified L | Defined by a finite Fourier sum over L³ modes |
| τ_proton = ∞ ("charge conservation is exact") | Restate as "at every finite tick, Σs is conserved" — holds pointwise in time, no completed-infinity time needed |
| N_monopole = 0 (∇·(∇×J) = 0 identity) | Local vector-calculus identity; pointwise |
| D = 3 from \|Aut(E)\|² = 2^D·(D−1)! | Algebraic identity |
| Dual-prediction evidence for x+ ↔ 1/α, x- ↔ N_c | Algebraic match, no limit invoked |
| Watson identity G*²/(2π) as ALGEBRAIC identity | Chowla-Selberg: closed-form relation between gamma values and a period. Computable to any finite precision. Does NOT require a "completed infinite lattice sum." |

These are the firm content of FTD under the reframe. They total **~5
firm [THEOREM]s plus the Phase G/H/J finite-L results**, consistent with
the Phase I + Option 4 final tally.

### 2.2 · RESTATE (finitary rewording, content preserved)

| Current statement | Finitary replacement |
|---|---|
| "The lattice is ℤ³ (infinite cubic lattice)" | "The lattice is a cubic graph with no defined boundary. At every specified position, 6 axis-adjacent sites exist." |
| "In the continuum limit, property P emerges" | "For every ε > 0, there exists L such that property P(L) is within ε of its continuum counterpart." |
| "In the L → ∞ limit, G_L(r) → 1/(4π r)" | "G_L(r) approximates 1/(4π r) with error O(1/L²); at L=128, the error is ≲1%." |
| "Running coupling asymptotes to α*" | "At each finite L, the effective coupling takes a specific value; the sequence α(L) for increasing L approaches α* with rate O(1/L^p) for specific p." |
| "The whole lattice" | "A lattice region of arbitrarily large but finite extent." |
| "All configurations of the flux field" | "A finite but arbitrary-size configuration space (for L=N voxels, 3^N state configs × continuous flux)." |
| "The state space" (implicit totality) | "At each finite L, the state space has finite dimension (for L=2: 6561 raw / 1107 neutral configs)." |

**Files needing restatement:**

- `FOUND_AXIOM_ZERO.md` — remove explicit "ℤ³" ontological commitment
- Multiple `DERIV_*.md` files that use "in the L → ∞ limit" as stylistic
  framing (many occurrences in `/docs/theory/03_derivations/`)
- `DERIV_LATTICE_QED_COMPLETE.md` — restate continuum-limit checks as
  finite-L approximation statements
- `DERIV_COULOMB_SCATTERING_AMPLITUDE.md` — same

These are ~dozens of occurrences; most are stylistic rather than
load-bearing, and the restatement is mechanical once the convention is
fixed.

### 2.3 · RE-DERIVE (technical content must change, not just language)

| Claim | Why it needs re-derivation |
|---|---|
| **Master quadratic as "thermodynamic limit property of ℤ³"** (`DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` §VI) | The "limit" framing was the load-bearing justification for why the polynomial produces α. Under the reframe, the polynomial is a **pure algebraic object**, not a limit of anything. The identification with α is [SELECTION], not derivation. Phase I already established this; the reframe makes it foundational. |
| Gap-equation convergence to master quadratic | Phase I Item 1 showed the numerical claim fails. Under the reframe, the "convergence" question is not well-posed — the master quadratic is not a limit of a finite-lattice gap equation, it's an algebraic identity. Drop the gap-equation narrative in `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` or fully re-derive on a finitary basis. |
| Path integral / partition function construction (`DERIV_PATH_INTEGRAL_CONSTRUCTION.md` §5.2) | "True phase transitions require N → ∞" is a completed-infinity claim. Must be restated as "at each finite N, the free energy F_N is analytic; the sequence F_N shows increasingly sharp crossovers but no exact singularity at any finite N." The engineering content is unchanged; the ontological framing is. |
| Von Neumann Type III₁ emergence (`DERIV_VON_NEUMANN_CONSTRUCTION.md` §5) | Already tagged [SELECTION], but the reframe makes the tag **binding**: Type III₁ is not a theorem about FTD; it's a statement about what the infinite-lattice limit WOULD be IF constructed. Under the reframe, FTD is Type I at every finite L. The Type III₁ tag should be either dropped or explicitly restated as "the consciousness hypothesis requires a scale larger than any scale tested, but is not a theorem about FTD-as-defined." |
| Watson integral as "infinite-lattice Green's function" | Restate: W_3 is a **classical-analytic integral** with a closed-form expression via gamma values. It exists as an algebraic object independently of any lattice. FTD's use of W_3 does NOT require the lattice to be infinite — it requires the polynomial identity `G*²/(2π) = W_3` (Chowla-Selberg) to be known. |

**These are substantive revisions.** Each entails re-writing a proof
argument, not just polishing language. Estimate: 1-2 weeks of focused
work per document.

### 2.4 · REFRAME (ontological claims that need to be restated or dropped)

| Current stance | Reframed stance |
|---|---|
| "FTD's lattice IS ℤ³" (Axiom Zero) | "FTD is defined pointwise: at every site, the ternary state and flux fields are defined, and neighbors exist. No claim is made about the lattice's global extent." |
| "The master quadratic IS the thermodynamic limit of FTD" | "The master quadratic is an **algebraic object** whose roots match FTD's target physical constants to 1.26 ppm (x+ → 1/α) and 0.80% (x- → N_c). The connection is algebraic coincidence + structural uniqueness, not a dynamical limit." |
| "α is derived from first principles" (classical reading) | "α = 1/137.036 matches the master quadratic's larger root to 1.26 ppm; this match is structurally unique among class-number-1 CM curves; the identification is [STRONGLY MOTIVATED CONJECTURE], not [DERIVATION]." (Already established in Phase I + Option 4; the reframe reinforces it.) |
| "Path integrals over all field configurations" | "At any specified L, the classical action S_E[J, s] is defined on a finite configuration space, and extrema of S_E give classical solutions. No path integral over a completed totality is invoked." |
| "Consciousness as Type III₁ factor" | "Consciousness is modeled as a self-referential finite-but-unbounded structure. The Type III₁ label is **not** claimed as a theorem about FTD; the Araki-Woods construction is cited as a scaffold for the formal model but the emergence is hypothesized, not proven." |

## 3 · Interpretations of the 3.6× EFT gap, re-examined

Before the reframe, the gap had three interpretations:
- **A:** Engine right, dictionary wrong (a_phys miscalibration)
- **B:** Engine wrong at finite L, converges in L → ∞ limit
- **C:** α_ref isn't the target; engine is predicting something else

Under the reframe, Interpretation B **was never well-posed**. "Convergence in the L → ∞ limit" requires L → ∞ to be a meaningful limit; under the reframe, it's not. The Phase G finding was already consistent with this: α_r = 2·r·G_L(r) at EVERY finite L with R²=1.0000 — no limit needed. So B's refutation is foundational, not just empirical.

**Interpretation D (new):** Engine is correct at every finite L, and the framework's axioms should specify at which L the engine's α_r should be compared to α_ref. This is the finitist version of "convergence": instead of "in the limit," the question is "at what specified finite L does FTD predict R_engine = R_ref?"

Interpretation D is well-posed under the reframe and is the most productive path forward. It requires the framework to supply a **lattice-to-physical length conversion a_phys** (the ratio between one lattice unit and, say, a Planck length). If a_phys is derivable from {D=3, ternary, 26-Moore locality, determinism, discrete time}, the 3.6× gap becomes a computation, not a mystery. If a_phys is empirical input, that must be declared.

## 4 · Consequences for the master quadratic's epistemic status

Under the reframe:

- **Algebraic status: unchanged.** The polynomial and its roots are
  algebraic; no limit invoked. [THEOREM] for the algebra.
- **Connection to physics: unchanged.** x+ = 1/α is still [STRONGLY
  MOTIVATED CONJECTURE]; the dual match + CM uniqueness are the
  evidence.
- **"Derivation from lattice dynamics": was never possible.** Phase J
  (ultralocal action) + Phase I Item 1 (gap equation doesn't converge)
  + the reframe (L → ∞ is not well-posed) combine to close this route
  permanently. The master quadratic is not derived from FTD's
  dynamics; it's an algebraic identity that matches the dynamics'
  targets.

This is **cleaner** than the pre-reframe picture. The question "is
FTD's α a derivation or a coincidence?" was muddled by limit-language.
Under the reframe: it's an **algebraic match with structural uniqueness**,
and that's what the paper should claim.

## 5 · What this costs the project

- **Standard physics tools** (path integrals, thermodynamic limits,
  continuum QFT) require finite reformulation. Some FTD results that
  used these as proof techniques will need alternative proofs or honest
  [OPEN] flags.
- **Review friction.** Physics reviewers expect completed-infinity
  reasoning; reformulated proofs need more scaffolding. Expect longer
  response-to-reviewer cycles.
- **Some portfolio results may not survive** if their only proofs route
  through infinite-limit arguments that don't have finitary analogs.
  The Type III₁ consciousness claim is in this category — it likely
  drops to "hypothesis" rather than "derivation" under the reframe.

## 6 · What this buys the project

- **Ontological consistency.** A lattice theory that is "Z³ in the
  limit" invokes a controversial completed-infinity commitment. A
  lattice theory that is "pointwise, with no claimed boundary" is
  philosophically cleaner and aligns with constructive mathematics.
- **Sharper falsification.** "Convergence in the limit" is hard to
  falsify — any finite-L deviation can be blamed on finite-size effects.
  "At L=L_phys, the prediction is X" is falsifiable by running at L_phys.
- **Cleaner writing.** The paper can say what it actually proves (an
  algebraic identity matching physical constants with structural
  uniqueness) without dressing it as a "derivation from a completed-
  infinity lattice dynamical system."

## 7 · Recommended next actions (prioritised)

1. **[Priority 1]** Update `FOUND_AXIOM_ZERO.md` to restate the lattice
   ontology as undefined-boundary rather than ℤ³-as-totality. This is
   the load-bearing foundational change.

2. **[Priority 2]** Update `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` §VI
   to remove the "thermodynamic limit" framing. State the master
   quadratic as a pure algebraic object and cite Phase J + Phase I for
   the (now-confirmed) finding that the limit interpretation was never
   viable.

3. **[Priority 3]** Formalise the a_phys question: write a one-page
   document stating "the framework must either derive a_phys from
   lattice invariants or declare it empirical." Attempt the derivation
   if lattice invariants seem to force a specific value.

4. **[Priority 4]** Editorial pass on docs/theory/03_derivations/ to
   restate any "in the continuum limit" / "in the L → ∞ limit" language
   as finitary ε-L statements. Mostly mechanical.

5. **[Priority 5]** Revisit Type III₁ consciousness claim; either
   restate as hypothesis or drop from the core framework.

## 8 · Per-file disposition (top-level summary)

| File | Current reliance on completed infinity | Disposition |
|---|---|---|
| `FOUND_AXIOM_ZERO.md` | Load-bearing ("ℤ³") | **RESTATE** |
| `DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` | Load-bearing ("thermodynamic limit property") | **RE-DERIVE** (reframe the argument as algebraic, not limit-based) |
| `FOUND_DIMENSIONAL_COUNTING.md` | Uses "L → ∞ limit" for W_3 | **RESTATE** (W_3 is algebraic identity, not limit) |
| `DERIV_VON_NEUMANN_CONSTRUCTION.md` | Load-bearing for Type III₁ | **REFRAME** or drop |
| `DERIV_PATH_INTEGRAL_CONSTRUCTION.md` | Load-bearing ("N → ∞ for phase transitions") | **RE-DERIVE** (finite-N formulation) |
| `DERIV_LATTICE_QED_COMPLETE.md` | Stylistic ("in continuum limit") | **RESTATE** (mechanical) |
| `DERIV_LATTICE_CHIRAL_ANOMALY.md` | Stylistic | **RESTATE** |
| `DERIV_COULOMB_SCATTERING_AMPLITUDE.md` | Stylistic | **RESTATE** |
| `DERIV_BETA_FUNCTION_MEASURED.md` | Interpretive (finite-L measurements) | **RESTATE** (light touch) |
| `AUDIT_MASTER_QUADRATIC.md` | Already flags the gap-equation failure | **AUGMENT** with reframe pointer |
| `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (Phase G) | None — holds at every finite L | **SURVIVES** |
| `DERIV_PARTITION_FUNCTION_L2.md` (Phase J) | None — explicit finite-L computation | **SURVIVES** |
| `scripts/proofs/*.py` | Mix: some explicit L → ∞ claims in proof_gap_equation_scaling.py, others finite | Audit each; most are finite computations mis-labeled |

## 9 · Status snapshot after reframe

**Firm [THEOREM] (unchanged after reframe):** 5
- G* algebraic identity
- Master quadratic polynomial + roots
- CM curve uniqueness (class-number 1)
- Phase G lattice Green's function identity (at every L)
- Phase J partition function ultralocality (at every L)

**Finitary theorems (survive, finitary in nature):** several
- Moore integer invariants
- Charge conservation per tick
- D = 3 from |Aut|² = 2^D·(D−1)!
- Phase H coupling scaling

**[STRONGLY MOTIVATED CONJECTURE] (unchanged):** master quadratic α
identification + structural null predictions.

**[SELECTION] that was previously dressed as [THEOREM]:** the "thermodynamic
limit" framing is now explicitly a [SELECTION] / [CONJECTURE], no longer
a hidden limit-assertion masquerading as derivation.

**Newly flagged [OPEN]:** a_phys derivation (conversion between lattice
units and physical length).

## 10 · Reproducibility

```
docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md  # this doc
docs/theory/07_assessment/AUDIT_MASTER_QUADRATIC.md  # Phase I core audit
docs/theory/10_eft_program/DERIV_PARTITION_FUNCTION_L2.md  # Phase J
docs/theory/10_eft_program/OPEN_GC_FROM_FIRST_PRINCIPLES.md  # three-mechanism analysis
docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md  # Phase G/H
```

## 11 · One-paragraph summary

FTD is shifting from "completed-infinity" ontology (the lattice is ℤ³
as a single completed object) to "undefined-boundary" ontology (at
every specified position, adjacent sites exist; no claim about global
extent). Under the reframe, the master quadratic's status is actually
**clarified, not weakened**: it is an algebraic identity whose roots
match α and N_c via structural uniqueness, NOT a thermodynamic-limit
property of a completed-infinity dynamics. The 3.6× EFT gap's
"convergence in the L → ∞ limit" interpretation (B) was never
well-posed and is permanently refuted. Phase G/H/J results all
survive unchanged because they hold at every finite L. About a dozen
derivation documents need language edits (RESTATE), a handful need
technical re-derivation (RE-DERIVE: gap equation, path integral, Type
III₁), and the foundational Axiom Zero needs restatement
(REFRAME). The project emerges smaller, sharper, and more honest.
