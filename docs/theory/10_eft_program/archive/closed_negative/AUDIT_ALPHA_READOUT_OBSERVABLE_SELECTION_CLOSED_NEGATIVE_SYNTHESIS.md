# AUDIT -- ARC-B1 alpha-readout closure attempts: **CLOSED-NEGATIVE** synthesis across catalog items 4, 6, 7

**Tag:** [CLOSED NEGATIVE -- ARC-B1 primary catalog items] -- per §6 outcome (c) of the locked pre-registration; verdict result of executing the §9 11-step method against each of the three primary FTD-native non-site-local observable classes (plaquette bivectors / boundary-to-boundary transfer observables / reference frame projections). **No FTD claim promoted or demoted.** FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`; spine untouched.
**LEDGER row:** FTD-0205.
**Date:** 2026-05-23 (Path V Sessions C3 + C4 + synthesis of the multi-session coordinated arc `.claude/plans/let-s-proceed-on-the-eager-rocket.md`; immediately follows the C1 plaquette-bivector closure attempt).
**Companion verdict (C1, already landed at commit `01d171d`):** [`AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md`](AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md) -- catalog item 4 (plaquette bivectors). FTD-0204.
**Pre-registration governing all three attempts:** [`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](../../preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) (FTD-0198), hash-locked at commit `0e79820`, tag `preregister-alpha-readout-observable-selection-v1`, SHA256 `e273ca85234c04406c14b0b0bb01bb2ea760367ca7286c2b35649b80563b582a`.
**Verdict scope:** the three **primary catalog items** of the §4 frozen catalog: item 4 (plaquette bivectors, closed in C1), item 6 (boundary-to-boundary transfer observables, closed here in §1.B), item 7 (reference frame projections, closed here in §1.C). Items 1-3 + 5 are construction-input primitives (state field; flux field + dual substrate; bilinear link observables; Wilson-loop-style closed flux-loop traces) used by all three primary candidates -- they are not standalone A_obs candidates because their use without one of items 4/6/7 reduces to site-local 0-form readouts (F-e firing) or trivial-degree scalar readouts. Items 8-9 are target-not-input. **The verdict closes ARC-B1 at the primary-catalog-item level**; catalog-item *variants* (finer subalgebra projections within each item, alternative Wilson-loop classes, alternative boundary geometries) remain open in principle but each would require a fresh closure attempt against the locked pre-reg.

---

## §0 -- Executive summary

The three primary FTD-native non-site-local observable classes of the FTD-0198 frozen catalog have now each been subjected to the §9 11-step method:

| Catalog item | Route | Verdict | Step failure | Reason |
|---|---|---|---|---|
| 4 -- plaquette bivectors | C1 | CLOSED-NEGATIVE (§6 (c)) | Step 5 structural-match | 𝔰𝔲(2)-type structure constants `κε_{abc}` do not produce `16(G\*)²`/`16(G\*)³` coefficients |
| 6 -- boundary-to-boundary transfer | C3 (this doc §1.B) | CLOSED-NEGATIVE (§6 (c)) | Step 5 structural-match | Lattice-Laplacian transfer-matrix spectrum is `{e^{−E_k τ}}` indexed by lattice momenta `k ∈ L³`; no projection to a 2-dim subspace produces `(16(G\*)², 16(G\*)³)` trace/determinant from FTD-native primitives |
| 7 -- reference frame projections | C4 (this doc §1.C) | CLOSED-NEGATIVE (§6 (c)) | Step 5 structural-match | Lindblad-style projection onto a public measurement channel yields characteristic equations whose coefficients are subalgebra dimensions and channel-rates -- finite-combinatorial quantities, not G\*-transcendentals |

**All three primary catalog items close negative at §9 step 5 by categorical structural mismatch**: each forward-derivable T_O has a characteristic equation whose coefficients are **finite-combinatorial or lattice-spectral quantities** (`κε_{abc}` structure constants; `{e^{−E_k τ}}` lattice eigenvalues; subalgebra dimensions + channel rates), while the master quadratic's coefficients `16 = |Aut(E)|²` (FTD-0006) and `G\* = Γ(1/4)/Γ(3/4)` (FTD-0002) are **number-theoretic invariants of the lemniscatic curve `E : y² = x³ − x`** (ℤ[i]-module / Chowla-Selberg / CM-curve arithmetic family). The structural mismatch is categorical at every primary catalog item -- not specific to bivectors.

