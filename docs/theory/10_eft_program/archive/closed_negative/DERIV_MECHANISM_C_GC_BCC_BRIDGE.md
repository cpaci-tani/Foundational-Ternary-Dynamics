# DERIV — Mechanism C: g_c as bridge-operator eigenvalue on σ_BCC

**Tag:** [CONJECTURE]
**Date:** 2026-04-26
**LEDGER row:** FTD-0093
**Dependencies:** FTD-0001 (master quadratic THEOREM), FTD-0013 (x₊ = 1/α SMC), FTD-0014 (x₋ ≈ N_c SMC), FTD-0028 (Moore Layer Theorem), FTD-0029 (BCC multiplicative structure SELECTION), FTD-0050 (Link 8 closure — engine stencil ⊥ BCC), FTD-0051 (Langevin thermostat), FTD-0059 (THEOREM_A_PHYS_NO_GO), FTD-0094 (L2 candidate identity), FTD-0095 (Bridge Functional), FTD-0096 (μ-from-ℓ_P missing arrow)
**Supersedes:** Mechanism A (Dirac quantization, ruled out), Mechanism B (`DERIV_MECHANISM_B_GC_DERIVATION.md`, CLOSED NEGATIVE 2026-04-25)
**Status:** Pre-registered structural argument with falsifier in `PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`. Conditional promotion to [SELECTION] if D6 PASS + circularity self-test (§7) clean.

---

## 1 · Problem statement

