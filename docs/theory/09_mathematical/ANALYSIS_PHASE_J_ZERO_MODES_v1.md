# ANALYSIS — Phase-J Ultralocality Beyond L=2: Zero Modes, the Correct Measure, and the Verdict

**ID:** FTD-0350
**Date:** 2026-07-01
**Tag:** [THEOREM] — matched-stencil ultralocality at **every** L ≥ 2 on the
Gauss-realizable state space, conditional on (i) the exact-constraint limit
λ_G → ∞ (the spec's own [AXIOM], SPEC_FTD_LAGRANGIAN.md §3.3) and (ii) the
stencil-consistency [SELECTION] (FTD-0090 matched-stencil discipline), which
fixes the *domain* of realizable configurations but never the ultralocality
on it. The prior L ≥ 4 ambiguity is **closed as a proven masking artifact**,
not a structural failure.
**Script:** `scripts/proofs/proof_phase_j_zero_modes.py` (44/44 checks pass)
**Supersedes (on the L ≥ 4 point):** the AMBIGUOUS/OPEN reading in
`scripts/proofs/proof_phase_j_general_L.py` and spine §7's
"[AMBIGUOUS / OPEN for L ≥ 4]" clause.
**Reconciliation note (2026-07-22):** `SPEC_ALGEBRAIC_SPINE.md` §7 now
incorporates this all-`L` scope and removes the contradictory pre-FTD-0350
`L≥4 [OPEN]` prose. The LEDGER tag/count and trackers are unchanged.

---

## 1 · Question and prior status

Spine Theorem 7 (SPEC_ALGEBRAIC_SPINE.md §7) held, as of 2026-07-01:
ultralocality — the classical FTD partition function's Euclidean action S_E
depending on s ∈ {−1,0,+1}^{L³} only through Σᵢ sᵢ² — is [THEOREM at L=2]
(all centered-difference symbols vanish), holds numerically at L=3, and is
OPEN/ambiguous at L ≥ 4 because the lattice acquires zero modes (e.g.
k = (0,0,π) at L=4) lying in the Gauss-excluded kernel; the naive scan in
`proof_phase_j_general_L.py` showed placement-dependent S_E (~3–28% spread)
that was *plausibly* a setup/masking artifact.

**This document settles the question.** The spread is a masking artifact,
proven exactly (§6.1); ultralocality is a theorem at every L on the correctly
characterized configuration space (§5), with the effective quadratic form
equal to the identity on the physical subspace (§5, Test 4).

## 2 · Setup and notation

Torus T_L = (ℤ/L)³, N = L³ sites. State field s : T_L → {−1,0,+1}; flux
J : T_L → ℝ³. A **first-difference stencil** is a triple D = (D₁,D₂,D₃) of
translation-invariant operators with Fourier symbols dᵢ(k), k ∈ (2π/L)·ℤ³.
Two instances matter here:

- **centered** (the FTD-0090 matched analysis stencil, used by
  `proof_phase_j_general_L.py`): (Dᵢf)(x) = ½(f(x+eᵢ) − f(x−eᵢ)),
  symbol dᵢ(k) = i·sin kᵢ;
- **forward**: (Dᵢf)(x) = f(x+eᵢ) − f(x), symbol dᵢ(k) = e^{ikᵢ} − 1. Its
  normal operator Σᵢ Dᵢᵀ Dᵢ is the engine's 7-point (G6) Laplacian, symbol
  2Σᵢ(1 − cos kᵢ) — i.e. this is the stencil adjoint-consistent with the
  engine's Gauss-projection Poisson operator.

The stencil is used **consistently**: the divergence is (div J) = Σᵢ DᵢJᵢ and
the kinetic norm is K[J] = Σ_x Σ_{i,j} (DᵢJⱼ)(x)². The static-sector
Euclidean action (SPEC_FTD_LAGRANGIAN.md §3.3, as instantiated in
`DERIV_PARTITION_FUNCTION_L2.md` §2.1) is

```
  S_E[J, s] = (c²/2)·K[J] + g_c·Σ_x s·(div J) + λ_G·Σ_x (div J − s)²,
```

with λ_G → ∞ declared **[AXIOM]** by the spec ("primary constraint … We work
in the constrained theory throughout", SPEC_FTD_LAGRANGIAN.md §3.3). Write
d(k) = (d₁,d₂,d₃)(k), |d(k)|² = Σᵢ|dᵢ(k)|², and

```
  Ker(D)      = { k : d(k) = 0 }                          (the zero modes)
  S_phys(D)   = { s : ŝ(k) = 0  for all k ∈ Ker(D) }      (realizable states)
```

DFT convention: ŝ(k) = Σ_x s(x)e^{−ik·x}; Parseval Σ_x s² = (1/N)Σ_k |ŝ|².

## 3 · The Gauss-excluded kernel, characterized exactly [THEOREM]

**Theorem A (kernel).** For the centered stencil, sin(2πn/L) = 0 ⟺
n ∈ {0, L/2}, so

- **L odd:** Ker = {0} — the constant mode only;
- **L even:** Ker = {0,π}³ — **8 modes**. At L=4 these are exactly
  k ∈ {(0,0,0), (π,0,0), (0,π,0), (0,0,π), (π,π,0), (π,0,π), (0,π,π),
  (π,π,π)} (indices {0,2}³). At L=2 this is *all* modes (the known
  degeneracy).

For the forward stencil, |e^{ik}−1|² = 2(1−cos k) = 0 ⟺ k = 0, so
**Ker = {0} at every L** — no Nyquist zero modes exist at all.
*(Script Test 1: symbol count and real-space dim ker(Σ DᵢᵀDᵢ) agree at
L ∈ {2,…,6} for both stencils.)*

**Theorem A′ (real-space meaning, even L, centered).** The modes {0,π}³ are
the characters of (ℤ/2)³ acting on coordinate parities, so ŝ|_Ker = 0 ⟺ the
**eight parity-class sublattice charges** Q_σ = Σ_{x ≡ σ (mod 2)} s(x) all
vanish, σ ∈ {0,1}³. Quantitatively, (1/N)Σ_{k∈Ker}|ŝ(k)|² = (8/N)Σ_σ Q_σ².
*(Test 2.)* Intuition: the centered difference hops by ±1 per axis, so on an
even torus its conservation structure refines from one global charge to one
charge per parity class; on an odd torus the wrap-around identifies the
classes and only the global charge survives.

**Theorem B (solvability / what "Gauss-excluded" means).** At a kernel mode
k*, (div J)^(k*) = Σᵢ dᵢ(k*)Ĵᵢ(k*) = 0 for *every* J, so the constraint
div J = s reads 0 = ŝ(k*) there. Hence

```
  div J = s is solvable   ⟺   ŝ|_Ker = 0   (i.e. s ∈ S_phys),
  and exactly:  min_J ‖div J − s‖² = (1/N)·Σ_{k∈Ker} |ŝ(k)|².
```

*(Test 2: machine precision on random neutral configs at L ∈ {3,4,6};
worked example — the nearest-neighbour dipole at L=4 centered has
residual² = 1/4 exactly: a NN dipole puts its two charges in different
parity classes and is therefore NOT Gauss-realizable under the centered
stencil.)* Under the λ_G → ∞ [AXIOM], configurations outside S_phys carry
infinite action (§4.1): they are not in the support of the constrained
partition function. "Gauss-excluded" is thus literal — the projection cannot
act on those channels, and the constraint cannot be satisfied on them.

## 4 · The correct measure/quotient

Three measure-adjacent choices arise when integrating out J. Their statuses
differ, and only one is a live selection:

### 4.1 · Exact constraint (λ_G → ∞): the spec's own [AXIOM], not a new choice

SPEC_FTD_LAGRANGIAN.md §3.3 declares λ_G → ∞ as the primary constraint
([AXIOM] in its parameter table) and works in the constrained theory
throughout; the engine's `gauss_project` enforces div J = s exactly (to
solver tolerance) every tick. Under it, the J-integral at fixed s is
supported on {J : div J = s}, which is empty for s ∉ S_phys — those states
get zero weight. **No new selection is introduced by restricting to
S_phys; it is forced by solvability given the spec's axiom.**

For completeness the soft-constraint alternative is computed exactly (§6.2):
at finite λ_G the only non-ultralocal term is λ_G × (the constraint-violation
norm itself), and on S_phys ultralocality holds identically **for every
λ_G > 0** — the theorem is not an artifact of the limit.

### 4.2 · Flat directions of the J-integral: provably immaterial

At each kernel mode, all three components of Ĵ(k*) appear in **no** term of
the action (kinetic weight |d(k*)|² = 0, divergence contribution 0). These
flat directions (3 per kernel mode; also the k=0 uniform-flux background)
make ∫DJ divergent. Any regularization — quotient by the flat subspace,
compactification, or an ε-mass — multiplies Z by a factor that is
**s-independent**, because the flat subspace is one fixed linear subspace,
the same for every s. Likewise the Gaussian fluctuation determinant on the
non-flat directions is s-independent (s enters the action linearly in J).
**Hence the s-dependence of the effective action is invariant under every
translation-invariant treatment of the flat directions: no measure choice
here can alter the theorem.** This is itself checkable (and checked): the
KKT minimizer is non-unique exactly along these directions, but the minimum
*value* is unique (Test 3's KKT residuals ~1e-14).

### 4.3 · The stencil: the one genuine [SELECTION]

Which difference stencil realizes ∇_L is a discretization choice. The
theorem below holds for **every** consistent choice (same D in the
divergence and the kinetic norm — the FTD-0090 matched-stencil discipline).
What the choice moves is only the **domain**: Ker(D), hence S_phys(D).
Centered ⇒ even-L domain is the 8-fold sublattice-neutral subspace;
forward ⇒ domain is all charge-neutral configs at every L, with no
exclusions. The selection never touches whether ultralocality holds on the
domain. [SELECTION, domain-only.]

## 5 · Main theorem and proof

**Theorem C (matched-stencil ultralocality, all L).** Let D be any
translation-invariant first-difference stencil used consistently, L ≥ 2, and
s ∈ S_phys(D). Then

```
  min { K[J]  :  div J = s }  =  Σ_x s(x)²,
```

exactly, and therefore, in the constrained theory,

```
  S_E[s] = (c²/2 + g_c) · Σ_x s(x)²  =  (c²/2 + g_c) · N_manifested.
```

S_E depends on s only through the manifestation count, independent of
spatial placement, **at every lattice size**. Equivalently: the effective
quadratic form in s obtained by integrating out J is (c²/2 + g_c) × Identity
on the physical (kernel-quotiented) subspace.

**Proof.** Work per Fourier mode; the action is diagonal in k because D is
translation-invariant.

*Non-kernel modes (d(k) ≠ 0).* The kinetic norm at mode k is
Σ_{ij}|dᵢ(k)Ĵⱼ(k)|² = (Σᵢ|dᵢ|²)(Σⱼ|Ĵⱼ|²) = |d(k)|²·|Ĵ(k)|², and the
constraint reads Σᵢ dᵢ(k)Ĵᵢ(k) = ŝ(k). By Cauchy–Schwarz,
|ŝ(k)|² ≤ |d(k)|²·|Ĵ(k)|², with equality iff Ĵ(k) ∥ conj(d(k)); the
minimizer Ĵ(k) = conj(d(k))·ŝ(k)/|d(k)|² is admissible. Hence the minimal
kinetic contribution of mode k is

```
  |d(k)|² · |ŝ(k)|²/|d(k)|²  =  |ŝ(k)|²   —   the |d(k)|² cancels
                                              identically, mode by mode.
```

This cancellation is the structural heart of the result: the kinetic weight
|d(k)|² is exactly the reciprocal of the constraint's "leverage" at that
mode, for *any* symbol d — which is why the result is stencil-independent on
the realizable domain and why no continuum limit or Parseval heuristic is
needed.

*Kernel modes (d(k) = 0).* The constraint there reads 0 = ŝ(k), satisfied by
hypothesis (s ∈ S_phys); the J-components at those modes carry zero kinetic
weight (§4.2) and contribute 0.

*Sum.* min K = (1/N)·Σ_{k∉Ker}|ŝ(k)|² = (1/N)·Σ_k|ŝ(k)|² = Σ_x s(x)²,
using ŝ|_Ker = 0 and Parseval. The coupling term is g_c·Σ_x s·(div J) =
g_c·Σ_x s² on the constraint surface, and the penalty term vanishes. ∎

**Numerical verification (independent of the Fourier argument).** The script
computes min{JᵀGJ : BJ = s} by generic real-space KKT least squares — no
Fourier, no masking, no structure assumed — and confirms:

- L=4 centered, three hand-built realizable witnesses with equal Σs² = 2 and
  maximally different placements (same-class separation 2; same-class body
  diagonal 2√3; the opposite parity class): K_min = 2.000000000000 each,
  S_E = 7/3 each, spread 2.3e-14 (Test 3) — the configurations that would
  exhibit a structural failure, if one existed, provably do not;
- L=4/6 centered random realizable configs and L=5 centered (odd; all
  neutral configs realizable): K_min = Σs² to ≤1e-13 (Test 3);
- the form itself is the **identity** on the 56-dimensional physical
  subspace at L=4: Q(v) = ‖v‖² for random subspace vectors *and* the
  polarization identity Q(u+v) − Q(u) − Q(v) = 2⟨u,v⟩ holds to 3e-13
  (Test 4) — proportional-to-identity is verified as a form, not merely on
  a diagonal sample.

## 6 · Corollaries

### 6.1 · The old 3–28% spread, exactly accounted [THEOREM + verified]

The masked-Parseval quantity computed by `proof_phase_j_general_L.py`
(kernel modes silently dropped from the sum) satisfies the identity

```
  K_masked[s]  =  Σ_x s(x)²  −  (1/N)·Σ_{k ∈ Ker, k ≠ 0} |ŝ(k)|²
```

for **every** config, realizable or not. On the same seed-42 random neutral
configs as the old scan, the identity holds to ≤4e-15 at L ∈ {3,4,6,8},
reproducing the old relative spreads (0.0%, 27.6%, 8.3%, 3.0%) and proving
they are **exactly the kernel content of Gauss-unrealizable
configurations** — states for which no flux field satisfies div J = s and
which the constrained partition function assigns zero weight (Test 5). The
spread was never the action of anything: the masked formula computed a
pseudo-inverse value that silently discarded the unsatisfiable constraint
components. **Verdict on the old ambiguity: masking artifact, proven — not a
structural failure.**

### 6.2 · Finite λ_G, exactly [THEOREM + verified]

Integrating out J at finite λ_G (soft constraint) gives, exactly,

```
  S_min[s] = A(λ_G)·(Σs² − κ) + λ_G·κ,       κ = (1/N)·Σ_{k∈Ker}|ŝ(k)|²,
  A(λ_G)   = λ_G − (2λ_G − g_c)² / (4(c²/2 + λ_G))  →  c²/2 + g_c.
```

On S_phys (κ = 0): S_min = A(λ_G)·Σs² — **ultralocal for every λ_G > 0**,
not only in the limit. Off S_phys the sole extra term is λ_G·κ, i.e. λ_G ×
the constraint-violation norm of §3 — placement-dependent only through the
violation itself, and divergent as λ_G → ∞, recovering the exclusion of
§4.1. (Test 6: exact match at λ_G ∈ {1, 10, 1000} for realizable and
non-realizable states; A(∞) = 7/6 at g_c = 1, c² = 1/3.)

### 6.3 · Stencil instances and the L=2 provenance repair

- **Forward stencil (engine-Laplacian-consistent): no exclusions at any L.**
  Ker = {0}, so S_phys = all charge-neutral configs; ultralocality holds for
  every neutral state at every L. In particular all **1107** neutral L=2
  configs verify K_min = Σs² by explicit KKT computation, and the unit
  dipole gives S_E = 7/3 = 2.3333 — the historical value of
  `DERIV_PARTITION_FUNCTION_L2.md` §4.1, now computed honestly on the
  lattice (Test 7). This closes a provenance gap: the original
  `partition_function_L2.py` *asserted* the kinetic value Σs² from the
  continuum Parseval identity (`gradient_squared_energy` returns it by
  fiat), and the DERIV doc's claim of numerical verification was not backed
  by that script. The claim was true; the verification now exists.
- **Centered stencil at L=2 is degenerate, and its "theorem" was vacuous as
  a constrained statement:** D ≡ 0 (all 8 modes are kernel modes), so
  div J = s is unsolvable for every s ≠ 0 and the kinetic term is
  identically zero (Test 7). The historical L=2 result and the
  `proof_phase_j_general_L.py` investigation therefore used **different
  stencils** — forward-consistent vs centered — which is precisely why the
  L=2 lore and the general-L scan appeared to be in tension. Theorem C
  subsumes both under one uniform statement.
- **Mismatched pairing (G6 Laplacian + centered gradient — the engine's
  historical FTD-0090 configuration) is genuinely non-ultralocal at L ≥ 4**
  (mode factor sin²k/[2(1−cos k)] ranges over [0, 1/2] at L=4) **but
  accidentally ultralocal at L = 3** (single frequency shell: the factor is
  constant = 1/4 on all nonzero modes), Test 8. This is a real property of
  the mismatch, consistent with the known ~1% Ward residual; it does not
  touch the matched action.

### 6.4 · The g_c informational obstruction generalizes to all L

Spine §7's "Consequence" paragraph states the obstruction (classical
extremization of S_E cannot fix g_c, since the action sees only Σs²) is
"L=2-specific; at L ≥ 3 the action does depend on spatial placement". Under
Theorem C that sentence is **stale**: on the realizable space the action
depends on no spatial structure at any L, so the informational obstruction
holds at **every** lattice size. This *strengthens* the closed-negative
FTD-0031 (no first-principles g_c from classical extremization — now
unconditionally in L) and removes the residual hope that larger lattices
reopen a variational route. It promotes nothing: g_c remains [PARAMETRIC],
and x₊ = 1/α remains [STRONGLY MOTIVATED CONJECTURE], untouched by anything
in this document.

## 7 · Verdict

From the pre-registered menu {THEOREM at L≥4 under the corrected measure /
STRUCTURAL FAILURE / REFINED-OPEN}:

> **THEOREM at L ≥ 4 — indeed at every L ≥ 2 — under the corrected
> treatment.** On the Gauss-realizable configuration space S_phys(D)
> (which the zero modes define exactly: ŝ = 0 on Ker(D); equivalently, for
> the centered stencil at even L, all eight parity-sublattice charges
> vanish), with the constraint enforced exactly per the spec's λ_G → ∞
> [AXIOM], the classical Euclidean action is S_E = (c²/2 + g_c)·Σs²,
> placement-independent, and the effective quadratic form is the identity
> on the physical subspace. The proof is the per-mode Cauchy–Schwarz
> cancellation of §5; the script verifies every component by exact
> real-space linear algebra at L ∈ {2,3,4,5,6} (44/44 checks). The prior
> L ≥ 4 ambiguity is closed: the 3–28% spread is exactly the kernel content
> of unrealizable configurations (§6.1) — a masking artifact, proven.

**Conditionality, stated exactly (per §4):** the theorem is
[THEOREM conditional on (i) λ_G → ∞ — already the spec's [AXIOM], hence not
a new selection, and softened by §6.2's every-λ_G result on S_phys; and
(ii) the stencil-consistency [SELECTION], which is the FTD-0090
matched-stencil discipline and determines only the realizable domain]. The
flat-direction measure (§4.2) is provably incapable of affecting the
result, and no other measure freedom exists in the Gaussian sector.

## 8 · Downstream items (controller decisions; NOT applied here)

1. **Spine §7 status line** — eligible for upgrade from
   "[THEOREM at L=2] + [NUMERICAL EVIDENCE at L=3] + [AMBIGUOUS/OPEN L≥4]"
   to "[THEOREM at all L on the Gauss-realizable space, conditional on the
   stencil-consistency selection]", citing FTD-0350. Whether Theorem 7 then
   moves from the honestly-tiered bucket into the theorem-grade count is a
   spine-convention decision, not made here.