**No falsifier fires** on any of the three constructions as derived. **No banned move was invoked** in any of the three attempts. The verdict is the substantive step-5 finding in each case, not a falsifier or banned-move firing.

**Verdict per §6 (c):** ARC-B1 primary catalog items all close negative. The §11 verdict document `AUDIT_*_CLOSED_NEGATIVE_SYNTHESIS.md` records the three-route closure.

**Load-bearing input for Path II FTD-0186 v2 Stage 2:** this verdict adds 2 more closed-negative dynamical-value derivation attempts (C3 + C4) to the closed-negative record, bringing the total to **14 α-derivation routes closed negative** (the 11 prior + C1 + C3 + C4). Per the v2 falsifier criterion A1, all 3 new routes classify as type-i closed-negative (failed dynamical-value derivation, target = α / `1/x_+`), consistent with v2 Outcome A. The boundary theorem program now has a **substantially strengthened empirical base** for its structural-decoupling thesis: the observable-selection mechanism class as a whole closes negative at the primary-catalog-item level.

**What this verdict does NOT close:** catalog-item variants (finer subalgebra projections; alternative Wilson-loop classes; alternative boundary geometries) within each item remain open in principle. ARC-A (boundary-condition), ARC-C (quantization rule), ARC-D (discrete-native measurement) remain unattempted -- each gets its own pre-registration if/when pursued. **No spine tag move.**

---

## §1 -- Per-route §9 11-step execution

The C1 verdict (`AUDIT_*_CLOSED_NEGATIVE.md`, FTD-0204) covers the plaquette-bivector route in full §9 detail. The C3 + C4 routes are executed below using the same template, more compactly where the framework is already established by C1.

### §1.A -- Plaquette bivectors (C1, catalog item 4)

**See companion document** [`AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md`](AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md) for the full §9 execution. **Verdict: CLOSED-NEGATIVE per §6 (c) at step 5** -- the 𝔰𝔲(2)-type bivector structure constants `κε_{abc}` (per FTD-0086 matching signature) do not produce the master quadratic's lemniscatic-curve coefficients `16(G\*)²` / `16(G\*)³`. The verdict's load-bearing finding is the **categorical structural mismatch** between cubic-lattice bivector commutator algebra and lemniscatic-curve / ℤ[i]-module / Chowla-Selberg arithmetic.

### §1.B -- Boundary-to-boundary transfer observables (C3, catalog item 6)

**Catalog item 6 verbatim from pre-reg §4:** *"Propagator-style or transition-amplitude functionals from one face of a finite L³ block to the opposite face; transfer-matrix eigenvalues of operators constructed from items 1-5."*

#### Step 1 -- ARC tuple

- **P (preparation):** a finite L³ block (e.g. L = 8 per the FTD-0107 G1 protocol baseline) with two designated boundary faces -- the **source face** S = {(x_1, x_2, 0) | 0 ≤ x_1, x_2 < L} and the **sink face** T = {(x_1, x_2, L−1)}. The bulk between S and T evolves under the canonical FTD non-local dynamics (the full toggle set per FTD-0086 §1.2 protocol) for `N_τ` ticks (the "block thickness in time," part of the pre-registered preparation).
- **A_obs:** the algebra `𝔄_T` generated by **boundary-trace observables** on the source face: integrated flux `Φ_α(x_1, x_2) := J_α(x_1, x_2, 0)` for `α ∈ {1, 2, 3}` (catalog item 2 evaluated on S), boundary-localized bilinears `J_α(x_1, x_2, 0) J_β(x_1', x_2', 0)` for distinct sites on S (catalog item 3 restricted to S), and boundary-localized Wilson-loop traces (catalog item 5 evaluated on closed loops in S). The corresponding sink-face algebra `𝔄_T'` is defined analogously on T. **Non-site-local by construction** -- each boundary observable has support across the entire face (an `L × L` 2D patch); F-e cannot fire.
- **O_EM (transfer observable):** the bulk-transfer kernel `K(o_T, o_S) := ⟨o_T | U_τ^{N_τ} | o_S⟩` where `U_τ` is the per-tick FTD evolution operator on the L³ block and `o_S ∈ 𝔄_T`, `o_T ∈ 𝔄_T'`. The transfer matrix is `T_{T,S} : 𝔄_T → 𝔄_T'`, a linear map on the source/sink boundary algebras. Charge-like interpretation: a source-face flux pulse `o_S` generates a sink-face response `K(o_T, o_S)` -- the bulk transmission of a "boundary EM source" to a "boundary EM detector."
- **R (readout map):** dimensionless transmission ratio. Specifically `R(o_S, o_T) := |K(o_T, o_S)|² / (⟨o_S, o_S⟩ ⟨o_T, o_T⟩)` -- a Rayleigh-quotient-style scalar between 0 and 1. The candidate readout: the largest such ratio over all `(o_S, o_T)` pairs is the dominant transmission eigenvalue of `T_{T,S}`, proposed as `1/x_+`.
- **C (calibration discipline):** the flux field `J` is dimensionless per `FOUND_FORCE_STRUCTURE.md`; bilinears + boundary integrals + transmission ratios are all dimensionless. No `a_phys` / `K_B` / `t_phys` calibration enters. F-h cannot fire.

