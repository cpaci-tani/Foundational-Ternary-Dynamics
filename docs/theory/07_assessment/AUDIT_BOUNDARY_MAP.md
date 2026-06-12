# AUDIT — The FTD Boundary Map

**Tag:** `[SYNTHESIS]` — cross-document integration of existing boundary claims at their
canonical tags. **Introduces no new theorem; promotes no tag; derives nothing new.**
Every row points at a canonical source; where a source and this map disagree on a tag,
the canonical source (LEDGER / TRACKER_ONTIC_TRUTH / SPEC_ALGEBRAIC_SPINE) wins.

**Created:** 2026-06-09
**Scope:** the *second clause* of the Number-One Goal — "...and rigorously establish what
we **cannot** [derive]." This document is the single-page map of what the discrete ternary
ontology provably does **not** determine. Closed-negatives and no-gos are deliverables here,
not failures: they map how far discreteness reaches.

**Companion canonical sources:**
- `core_ledgers/LEDGER.md` — per-claim provenance (source of truth for tags)
- `core_ledgers/TRACKER_OPEN_ITEMS.md` — every live `[OPEN]`
- `01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` — sector research queue + foundational-obstruction framing
- `01_reference/MONOGRAPH_FTD_CONSTRUCTION.md` Part II — the route-invariant α boundary in narrative form
- `audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` (FTD-0242)
- `audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md` (FTD-0243)
- `02_foundations/FOUND_SPACETIME_FORCING_BOUNDARY.md` (FTD-0253)

---

## 0 · The organizing thesis — three closure mechanisms + one engine category

Almost every boundary below is an instance of one of three recurring mechanisms:

1. **The "6th-postulate" family.** The substrate is **commutative**, **finite**, and (in its
   genuinely finite sector) **irreversible**. Three different physics targets each need a
   structure *provably absent* from postulates P1–P5, which would have to be injected as an
   independent axiom:
   - **non-commutativity** → quantum core (QM),
   - **reversibility** → the Lorentzian metric,
   - **the Pythagorean L² budget** → the clock hypothesis / γ.

   These are the same shape of result three times over.

2. **The assembly-vs-ingredients gap** (the central α boundary). The lattice forces every
   *scalar ingredient* of α's defining quadratic but **not their assembly into one operator**.
   α is therefore **dynamical, not structural**.