The fine-structure-like coupling `g_c` of FTD's wave equation
(`G_C` in `engine/include/ftd/constants.h`) currently enters as a calibrated
constant:

  g_c² = α    (engine convention; ALPHA_EFT identity static_assert'd)

This is a CALIBRATION, not a derivation. Multiple attempts to derive g_c
from first principles have closed:

| Mechanism | Route | Status |
|---|---|---|
| A | Dirac quantization / topological | Ruled out — no compact U(1) gauge structure on the projected stencil |
| B | Lattice → continuum matching at fixed-point | **CLOSED NEGATIVE 2026-04-25** — circular: target reference is x₊ itself (`DERIV_MECHANISM_B_GC_DERIVATION.md`) |
| C | Bridge-operator eigenvalue on σ_BCC | **THIS DOCUMENT** — pre-registered |

Mechanism B closed because the matching condition required a continuum
reference value of g_c that we wished to derive from x₊ — feeding x₊ in
to compute x₊ out. Mechanism C aims to side-step that circularity by
defining g_c as an eigenvalue of a bridge operator constructed from
Axiom-Zero invariants on σ_BCC, with **no a priori reference to x₊**.

The pre-registered prediction below tests whether the construction lands
on the master-quadratic spectrum *automatically* — not because we put it
there.

What success looks like:

> A bridge operator `B : J → s`, defined on the BCC sub-stencil σ_BCC ⊂ Moore-26
> using only Axiom-Zero invariants {D=3, ϖ, framework integers, c_lat, lattice
> kinematics}, has band-edge spectrum {α·x₊, α·x₋} where (x₊, x₋) are the
> master-quadratic roots and α is the EM-coupling normalization. The
> calibration-invariant ratio `m₊/m₋ = x₊/x₋ ≈ 45.31` emerges from the
> spectrum without any reference to CODATA α or m_e.

What failure looks like:

> The BCC band-edge spectrum recovers `m₊/m₋ ≠ 45.31` at the L→∞ extrapolation,
> OR the master-quadratic-side claim works only when we tacitly insert x₊
> into the construction (the Mechanism B failure mode replicated).

## 2 · What the bridge operator requires

| Ingredient | Definition | Status in FTD |
|---|---|---|
| (i) Sublattice projector Π_BCC | Linear projector onto `σ_BCC ⊂ Moore-26` | DERIVED in `engine/include/ftd/sublattice.h` (E1, E2 of the build plan); operator weights `W_BCC_CORNER = 1/8` are unique for a consistent discrete Laplacian on the body-diagonal sub-stencil |
| (ii) Flux kernel K_J | The dispositional → action transformation J → ∂_t s | partially derived: emerges from the manifestation rule (FTD genesis at \|J\| > K_GENESIS·K_B); the *specific form* on σ_BCC is what this document attempts to fix |
| (iii) Bridge operator B | B := Π_BCC ∘ K_J ∘ Π_BCC^* | not yet formally defined as an operator; §4 fixes the construction |
| (iv) Spectral theorem | `Spec(B) = α · {x₊, x₋}` | **THE LOAD-BEARING CLAIM** — §4-5 |

## 3 · The σ_BCC sub-stencil construction

The Moore-26 neighborhood decomposes as the direct sum of three
sub-stencils with disjoint neighbor sets (Watson 1939; FTD-0028):

  Moore-26 = σ_SC (6 face nbrs) ⊕ σ_FCC (12 edge nbrs) ⊕ σ_BCC (8 corner nbrs)

The corresponding Watson integrals satisfy
  I_3 ≈ 0.506 (SC),  I_2 ≈ 0.446 (FCC),  I_1 = G*²/(2π) (BCC) [THEOREM, FTD-0029]

The BCC sub-stencil is THE one where the master-quadratic identity
`x₊ + x₋ = 16 G*² = 32 π · I_1` lives at the algebraic level (FTD-0029
multiplicative structure: Watson identity W_3 and SU(3) gauge group both
arise from the BCC eigenvalue's triple-cosine product).

The FTD-0050 Link-8 closure proved that the engine's 18-pt coupling stencil
is structurally orthogonal to σ_BCC:

  σ_engine = (σ_SC + σ_FCC) / 2 ⊥ σ_BCC

This is the *negative* result that motivates Mechanism C. The master-
quadratic spectrum cannot be probed by the engine's default 18-pt path; it
requires a BCC-projected variant. The engine infrastructure for that variant
is now in place (`bcc_stencil` toggle in `term_toggles.h`, BCC-only Laplacian
in `sublattice.h`, sublattice-filtered Langevin in `phase_write`,
sublattice-filtered correlators in `correlations.h`).

The pre-registered question Mechanism C tests is whether the BCC band-edge
eigenvalues, measured under thermalized Langevin dynamics on σ_BCC, line up
with `α · {x₊, x₋}`.

## 4 · The bridge operator B (formal definition target)

Working definition (target — to be made precise in subsequent passes):

  B  :=  Π_BCC ∘ ∇_J · K  ∘ Π_BCC^*

where:
- Π_BCC is the BCC-sub-stencil projector (defined; W_BCC_CORNER = 1/8).
- ∇_J · is the divergence operator on the flux field J.
- K is the genesis kernel — the threshold operator
  `K(J) := Θ(|J| − K_GENESIS·K_B) · sign(J)` from `phase_write`.
- The composition acts on the flux history and projects to the state-field
  manifestation rate ∂_t s.

This is **not yet an operator in the strict sense** — K is a threshold,
not linear. The first task of the Mechanism C closure is to linearize K
about the ground state and define `B_linear` whose spectrum can be computed.

Open subquestions (must be resolved for the construction to be usable):

(a) **Linearization of the genesis kernel.** Around what background?
    Vacuum (|J| = 0)? Mean-field (|J| = K_GENESIS·K_B)? Thermalized BCC
    ground state (the Langevin equilibrium at small T)? Different choices
    give different linearized operators.

(b) **The eigenvalue claim.** What does it mean for `Spec(B_linear) = α·{x₊, x₋}`?
    Are these eigenvalues of the spatial transfer matrix? Of the temporal
    correlator decay rate? Of a band-edge dispersion? The pre-registered
    prediction in §5 commits to one specific reading: temporal correlator
    decay rates of the thermalized BCC-projected energy time series.

(c) **The α normalization.** Where does α enter? In the engine, `G_C = √α`
    is the coupling that enters phase_read as the gradient_state scale. In
    the master-quadratic algebra, α = 1/x₊. If Mechanism C makes both
    consistent simultaneously without circularity, that consistency is the
    derivation. If it requires inserting α to extract α, it has replicated
    the Mechanism B failure.

## 5 · Pre-registered prediction

Calibration-invariant prediction, to be tested by `PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`:

**P1 (RATIO):** The thermalized BCC sub-stencil correlator
  `C(τ) = ⟨ψ(t) ψ(t+τ)⟩_t,    ψ(t) = Σ_{i ∈ BCC sites} |J(i,t)|²`
  admits a two-exponential decomposition with decay rates `(λ₊, λ₋)`
  satisfying
  λ₊ / λ₋  =  x₊ / x₋  ≈  45.31
  to ppm precision in the L→∞ extrapolation.

**P2 (SUM):** The same decomposition satisfies, in dimensionless lattice
  units,
  λ₊ + λ₋  =  16 G*²  ≈  140.06
  to 1% at L=64 (subject to the lattice-anisotropy correction estimated in
  AUDIT_LORENTZ_ANISOTROPY).

**P3 (CONTROL):** On σ_SC the ratio λ₊/λ₋ does NOT equal 45.31. On σ_FCC
  the ratio does NOT equal 45.31. On the legacy 18-pt FULL stencil the
  ratio does NOT equal 45.31 (per FTD-0050 orthogonality).

**P4 (FINITE-SIZE):** The residual `Δ_L = λ₊/λ₋ − 45.31` scales as
  Δ_L ∝ 1/L²
  with χ²/dof < 2 across L ∈ {16, 24, 32, 48, 64}. (1/L² is the expected
  Symanzik a²-improvement scaling for an isotropic discrete Laplacian; any
  other scaling indicates a different systematic.)

**P5 (WARD):** The current Ward identity for the conserved flux current
  restricted to σ_BCC is satisfied to L∞ < 10⁻⁸ at L=64 (matched-stencil CG
  Poisson, not engine SOR — see AUDIT_WARD_IDENTITY).

## 6 · Falsification criteria

The Mechanism C [CONJECTURE] tag flips to [CLOSED NEGATIVE] if **any** of:

| Criterion | Threshold | Origin |
|---|---|---|
| (a) BCC ratio measured at L=64 | \|ratio − 45.31\| / 45.31 > 5σ statistical error | P1 |
| (b) BCC ratio recovered on a control stencil (SC or FCC) | one of those matches 45.31 within 1% | P3 contradiction → bridge specificity broken |
| (c) Finite-size scaling | not 1/L² (e.g., 1/L or constant) | P4 — wrong systematic |
| (d) Ward residual on σ_BCC | L∞ > 10⁻⁸ | P5 — current is not conserved on σ_BCC |
| (e) Look-elsewhere positive | scan finds 1+ comparable 10⁻⁴ matches in unrelated targets | FTD-0097 — strong evidence of catalog over-richness |

If only (a) trips, the closure is partial: the bridge functional may be
arithmetic-mean (FTD-0095 §2 [OPEN]) but with a non-zero residual. That
sub-case re-routes to FTD-0094 [PARTIAL] in `AUDIT_BCC_SUBLATTICE_RESULTS.md`.

If (b) trips, the bridge does not live on σ_BCC; the framework retains the
algebraic core (FTD-0001) but loses the dynamical reading entirely.

If (c) trips with (a) clean, the bridge dynamic is fine but the lattice-
anisotropy systematic is misidentified; rework AUDIT_LORENTZ_ANISOTROPY
applied to BCC kernel.

## 7 · Self-circularity test (mirrors Mechanism B §6)

This is the load-bearing self-check. Mechanism B closed negative because
its target reference (continuum g_c) was x₊-derived. The analogous test
for Mechanism C:

> **Q.** In the construction of B (§4), do any of {Π_BCC, K_J, the
> linearization point, the α normalization} require x₊ as an INPUT,
> as opposed to producing x₊ as an OUTPUT?

Audit table:

| Ingredient | Inputs required | Source | x₊-circular? |
|---|---|---|---|
| Π_BCC weights (1/8 corner) | Watson sub-stencil decomposition | FTD-0028, FTD-0029 | NO — algebraic, not spectral |
| `c_lat = 1/√3` | dimensionality D=3 | FTD-0001 axiom | NO |
| K_GENESIS = 3·K_B | manifestation rule | SPEC_FTD §6.2 | NO (depends on K_B = m_e calibration, not on x₊) |
| Linearization background | TBD: {vacuum, mean-field, thermalized} | this document, open | **CHECK CAREFULLY** |
| α normalization for spectrum | TBD: from G_C = √α (engine) or 1/x₊ (algebra) | this document, open | **CRITICAL — if α := 1/x₊ is inserted, circular** |

The "α normalization" line is where Mechanism C is at greatest risk.
Two readings are possible:

- **Reading R1 (constructive).** The α normalization comes from the
  bridge operator's *own* spectrum: B's ground-state eigenvalue, computed
  from Axiom-Zero invariants alone, lands on α·x₊ where α := 1/x₊
  emerges as the *ratio* of two B-eigenvalues. In this reading, x₊ is
  output, not input; α is a derived ratio, not a constant we insert.

- **Reading R2 (circular).** We compute B's eigenvalues using the
  engine's `G_C = √α` constant, where α is hardcoded as `1/X_PLUS_PRECISION`
  (engine/include/ftd/constants.h). The engine then "predicts" α-related
  spectra because we put α in.

**The Mechanism C closure is positive only if R1 is realizable.** If the
PROTOCOL_BCC_SUBLATTICE_SPECTRUM measurement passes only because the
engine has α hardcoded (R2), Mechanism C is informationally equivalent to
Mechanism B — same fate.

A clean test of R1 vs R2: **run the campaign with G_C set to a value
deliberately offset from √α** (e.g., G_C = 0.1, factor 1.17 off). If the
ratio λ₊/λ₋ remains at 45.31 (calibration-invariant), R1 is supported.
If the ratio changes, R2 is correct and Mechanism C closes negative.

This control run should be added to D2 §6 (confound checks) before
publication-grade execution.

## 8 · Resolution

Tag-history slot:

  2026-04-26: [OPEN → CONJECTURE]  (this document)
  YYYY-MM-DD: [CONJECTURE → SELECTION | CLOSED NEGATIVE]  pending D6

The §6 falsification criteria are pre-registered. They will not be modified
post-hoc. If the smoke test reveals systematic issues that require threshold
adjustments, those adjustments will be flagged in D6 with explicit
justification — they will not be silent.

## 9 · Pointers

Engine path (Cluster A build plan, complete 2026-04-26):
- `engine/include/ftd/sublattice.h` — Π_BCC, sublattice-projected Laplacians
- `engine/include/ftd/term_toggles.h` — bcc_stencil, langevin_site_filter
- `engine/include/ftd/correlations.h` — sublattice + diagonal correlators
- `engine/include/ftd/spectrum_extraction.h` — Prony, GEVP
- `engine/src/render_bridge.cpp` — phase_read dispatch, phase_write Langevin filter
- `engine/tests/campaign_bcc_band_spectrum.cpp` — measurement harness

Theory:
- FTD-0095, `FOUND_BRIDGE_FUNCTIONAL.md` — ontology side
- FTD-0096, `archive/closed_negative/OPEN_MU_FROM_LP_MISSING_ARROW.md` — calibration side
- FTD-0097, `PROTOCOL_LOOK_ELSEWHERE_SCAN.md` — author-isolation cross-check

Closure precedents (templates):
- `DERIV_MECHANISM_B_GC_DERIVATION.md` — closure structure
- `AUDIT_LINK8_CLOSURE.md` (FTD-0050) — orthogonality proof template
- `THEOREM_A_PHYS_NO_GO.md` (FTD-0059) — no-go theorem template