#### Step 2 -- Derive A_obs from §4 catalog primitives

1. `J` per catalog item 2 (flux field).
2. Boundary restriction `J|_S` -- the flux field evaluated on the source face S, a 2D `L²`-dim vector. Derived from item 2 by spatial restriction (a finite linear functional, no new postulate).
3. Boundary bilinears `J|_S(x_1, x_2) · J|_S(x_1', x_2')` for distinct boundary sites -- catalog item 3 restricted to S.
4. Boundary Wilson loops: closed loops in S form a 2D lattice gauge theory's loop algebra; loop traces are gauge-invariant scalars per catalog item 5.
5. The algebra `𝔄_T` is closed under sum, scalar multiplication, and the boundary-bilinear product; it is a finite-dimensional commutative algebra (since J is a classical real-valued field, not a non-commuting operator algebra in the absence of quantization).

No step uses items 8 or 9 as input. The derivation is forward from items 1-5.

#### Step 3 -- Verify gauge / translation / O_h invariance

- **Translation invariance** within S (translations parallel to the boundary): manifest under PBC in the (x_1, x_2) directions.
- **O_h-restricted invariance:** the full O_h is broken by the choice of boundary direction (the source/sink direction breaks the 3-fold axial symmetry to C_4v). The boundary algebra `𝔄_T` carries the C_4v subgroup of O_h. This is the standard transfer-matrix setup and is consistent with §5 contract item 2 (which requires "cubic symmetry" -- the C_4v boundary subgroup is the natural cubic symmetry of a square-face boundary).
- **Gauge invariance:** trivially (no gauge redundancy in the pure-flux substrate).

#### Step 4 -- Construct T_O on A_obs

The natural transfer/readout operator on `𝔄_T` is the **bulk-transfer matrix** `T_{T,S}` defined in step 1, restricted to a finite basis of boundary observables.

**The lattice Laplacian transfer matrix.** The canonical FTD evolution `U_τ` is the discrete-time wave equation (Phase G), which on the bulk between S and T gives a unitary evolution whose spectrum on the boundary modes is `{e^{−E_k τ}}` for the bulk Laplacian eigenvalues `E_k`. For an L³ block, the lattice momenta `k ∈ (2π/L) {0, 1, ..., L−1}³` give the spectrum.

**Forward construction of T_O.** Take the boundary basis `{Φ_α(k_⊥) | k_⊥ ∈ (2π/L){0, ..., L−1}², α ∈ {1, 2, 3}}` -- the 2D momentum modes of the boundary flux. The transfer matrix in this basis is **block-diagonal in k_⊥** (translation invariance parallel to S), and within each k_⊥-block is a 3×3 matrix `T_{αβ}(k_⊥) = e^{−E(k_⊥) τ N_τ} P_{αβ}(k_⊥)` where `P_{αβ}` is the polarization-projector inherited from the bulk Laplacian's transverse + longitudinal decomposition (per the Gauss-constraint structure).