3. **Parametric / numerical disproof.** A specific framework-integer reading or candidate
   identity is computed and falsified (gravity's "1/100"; x₋ ↔ SM; the η-quotient routes; …).

A fourth, softer category is the **engine-blocked boundary**: a result true in principle but
not yet expressible in the current simulator without breaking the bit-exact golden gate
(cluster rigid-translation).

---

## 1 · The central boundary — α is dynamical, not structural

> **One-liner (FTD-0242 §5):** *"The substrate forces every ingredient of α's defining
> quadratic — the even trace 16G\*² from the Watson integral, and the existence of a clean odd
> G\* from the J-twisted ζ-determinant — but it does not force their assembly into one readout
> operator, so α's value is selected by a logically independent convention, not derived; the
> discrete ontology determines the menu, not the dish."*

**What is forced** (all `[THEOREM]`):
- the **even trace** `16G*² = 16·|μ₄|²·G_BCC(0)` — 16 from ℤ[i] automorphisms; G*² from the
  Watson BCC self-energy;
- the **existence of a clean odd source**: the J-twisted ζ-regularized determinant ratio
  `det_ζ(D_{3/4}) / det_ζ(D_{1/4}) = Γ(¼)/Γ(¾) = G*` — degree 1, with √(2π) cancelling so there
  is **no √π prefactor**; this *lifts* the bare-parity no-go FTD-0233 (FTD-0234).

**What is NOT forced:**
- That these two assemble into **one** readout operator with `(Tr, Det) = (16G*², 16G*³)`. For a
  2×2 operator, trace and determinant are **independent invariants** — fixing one leaves the
  other free. The factorization `Det = Tr·G*` is a *role assignment*, the imposed
  master-quadratic Vieta target (**W-CRIT-2**), `[UNDERDETERMINED]` (FTD-0235).
- Attacked from **four FTD-native routes** (jtwist, bcc, cm, novel): **0/4 force the assembly**
  (FTD-0242, `[STRONGLY MOTIVATED CONJECTURE no-go]`). A sympathetic red-team's attempt at a
  *5th* route also failed (0/5), *strengthening* the boundary.

**The route-invariance theorem (FTD-0243, RSI Leg 3):**
- **Flip ruled out** `[THEOREM]` — the only geometric alternative (the D6 ⟨111⟩ three-plane
  det_ζ product) is excluded because a definite complex structure needs `mult_O(E)=0`, which
  breaks O_h to a single C₄ axis, while D6 needs C₃∈Stab; `⟨C₄,C₃⟩=O` → no readout.
- **Leg 3b closes its own scope** `[THEOREM]`.
- **Reduction is route-invariant** `[THEOREM]` — **Q(G\*) is the Galois-fixed field** of the
  master quadratic's ℤ/2 (disc = 64G*³(4G*−1)). *Every* forward-forced symmetric FTD datum lives
  in Q(G*) and is therefore **provably blind to which root is 1/α**. Witness: the family
  det = 16G*²·G*^k for k = 0,1,2,3 gives dominant roots 139.05 / **137.04** / 130.68 / 105.76 —
  all F-consistent; *nothing in F selects k = 1*.

**The irreducible kernel — K-BIND** `[OPEN]`:
The one thing that would fix α is for the substrate to **natively realize √(G\*(4G\*−1))** — the
squarefree generator of the unique quadratic extension Q(G\*)(√disc)/Q(G\*) in which x₊ and x₋
first become distinguishable. The monograph calls this **"the discrete limit of a square root."**
It is a *universal negative over substrate-native operators* and is **not even well-posed over
current 𝔉**, because 𝔉 contains no finite, closed generating system for admissible operators on
V_complex = ℤ[i]² from lattice data. Closing it requires *axiomatizing* a "substrate-native
operator construction calculus."

**Sub-boundary — x₋ has no physical correspondent** (FTD-0210, `[CLOSED NEGATIVE]`): the smaller
root x₋ ≈ 3.024 was tested against 25 pre-specified SM observables — all falsified (closest
39.7% off). x₋ is a pure chirality/coordinate artifact of the quadratic; the old `x₋ ↔ N_c`
identification is **retired** (FTD-0014 removed, commit `ca7eb61`). N_c = 3 is independently
sourced (Moore Layer Theorem + four topological routes, `DERIV_NC_FROM_TOPOLOGY.md`).

**Exits (shared by the whole cluster):**
1. a substrate-native binding law **W** logically independent of P1–P5 (both `𝔉∪{W}` and
   `𝔉∪{¬W}` have explicit consistent models — W is genuinely a 6th-postulate-class input); or
2. a fresh engine-native **ARC-D** measurement — but **ARC-D1 already returned `[CLOSED NEGATIVE]`**
   (0 macroscopic cluster fissions across 2000 seeds; the lattice is topologically rigid).

| # | What is bounded | Mechanism | Tag | Ledger |
|---|---|---|---|---|
| 1 | Operator assembly `(Tr,Det)=(16G*²,16G*³)` not forced by any of 4 routes | trace & det independent invariants | `[SMC no-go]` | **FTD-0242** |
| 2 | Conditional independence theorem (RSI Leg 3) | Q(G*) Galois-fixed → blind to which root is 1/α | `[THEOREM]` (conditional) | **FTD-0243** |
| 3 | **K-BIND** — native √(G\*(4G\*−1)) | "discrete limit of a square root"; not well-posed over 𝔉 | `[OPEN]` | FTD-0243 §6 |
| 4 | x₋ ≈ 3.024 has no SM correspondent | all 25 observables falsified; chirality artifact | `[CLOSED NEGATIVE]` | **FTD-0210** |
| 5 | Clean odd source *does* exist | J-twisted det_ζ = G*, √(2π) cancels | `[THEOREM]` | FTD-0234 |
| 6 | `Det = Tr·G*` factorization unforced | assembly-level independence (W-CRIT-2) | `[UNDERDETERMINED]` | FTD-0235 |

**Central obstruction:** all of the above sit under **MC-T4.3** — the operational α-readout
mechanism — a `[FOUNDATIONAL OBSTRUCTION]` open 3+ months. `x₊ = 1/α` (FTD-0013) stays
`[STRONGLY MOTIVATED CONJECTURE]`.

---

## 2 · The 6th-postulate family — three structures the lattice lacks

Structurally the *same boundary* in three sectors: a target physics needs a structure the
commutative / finite / irreversible substrate provably does not contain.

### 2a · Quantum non-commutativity — re-derived from four angles (all `[CLOSED NEGATIVE]`)
The substrate's observable algebra is **commutative** (classical fields s, J, v):
- **Modular time** (FTD-0225): commutative → abelian → type-I → trivial modular flow → no
  canonical objective time.
- **Complementarity** (FTD-0226): the manifestation map is a deterministic function of
  *commuting* flux → distributive/Boolean → a joint distribution exists → classical
  coarse-graining, not quantum complementarity. Structurally *explains* the 6-neighbour Born-rule
  closed-negatives (FTD-0197/0199/0200).
- **N_c=3 → ℤ/3 budget symmetry** (FTD-0228): flagged **apophenia** — `{J_x,J_y,J_z}` commute
  (co-measurable), but ℤ/3 would permute three *complementary* bases; same count (3), wrong *kind*.
- **The native angle** (FTD-0251): the substrate's *only* native dynamical angle is the symplectic
  **quadrature clock** `arg(q+ip)`, winding at ω(k) (`{q,p}≠0`, real physics) but strictly
  **commutative** (`[q,p]=0`). It carries **zero G\* content**, so it closes the
  "symplectic-phase → readout-rotation" route *before it is walked* — disjoint from the
  α-readout parameter set.

**Verdict:** quantum-core non-commutativity is an injected 6th-postulate-class input M.

### 2b · The Lorentzian metric & reversibility (FTD-0253, `[SYNTHESIS]+[BOUNDARY]`)
- **The causal cone is FORCED** `[THEOREM]` — c = 1/√3 from P4 (locality, ≤1 voxel/tick).
- **The Lorentzian metric is POSITED** `[AXIOM]` — it rides on the *second-order, reversible*
  Born-Infeld action `(Δ_t J)²`. **P5 is determinism, not reversibility** (deterministic
  diffusion is metric-free, γ-free). Reversibility is the missing forcing principle, a
  6th-postulate-class input *the same shape as QM's non-commutativity*.
- **Structural signature:** FTD's own **π/G\* split is the reversible/irreversible split** — the
  Euler reflection *product* branch `Γ(z)Γ(1−z)→π` (reversible, spacetime metric) vs the *ratio*
  branch `Γ(z)/Γ(1−z)=G*` (irreversible, arrow of time). **Finiteness opposes reversibility**:
  FTD's genuinely finite sector (the ternary state field) is irreversible.
- **Engine demo** (`test_spacetime_forcing_demo`, 9/9): same lattice → causal cone bit-identical
  (both fronts 7.211 @ t=8), but the metric (clock / reversibility / ballistic ruler r_rms∝t ×4.68
  vs diffusive ×2.46) appears **only in 2nd order**.

### 2c · The clock hypothesis / Pythagorean budget (FTD-0208, `[CLOSED NEGATIVE]`)
Three iterations (v1 UNDERDETERMINED → v2 INVALIDATED → v3 CLOSED-NEGATIVE, adversarial review
9.8/10): the quadratic budget `(dτ/dt)² + v² = 1` is **structurally incompatible with Scale-0
primitives** — the ternary state space has no native L²-norm; the natural Scale-0 addition law is
the **L¹ linear** ceiling `v + dτ/dt ≤ 1`. The Pythagorean/isotropic structure is
macroscopic-emergent (Scale 5+). So the clock hypothesis is an independent **`[AXIOM]`** at the
coordinate level; `SPEC_FTD_LAGRANGIAN.md` §4.3 is `[THEOREM conditional on clock-hypothesis AXIOM]`.

**Related positive-but-scoped result — dynamical time dilation (FTD-0252):** a wave-clock built
from the *massless* lattice (never reading the circular `voxel.tau`). v1 → **OTHER** (`√(1−v²)` is
an *identity of the construction*, not a contest). v2 → IR-scoped `[MEASURED]`: on the ⟨100⟩ axis
the departure from γ vanishes as **`R ∝ L⁻²` (∝k²)** — γ emerges in the IR — but
ultra-relativistic diagonals don't converge at L≤193 `[OPEN]`. FTD-0208 *clarified, not refuted*
(different observable).

---

## 3 · Gravity

- **Framework-integer `G_N = 1/(b₃+N_c)² = 1/100` is FALSIFIED** (FTD-0131, `[CLOSED NEGATIVE]`):
  off by ~10²⁰–10⁴³ under any natural calibration; "the 1/100 coincidence has no substrate
  justification." The native substrate derivation gives instead the **gravitational fine-structure
  ratio** `α_G(e,e) = (m_e/m_P)² ≈ 1.745×10⁻⁴⁵` (0.38% match), `[DERIVED]` modulo the
  clock-hypothesis axiom and inheriting FTD-0015's `[STRONGLY MOTIVATED CONJECTURE]` floor. The
  Schwarzschild form is recoverable; the remaining open piece narrowed to **one** interpretive
  step (the clock hypothesis).
- **Scalar / exponential-metric routes closed:** the Yilmaz exponential metric (FTD-0184) is
  horizon-free and diverges from GR at O(v³); a native scalar-vector gravity model (FTD-0213) is
  excluded by the **Hulse-Taylor binary pulsar at 152.88σ** (33% error in dP/dt). The
  effective-metric Deser-bootstrap scaffold is retained.
- **`a_phys` from gravity** (Mechanism γ, FTD-0035) closed-negative → `a_phys ≡ ℓ_P` adopted as
  calibration.
- **Emergent spin-2 / graviton provenance** (Frontier 4): the massless spin-2 field h_μν is
  *posited, not derived* (graviton-provenance audit; Conjecture 10.1, Gap 10.1); the empirical
  substrate floor at L∈{32,64} shows 11/12 k-points TT-correlator identical to the spin-1 control;
  the spin-2 boundary theorem (free-theory) is pre-registered. Whether the substrate carries an
  emergent spin-2 mode is `[OPEN]`. *(Ledger-ID note: this cluster is entangled with the
  FTD-0189/0193 collision — see `AUDIT_LEDGER_ID_COLLISIONS.md`.)*

---

## 4 · The engine↔algebra mass bridges — the dynamical twins

`FTD-0110` and `FTD-0250` are the **same open gap** at two levels: the unclosed
**collective-coordinate reduction**.

- **FTD-0110 (cluster size ↔ mass).** Linear-level `k = 1/4` is `[DERIVED]` from O_h A₁g
  multiplicity; Bridge-I global O_h-equivariance is `[DERIVED]`. But **local A₁g purity is
  empirically FALSIFIED under the full pipeline** — the *non-local Gauss projection* drives f_A1g
  from 1.0 → 0.15 by ~tick 100 (GPU exact-FFT = CPU SOR bit-exactly, so structural, not
  convergence). The v2 "Orbit-Equipartition + Timescale-Separation" rescue is `[DISPUTED]` (4
  defects incl. a load-bearing arithmetic error ∑(A²/4)=A²). Empirically `N(A)≈k·A²` holds to
  **~5%** with a logarithmic drift `k≈¼(1−0.030·ln(A/2))`. Net: linear level `[DERIVED]`;
  nonlinear / multi-scale / SM-mass identification `[STRONGLY MOTIVATED CONJECTURE]`.
- **FTD-0250 (transport inertia, the dynamical twin).** The engine now *honors* the action's EP
  `[THEOREM]` — a locked N-voxel cluster carries inertial mass N·M_REST and the **velocity-EP is
  DEMONSTRATED** (unequal-N clusters free-fall bit-identically while F∝N). But this ships
  **`[IMPOSED]`**: proving that rigid translation of a Gauss-dressed N-voxel cluster *costs*
  co-moving momentum N·M_REST·v (perturbation theory in the collective coordinate) is `[OPEN]`.
  The **rigid-lattice translation visual is `[BOUNDARY — blocked]`** — the current movement phase
  same-sign-bounces a packed cluster, has an x-major visitation race, and resolves GPU collisions
  via nondeterministic atomicCAS (breaking the bit-exact golden gate); a new cluster-aware
  translation phase is needed.

Closing the reduction upgrades **both** FTD-0110 and FTD-0250 to `[DERIVED]`/`[THEOREM]`.

---

## 5 · The closed-negative catalog (~51 sealed routes, by theme)

Preserved for provenance — to prevent zombie re-emergence of exhausted routes.

| Theme | IDs | One-line reason for closure |
|---|---|---|
| **α-derivation routes** | 0031 (g_c, all 3 BCC routes), 0050 (RG char-poly; stencil ⊥ BCC), 0073 (Clifford/Dirac mode-erasure), 0094 (L2 identity → parametric), 0097 (look-elsewhere), 0116 (Z-factor), 0164 (3 χ₋₄→P_G* routes), 0183 (N_base=4 not ℤ[i]), 0204/0205 (ARC-B1 categorical mismatch), 0212 (Beilinson PSLQ null) | structural orthogonality or high-precision numerical disproof |
| **Operator-forcing / quantization** | 0197/0199/0200 (Born in 6-nbr), 0210 (x₋↔SM), 0233 (parity in ℚ[G*²]), 0235 (Tr/Det independent), **0242/0243 (route-invariant)** | trace/det independence + Galois-blindness → assembly unforced |
| **QM non-commutativity** | 0225 (modular flow), 0226 (distributivity), 0228 (ℤ/3 apophenia) | commutative substrate → no quantum core |
| **Gravity / relativity** | 0035 (a_phys γ), 0131 (1/100), 0184 (Yilmaz), 0208 (clock hyp.), 0213 (scalar-vector vs pulsar) | parametric falsification / Pythagorean incompatibility |
| **BCC / unification no-gos** | 0058 (two-U(1) Ward), 0079 ((SC+FCC)/2↔BCC exact identity) | finite-L stencil mismatch ~3% |
| **Strong / confinement** | 0025 (3 substrate routes) | confinement lives in `Z=∫dU e^{−S}`; deterministic substrate has no Phase-G analog — possibly outside current ontology |
| **Engine phenomenology** | 0060, 0061, 0062, 0063, 0071/0072/0074, 0135 | reduce to tautology / null GPU measurement / blocked on MC-T4.3 |
| **Mass / flavor** | 0219 (µ-unit loophole) | retracted for post-hoc fitting |

**Root-cause grouping:** structural obstruction (0025, 0050, 0073, 0225, 0226, 0228, 0243) ·
numerical disproof (0116, 0164, 0197–0200, 0212) · route-invariant boundary (0242/0243, MC-T4.3) ·
epistemic/retraction (0219, 0208 v1–v2) · pre-registered adverse outcome (0197, 0199, 0200, 0204,
0205, 0210, 0212).

---

## 6 · Open boundaries still genuinely researchable

- **MC-T4.3** — central `[FOUNDATIONAL OBSTRUCTION]` (3+ months). Surviving exits: a 6th-postulate
  W that forces the operator assembly, or a fresh ARC-D engine measurement (ARC-A/B
  closed-negative; ARC-D1 closed-negative).
- **FTD-0110-NL / FTD-0250 reduction** — the collective-coordinate proof (cleanest remaining
  `[OPEN]→[THEOREM]` mass/gravity conversion). Pre-registration required before attempt.
- **K-BIND** — not closeable until "substrate-native operator construction" is finitely axiomatized.
- **Spin-2 / emergent graviton** (Frontier 4) — whether the substrate carries an emergent spin-2
  mode `[OPEN]` (pre-reg `spin2-boundary-theorem-v1` landed).
- **Confinement** — flagged with a recognized structural obstruction (RP–FO effort).
- **FTD-0252 diagonals** — ultra-relativistic γ convergence needs L ≳ 257.
- Sector queue (`SPEC_OPEN_MATH_BY_SECTOR.md`): §8-running β-coefficients, §11-confine, §10 flavor
  depth matrices, W-CRIT-3 (Lorentz invariance), W-CRIT-4 (Bell S>2), the BH 11-item cluster.

---

## 7 · The unifying picture

FTD's boundaries reveal a **scale-stratified, structurally honest architecture**. The five
discrete postulates force the **causal skeleton** — the cone, c = 1/√3, the *menu* of α's
ingredients, the linear-level mass coefficient. But the three structures that turn that skeleton
into known physics — **non-commutativity** (QM), **reversibility** (the metric), and the **L²
budget** (γ) — are each provably absent from P1–P5 and must be injected as independent,
mutually-analogous 6th-postulate-class inputs. And the one quantity that would *derive* α,
**√(G\*(4G\*−1))** — "the discrete limit of a square root" — the lattice does not natively produce.

That is the map drawn honestly in **both** directions, exactly as the Number-One Goal's second
clause demands: the spine *derives*, and these boundaries *mark precisely how far discreteness
reaches.*

---

*This is a `[SYNTHESIS]` document. It restates canonical claims at their canonical tags and
introduces no new result. If any tag here disagrees with `core_ledgers/LEDGER.md`, the LEDGER is
authoritative.*