2. **Spine §7 "Consequence" paragraph** — the sentence claiming the g_c
   informational obstruction is L=2-specific is stale (§6.4) and should be
   corrected to all-L (a strengthened negative).
3. **`DERIV_PARTITION_FUNCTION_L2.md` §3** — its "verified numerically"
   claim should point at Test 7 of the new script (the original script
   asserts the value analytically), and its implicit stencil (forward /
   G6-consistent) should be stated.
4. **`proof_phase_j_general_L.py`** — its route-(b) closure text remains
   accurate as history; its "AMBIGUOUS at L≥4" conclusion is superseded by
   this analysis.

## 9 · Reproducibility

```
python scripts/proofs/proof_phase_j_zero_modes.py     # 44/44 checks
```

Numerics: exact dense linear algebra (numpy only) — kernel dimensions by
eigendecomposition, solvability by least-squares residual, constrained
minima by generic KKT least squares (independent of the Fourier proof),
form-identity by diagonal + polarization sampling on the physical subspace,
artifact accounting on the identical seed-42 configs as the superseded scan,
and the finite-λ_G closed form at three penalty strengths. No Monte Carlo,
no state-space enumeration beyond the deliberate full L=2 sweep (1107
configs). Benchmark constants g_c = 1, c² = 1/3 match
`partition_function_L2.py`; Theorem C is independent of both values.