**Characteristic equation of T_O.** The full `T_{T,S}` has characteristic polynomial `∏_{k_⊥} det(λ I − T(k_⊥)) = ∏_{k_⊥} (λ − e^{−E(k_⊥) τ N_τ})^2 (λ − e^{−E_long(k_⊥) τ N_τ})` -- a polynomial of degree `3 L²` whose roots are the boundary-projected bulk eigenvalues. Any 2-dim projection (e.g. zero-momentum + slowest non-trivial mode) gives a 2×2 transfer matrix with eigenvalues `(1, e^{−E_1 τ N_τ})` and characteristic equation `(x − 1)(x − e^{−E_1 τ N_τ}) = x² − (1 + e^{−E_1 τ N_τ}) x + e^{−E_1 τ N_τ}`.

#### Step 5 -- Compare to master quadratic

The master quadratic is `x² − 16(G\*)² x + 16(G\*)³ = 0` -- the trace must be `16(G\*)² ≈ 140.05` and the determinant must be `16(G\*)³ ≈ 414.36`.

The 2-dim transfer-matrix characteristic equation derived above has trace `1 + e^{−E_1 τ N_τ}` (between 1 and 2) and determinant `e^{−E_1 τ N_τ}` (between 0 and 1). **Both are bounded by O(1)** -- the trace can never reach 140 and the determinant can never reach 414, *regardless of L, τ, N_τ, or boundary-basis choice*, because the underlying transfer eigenvalues are exponentials of negative eigenvalues (or unity for the zero-mode) and are bounded between 0 and 1.

The full degree-`3L²` transfer matrix has trace `Σ_k (2 e^{−E(k) τ N_τ} + e^{−E_long(k) τ N_τ})` -- bounded by `3L²` and decreasing in `τ N_τ`. The determinant is similarly bounded. The polynomial's coefficients are sums of products of bounded transfer eigenvalues -- no `G\*`-transcendentals appear because G\* is a Γ-function ratio of the lemniscatic curve, not a lattice-Laplacian eigenvalue or its function.

**Verdict at step 5 for C3:** the boundary-to-boundary transfer matrix's characteristic equation has **lattice-spectral coefficients** (`Σ` of `e^{−E_k τ}`-products), categorically distinct from the master quadratic's lemniscatic-curve coefficients (`16 = |Aut(E)|²`, `G\* = Γ(1/4)/Γ(3/4)`). **No forward derivation** from FTD-native lattice transfer-matrix structure produces the master quadratic. **CLOSED-NEGATIVE per §6 (c).**

#### Steps 6-7 (moot, partial firing recorded)

- **Step 6 (D4 dominant-branch):** moot. If step 5 had passed, dominance under the largest-magnitude transfer eigenvalue (the slowest-decaying mode) would be the natural rule; this is admissible D4 (iii). F-c not predicted to fire in this route.
- **Step 7 (D3 operational protocol):** moot. The transfer-matrix measurement is operationally well-defined -- a source-face perturbation, a sink-face readout, the bulk thickness `N_τ` controls the transfer time. D3 (a)-(d) all satisfiable. F-d not predicted to fire.

#### Steps 8-9 (mechanical, all clean)

All 10 falsifiers and all 11 banned moves applied as in C1; **no firing on construction as derived; no banned move invoked.** F-j risk is moderate (the transfer-matrix-style construction is closer to FQCR M_N(t) in form than the bivector route was, and a careless basis choice could resemble M_N(t)) -- but the construction here is forward from lattice primitives, the eigenvalues are bounded by 1 (not `16(G\*)²`), and no FQCR import occurred.

#### Steps 10-11 (skipped per step rule + verdict)

Step 10 skipped per §9 rule. Step 11 verdict: **CLOSED-NEGATIVE per §6 (c)** for the boundary-to-boundary transfer route. The structural mismatch is categorical: lattice-Laplacian transfer eigenvalues are exponentials of negative real numbers (bounded by 1); the master quadratic's roots are `137.04` and `3.02` (unbounded relative to lattice scales).

### §1.C -- Reference frame projections (C4, catalog item 7)

**Catalog item 7 verbatim from pre-reg §4:** *"Projections of the finite observable algebra onto a 'public' measurement channel, per `REF_REFERENCE_FRAME_VOCABULARY.md` and the math-first ontology of `SPEC_MATH_FIRST_ONTOLOGY.md`."*

*Note: the reference frame structure vocabulary doc was renamed during the 2026-05-22 corpus consolidation; the current reference is `docs/theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md` or its successor. The 2026-05-01 sweep retired "reference frame context" terminology in favour of "reference frame structure = structural; frame dynamics = dynamical." For this verdict, the relevant content is the projection-onto-public-channel construction, which is Lindblad-style in standard open-quantum-systems terminology.*

#### Step 1 -- ARC tuple

- **P:** a finite L³ block under canonical FTD non-local dynamics, with a designated **public observable channel** `Π : 𝔄_full → 𝔄_public` -- a projection from the full FTD observable algebra to a (typically much smaller) Lindblad-style "classical" subalgebra. The candidate `𝔄_public` is the abelian subalgebra generated by the zero-momentum flux mode `J(k=0) = Σ_x J(x)` (a 3-dim space, one per axis) together with the energy density `Σ_x |J(x)|²` (a 1-dim scalar). Total dimension: 4. This is the natural "macroscopic / coarse-grained" subalgebra of the lattice FTD substrate.
- **A_obs:** `Π(𝔄_full)` = the image of the full observable algebra under the public-channel projection. Generated by `{J_α(k=0), |J|²}` -- 4 generators. The algebra is commutative (since the zero-mode J components and `|J|²` all commute pairwise as classical c-numbers in the pure-flux substrate).
- **O_EM:** a charge-source measurement that reads the public channel -- a far-field flux measurement averaging over the whole substrate, returning `J_α(k=0) / |J|`. This is the FTD analog of a macroscopic dipole-moment measurement.
- **R:** dimensionless ratio of channel amplitudes. Candidate: `R(O_EM) := |J(k=0)|² / Σ_x |J(x)|²` -- the fraction of total flux energy in the zero-momentum mode (between 0 and 1).
- **C:** dimensionless throughout.

#### Step 2 -- Derive A_obs from catalog

1. `J` per item 2.
2. The zero-momentum mode `J(k=0) = Σ_x J(x)` -- a finite linear functional of J across the lattice. Non-site-local by construction (sums over all L³ sites).
3. The energy density `|J|² = Σ_α J_α²`. Pointwise local; the spatial sum `Σ_x |J(x)|²` is the total energy, non-site-local.
4. The public channel `𝔄_public = span{J_α(k=0), Σ_x |J(x)|²}` is the Lindblad-style coarse-grained subalgebra. Derived by spatial averaging from the full algebra -- a standard mathematical operation (reference frame projection per item 7).

No step uses items 8 or 9 as input.

#### Step 3 -- gauge / translation / O_h invariance

- Translation invariance: `J(k=0)` and `Σ_x |J|²` are both translation-invariant by construction.
- O_h invariance: `J(k=0)_α` transforms as the vector rep under O_h; `Σ_x |J|²` is a scalar (trivial rep). The 4-dim public algebra carries the rep decomposition `T_{1u} ⊕ A_{1g}` -- standard O_h irreps.
- Gauge invariance: trivial.

#### Step 4 -- Construct T_O on A_obs

The natural readout operator on the 4-dim `𝔄_public` is the **Lindblad super-operator restriction** `L_restr : 𝔄_public → 𝔄_public` -- the projection of the full FTD dynamics onto the public channel. For the linear-wave-equation Phase G sector, the zero-momentum mode is exactly conserved (no time evolution) and `Σ_x |J|²` is exactly conserved (Phase H scaling theorem). So `L_restr = I` (the identity on `𝔄_public`).

**The identity operator's characteristic polynomial is `(x − 1)^4 = 0`** -- one degenerate eigenvalue at `x = 1` with multiplicity 4.

**Beyond the linear regime:** with the full toggle set (Langevin, genesis, etc.), the public-channel restriction acquires dissipation. The zero-mode decays at the Langevin rate `γ_L`; the energy density flows to thermal modes at the equipartition rate. The characteristic polynomial of `L_restr` becomes `(x − e^{−γ_L τ})^3 (x − 1) = 0` -- three degenerate vector eigenvalues at the Langevin decay rate, one conserved energy eigenvalue at unity. Trace and determinant are both bounded by O(1) -- structurally distinct from `16(G\*)² ≈ 140` / `16(G\*)³ ≈ 414`.

**Alternative T_O constructions:**
- (a) The Casimir of the public algebra. Trivial scalar; characteristic equation `x − const = 0`.
- (b) The adjoint action of `J(k=0)` on the algebra. Vector-rep matrix; eigenvalues are simple multiples of `|J(k=0)|`. Not the master quadratic.
- (c) Any 2×2 projection of `L_restr` onto a 2-dim sub-channel. Trace and determinant inherit from the Lindblad rates `γ_L` and 1; bounded by O(1).

#### Step 5 -- Compare to master quadratic

For each T_O construction above, the characteristic equation has coefficients that are **Lindblad rates and subalgebra dimensions** -- finite-combinatorial quantities (the dimension count: 4, 3, 1) and dynamical rates (`γ_L`, 1). No `G\*`-transcendental appears: G\* is a Γ-function ratio of the lemniscatic curve, not a Lindblad rate or subalgebra dimension.

**Verdict at step 5 for C4:** the frame-relative-projection T_O's characteristic equation has **dissipation-rate + dimension coefficients**, categorically distinct from the master quadratic's lemniscatic-curve coefficients. **CLOSED-NEGATIVE per §6 (c).**

#### Steps 6-7 (moot, brief)

- **Step 6:** moot. Dominance under conserved-quantity selection (eigenvalue 1, not `e^{−γτ}`) is admissible D4. F-c not predicted to fire.
- **Step 7:** moot. Far-field flux measurement is operationally well-defined. F-d not predicted to fire.

#### Steps 8-9 (mechanical, all clean)

All falsifiers + banned moves applied; **no firing; no invocation.**

#### Steps 10-11

Step 10 skipped. Step 11 verdict: **CLOSED-NEGATIVE per §6 (c)** for the frame-relative-projection route. The structural mismatch is categorical: Lindblad-restriction eigenvalues are dissipation-decay exponentials or conservation-law identities (`1`, `e^{−γτ}`); the master quadratic's roots are number-theoretic `137.04` and `3.02`.

---

## §2 -- ARC-B1 primary-catalog-item synthesis

**The three primary FTD-native non-site-local observable classes have now each closed negative at §9 step 5 by categorical structural mismatch.** The mismatch is *not* mechanism-specific -- it is the same mismatch in each route: the FTD-native lattice-substrate observables produce characteristic equations whose coefficients are **lattice-combinatorial + dissipation-rate + structure-constant quantities** (`κε_{abc}`, `e^{−E_k τ}`, `γ_L`, dimensions like 3 and 4), while the master quadratic's coefficients are **number-theoretic invariants of the lemniscatic curve** (`16 = |Aut(E)|²`, `G\* = Γ(1/4)/Γ(3/4)`).

**What this generalises.** The mismatch is not about choosing the "wrong" subalgebra or the "wrong" 2-dim projection within each route. It is **at the level of what kind of mathematical object each side is**:

- The FTD-native lattice substrate has its own arithmetic: cubic-point-group structure constants, Moore-neighbourhood multiplicities, lattice Green's function values, Langevin dissipation rates, transfer-matrix lattice eigenvalues. These are **discrete-combinatorial / lattice-spectral / dynamical-rate** quantities.
- The master quadratic and its coefficients live in the arithmetic of the **lemniscatic elliptic curve** E : y² = x³ − x: CM-curve periods, Γ-function ratios, |Aut(E)|² automorphism counts, Z[i]-module structure. These are **number-theoretic / arithmetic-geometry / Chowla-Selberg** quantities.

**There is no known derivation chain** from the first arithmetic to the second. The G\*-bridge between the two (via the FQCR Model V transfer matrix M_N(t)) is the only explicit construction making them touch -- but M_N(t) is *defined* to have the master quadratic as its characteristic polynomial (per FQCR Prop 5); using it as scaffold to "derive" the master quadratic from observable structure is exactly the reverse-engineering pattern F-j prohibits.

**The ARC-B1 primary-catalog-item closure is therefore strong but bounded.** The closure does not prove that ARC-B1 is impossible -- it shows that the three primary catalog items, attempted by their natural forward derivations, all produce characteristic equations whose coefficient structure is categorically distinct from the master quadratic's. Catalog-item variants (finer subalgebras, alternative Wilson-loop classes, alternative boundary geometries) remain open in principle but face the same categorical obstruction unless they introduce some mechanism that bridges lattice-substrate arithmetic to lemniscatic-curve arithmetic *without* importing M_N(t) as scaffold.

---

## §3 -- Load-bearing input for Path II (FTD-0186 v2 Stage 2)

This synthesis closes ARC-B1 at the primary-catalog-item level and contributes **3 closed-negative dynamical-value derivation attempts** to the Path II FTD-0186 v2 corpus:

- **Pre-Session-C1 count:** 11 closed-negative α-derivation routes (per `SPEC_OPEN_MATH_BY_SECTOR.md` §2 closed-negative list).
- **Post-C1 count:** 12 (plaquette bivectors closed negative, FTD-0204).
- **Post-C3 count:** 13 (boundary-to-boundary transfer closed negative, this verdict).
- **Post-C4 count:** 14 (reference frame projections closed negative, this verdict).

All three new closed-negatives classify under v2's A1 as **type-i (failed dynamical-value derivation)**, with target = α / `1/x_+`. They are consistent with v2 Outcome A (Stage 1 CLOSED POSITIVE per v2, committed at `188c03e` in Session A2).

**For Stage 2 (the unsettled Structural Decoupling Theorem):** the ARC-B1 primary-catalog-item closure provides **strengthened empirical base** that the discrete substrate does not fix non-universal dynamical values through the observable-selection mechanism class. Stage 2 must still be pursued *as a genuine provable proposition with stated axioms* (per FTD-0186 v2 §1 honest framing); this verdict is necessary but not sufficient for Stage 2.

**Within the §5 alpha-readout-contract framework:** the surviving search space narrows from {ARC-A, ARC-B1, ARC-B2, ARC-C, ARC-D} to {ARC-A, ARC-B2-variants, ARC-C, ARC-D}, where ARC-B2-variants are the finer subalgebras within each of the three primary catalog items (still open in principle but facing the categorical obstruction documented here). Each remaining ARC class would require its own pre-registration if pursued.

---

## §4 -- What is closed; what is open

**CLOSED-NEGATIVE (this synthesis):**
- ARC-B1 primary catalog items 4 (plaquette bivectors -- C1, FTD-0204), 6 (boundary-to-boundary transfer -- this verdict §1.B), 7 (reference frame projections -- this verdict §1.C).
- The specific structural claim that any of the three primary FTD-native non-site-local observable classes produces a forward-derivable T_O whose characteristic equation is the master quadratic.

**NOT closed by this verdict:**
- ARC-B1 catalog-item variants (finer subalgebras within each item, alternative Wilson-loop classes, alternative boundary geometries). Each face the same categorical obstruction and would require a fresh closure attempt.
- ARC-A (boundary-condition), ARC-C (quantization rule), ARC-D (discrete-native measurement). Each gets its own pre-registration if/when pursued.
- The master quadratic itself (FTD-0001 [THEOREM]). Unchanged.
- FTD-0086 / 0087 / 0088 (the bivector campaign evidence for Cl(3,0) emergence in non-local dynamics). Unchanged -- this evidence stands; the verdict only forecloses the *alpha-readout* application.

**Spine status unchanged:**
- FTD-0001 (master quadratic): [THEOREM]. Untouched.
- FTD-0006 / 0007 (coefficient 16): [THEOREM]. Untouched.
- FTD-0013 (`x_+ = 1/α`): **[STRONGLY MOTIVATED CONJECTURE]. Tag unchanged.** The conjecture remains supported by the structural-uniqueness evidence of FTD-0189 (master quadratic is the unique dual-matcher across 2.65M polynomials) and the bridge identity `G\* = 2√π G_G`; the ARC-B1 closure-negative does not contribute to its support and does not detract from its support either.

---

## §5 -- Honest methodological notes

**Why all three primary catalog items closed negative at the same step.** The categorical structural mismatch is the same in each route: FTD-native lattice substrate arithmetic vs lemniscatic-curve arithmetic. The mismatch is not specific to bivectors / transfer / frame-relative; it is structural at the level of *which mathematical category each side lives in*. This is the load-bearing finding -- and it is consistent across three independent attempts using three categorically distinct observable classes.

**Why this is not a v1 falsifier firing.** No falsifier fires on any of the three constructions as derived. The verdict in each case is the substantive step-5 finding (characteristic-equation coefficient mismatch). Importantly, F-j (master quadratic inserted not derived) was the highest-risk falsifier in all three routes -- the constructions are forward from FTD-native primitives, and any "fix" that would force the master quadratic structure would require importing M_N(t) as scaffold (firing F-j). The discipline of writing CLOSED-NEGATIVE for each route instead of forcing the match is exactly what the pre-reg's §8 banned moves protect against.

**Why this is not a v2 pre-reg-required correction.** The FTD-0186 v1 → v2 cycle established that when a pre-reg's wording is found to be too broad or too narrow to capture the honest verdict, a v2 is required. **The FTD-0198 pre-reg's wording is correct for ARC-B1**: catalog items 4, 6, 7 are exactly the non-site-local FTD-native observable classes that ARC-B1 admits; the §9 method captures exactly what an admissible closure requires. The verdict (CLOSED-NEGATIVE for all three) is reachable within the v1 framework as designed; no v2 of FTD-0198 is required.

**What this verdict invites going forward.** If the user wishes to attempt closure within ARC-B1 by a variant catalog-item construction (e.g. a non-trivial subalgebra of bivectors that has not been tested, or a different boundary geometry, or a different frame-relative-projection channel), a fresh §9 execution against the FTD-0198 v1 pre-reg can be run. **The categorical structural mismatch documented in §2 is the load-bearing obstruction that any such attempt must address** -- bridging lattice-substrate arithmetic to lemniscatic-curve arithmetic *without* importing M_N(t). The FTD-0122 BCC complex-structure theorem (Z[i]-module structure on Z[BCC] ⊗ ℚ) is a possible algebraic bridge that has not been operationalised as an observable readout; whether such an operationalisation is possible without firing F-j is an open question for any future Session C-variant.

---

## §6 -- LEDGER + cross-references

LEDGER row FTD-0205 [CLOSED NEGATIVE -- ARC-B1 primary catalog items] records the synthesis verdict.

Cross-refs:
- FTD-0198 (the pre-reg, hash-locked at `0e79820`).
- FTD-0204 (C1 plaquette-bivector verdict, [CLOSED NEGATIVE], commit `01d171d`; reads this synthesis as its primary-catalog-item context).
- FTD-0086 / 0087 / 0088 (bivector campaign, unchanged).
- FTD-0001 / 0002 / 0006 / 0007 / 0013 (derivation targets, all tags unchanged).
- FTD-0186 v2 (boundary theorem Stage 1 CLOSED POSITIVE per v2, commit `188c03e`; the v2 corpus gains 3 new type-i closed-negatives from this synthesis).
- FTD-0073 (site-local Clifford no-go, broken by non-site-locality in all three routes).
- FTD-0050 / 0094 / 0097 / 0116 / 0035 / 0093 / 0031 (prior closed-negative routes that F-g checks against; this verdict is genuinely distinct from each).
- FTD-0122 (BCC complex-structure theorem, ℤ[i]-module on Z[BCC] ⊗ ℚ; the candidate algebraic bridge between lattice-substrate and lemniscatic-curve arithmetic that has not been operationalised as an observable readout).
- ARC-A / ARC-B2-variants / ARC-C / ARC-D (the surviving search space, each gets its own pre-reg if/when pursued).
- `.claude/plans/let-s-proceed-on-the-eager-rocket.md` Sessions C1 (commit `01d171d`) + C3-C4 (this verdict).

---

*End of synthesis verdict. ARC-B1 primary catalog items all CLOSED-NEGATIVE per §6 (c). Variants within ARC-B1 and the ARC-A / ARC-C / ARC-D alternatives remain open. No FTD claim promoted or demoted; spine untouched.*
