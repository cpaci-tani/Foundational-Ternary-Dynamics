# ANALYSIS — The Minimum Viable Clock Carrier v1

**Status:** `[ANALYSIS — EXACT CONDITIONAL RESULTS WITHIN A DECLARED TWO-SCALE EXTENSION]`
+ `[EXPLORATORY NUMERICAL SCREEN — native single-scale arm, scoped]`
+ `[BOOKED — FTD-0804, FTD-0805]`
**Date:** 2026-08-07
**Parents:** `SPEC_CARRIER_CONSTRAINTS_v1.md` (C1–C12), FTD-0789 (the n = 4
criterion), FTD-0800/0801 (screen scopes; the two-scale/pre-tension doors),
FTD-0783 (bracket theorem), FTD-0784 (surd bound), FTD-0794 (G\* enters by
choice), FTD-0575 `[OPEN]` (compact-law derivation — the retirement path)
**Production impact:** none. No engine state, toggle, scenario, or golden is
touched. No tag moves.
**Artifacts:** `scripts/experiments/mvc_fourchain_clock.py` (exact + ringdown),
`scripts/experiments/native_unitedge_stress_screen.py` (native screen),
`scripts/experiments/native_chain_network_search.py` (integer-`K₄` search),
`scripts/experiments/native_chain_network_verify.py` (gates G1–G5),
`scripts/experiments/native_wheel_clock_sim.py` (the wheel's refuting ringdown)

---

## 1. Question and result

The carrier programme's wall is C3: no native mechanism produces the
intermediate-exponent (`n = 4`) confining potential in which `G*` is the
period constant. FTD-0789 left a decidable criterion — **`n = 4` requires
first-order flexibility with second-order rigidity** — and FTD-0800/0801
named the two live doors: a two-scale interaction or pre-tension, both
outside the registered zero-tension single-scale law.

This analysis does two things.

**Arm 1 (native).** It identifies the C3 criterion with the classical
second-order-rigidity theory of bar frameworks (Connelly's stress test): a
first-order flex is blocked at second order **iff a nonzero self-stress
blocks it**. Zero-tension equilibria of the registered single-scale compact
law are exactly the frameworks whose active bonds all sit at the single
minimum `r0 = 1` with opposite-polarity endpoints — **unit-edge bipartite
frameworks**. The native question therefore reduces to: *does any unit-edge
bipartite framework admit a nonzero self-stress?* A scoped numerical screen
over every connected min-degree-2 bipartite interaction graph up to `N = 7`
(plus the cube graph and exact SC checkerboard blocks) is run against that
question; results in §3.

**Arm 2 (the minimum viable carrier).** It constructs the cheapest object
that provably carries the quartic clock — the **collinear two-scale
4-chain** — and prices it. Within the declared extension the result is
exact: the chain is prestress-stable at zero tension, every transverse flex
has strictly quartic energy with closed-form coefficient, the mirror-even
mode closes as a 1-DOF pure quartic oscillator, and a full 12-DOF
conservative simulation recovers `G* = Γ(1/4)/Γ(3/4)` from ringdown timing
with **no fitted scale to 2×10⁻⁶ relative** (A→0 extrapolation).

Neither arm moves a tag. The MVC is not a native carrier — it fails C11 *by
construction*; that is what "minimum viable" means. FTD-0794's verdict
("`G*` enters FTD by choice") stands: the MVC is that choice made minimal,
explicit, and priced.

## 2. The C3 criterion is classical second-order rigidity

Let a configuration of point bodies interact through central pair
potentials, and let every active bond sit at a potential minimum (zero
tension, `V'(ℓ_e) = 0`, `k_e := V''(ℓ_e) > 0`). Let `R` be the rigidity
matrix (rows = stretch rates `û_e·(q_i − q_j)`). Along a displacement path
`p(d) = p₀ + d·q + d²·w`, the bond stretch is `d·(Rq)_e + d²·[κ_e(q) +
(Rw)_e] + O(d³)` with `κ_e(q) = |q_i − q_j|²_⊥ / (2ℓ_e)`. Three exhaustive
cases for a direction `q`:

- **`(Rq) ≠ 0`** — first-order rigid direction: energy `~ d²` (`n = 2`).
- **`Rq = 0` and `κ(q) ⟂ coker(R)`** — the flex extends through second
  order; it may be obstructed at higher order or extend to a finite
  mechanism, so the quartic term vanishes and `n ≥ 6` or `n = ∞`.
- **`Rq = 0` and `κ(q)` is not orthogonal to `coker(R)`** — the flex is
  **blocked**: minimizing over `w` gives exactly

  > `E(d) = λ(q)·d⁴ + O(d⁵)`,
  > `λ(q) = (1/2) min_w Σ_e k_e[κ_e(q)+(Rw)_e]² > 0`.

  This is the weighted projection of `κ` onto the full stress space. If
  `dim coker(R)=1`, it reduces to
  `λ=⟨ω̂,κ⟩²/(2 Σ_e ω̂_e²/k_e)`. **`n = 4` at zero tension exists iff a
  self-stress exists and blocks the flex.** The MVC has one-dimensional
  coker, so the reduced formula used below is valid there. This is FTD-0789's
  criterion in closed form, and it makes C3 *decidable by linear algebra*
  at any candidate configuration.

Two corollaries frame everything below. (i) *No self-stress ⇒ no `n = 4`*:
frameworks with independent bonds have all their flexes extend — the
FTD-0789 trimer and the FTD-0800 row-slides are instances. (ii) *Actual
tension ⇒ `n = 2`*: a nonzero equilibrium stress puts a `d²` term on any
flex it blocks. The quartic lives **only** at the degenerate point — zero
actual tension with nonzero possible stress.

## 3. Arm 1 — the native single-scale class (screen)

Zero-tension equilibria of the registered law = unit-edge bipartite
frameworks (all active bonds at `r0 = 1`, opposite polarity only; the
polarity rule makes the interaction graph bipartite, hence triangle-free).
Structural obstructions visible before any computation:

- a vertex with exactly two **non-collinear** bonds forces both stresses to
  zero (two independent unit vectors); so stress support needs collinear
  pass-throughs or degree ≥ 3;
- a straight chain of unit bonds carries stress only if closed end-to-end,
  and a closed collinear cycle of unit steps forces position coincidences
  (capacity-forbidden); the end-to-end closure bond of the open chain has
  length ≥ 3 — outside single-scale support;
- even cycles embedded with unit edges are rhombus-family mechanisms with
  degree-2 non-collinear vertices — stress-free.

**Screen** (`native_unitedge_stress_screen.py`, seed 20260807, floors:
same-polarity ≥ 0.40, opposite non-edge ≥ 1.30, 40 restarts/graph): every
connected min-degree-2 bipartite graph with `N ≤ 7`, plus the cube graph
`Q₃` (`N = 8`), searched for embeddings with all edges at exactly unit
length and a vanishing smallest eigenvalue of `RRᵀ` (= a self-stress).

> **RESULT (2026-08-07 run): 17 graph classes, 0 hits.** Constraint-clean
> minima are bounded away from zero — smallest clean
> `λ_min(RRᵀ) = 1.03×10⁻²` ((3,4), e = 10); the three near-zero rows
> (K(3,3) at `5.97×10⁻⁴`, dense (3,4) at `6.06×10⁻⁴`/`3.98×10⁻⁴`) carry
> constraint violations 100–1000× the clean rows — they are the known
> unit-realizability degeneracies (three unit spheres force coincident
> vertices, blocked by the separation floor), and the hit gate correctly
> rejects them. No unit-edge bipartite self-stress exists in the enumerated
> scope.

Exact complement, lattice-geometry class: the SC checkerboard blocks (every
opposite-parity unit pair bonded) at `L = 2, 3, 4` have **self-stress
dimension 0** — coker(R) exact rank check — so all their flexes extend:
`n = 2` or `n = ∞` only. This is FTD-0800's "clamped-only" phenomenology
explained: the observed SC `γ⁴` shear needed the affine clamp because the
clamp *supplies the stress the network itself cannot*.

### 3.1 The chain-network extension — a native stressed equilibrium found, and refuted as a clock

The screen's scope bound (`N ≤ 8`) is not the end of the native class.
Contracting stress supports (degree-2 stressed vertices force collinear
bonds) shows stressed objects are networks of **straight unit chains**
terminated at degree-≥3 joints; overlapping parallel chains force
same-parity coincidences, so the minimal contracted joint graphs are `K₄`
(stressed only when coplanar) and the wheels `W_k` (stressed generically in
the plane). Two exact results follow:

- **Interior-`K₄` is squeezed to a razor.** The spoke to an interior joint
  splits a corner angle and some corner is ≤ 60°, so the minimum joint
  angle of any interior-`K₄` is **≤ 30°**; the registered law's support
  (`q < 3/2`, i.e. `r < 1.2247`) demands ≥ 28.96° for the near-joint
  opposite-parity station pair. An exhaustive integer-distance search
  (`native_chain_network_search.py`; 153,898 integral quadruples to
  diameter 60, 7,177 distinct interior ones) tops out at 26.14°: **no
  viable interior-`K₄` at diameter ≤ 60**, and any survivor at larger
  diameter lives in the [28.96°, 30°) sliver.
- **The `s = 2` regular hexagon wheel (`N = 19`) passes the static
  gates.** All 24 bonds exactly unit, parity-native 2-coloring, nearest
  opposite-polarity non-bonded pair at `q = 3` — **twice** the support
  edge; coker(R) exactly 1-dimensional with the chain-uniform wheel stress
  (rim `+t`, spokes `−t`); every individual chain-bowing flex blocked with
  `E₄ = 1/48` (`native_chain_network_verify.py`, gates G1–G3 and
  per-chain G4). **This is, to this programme's knowledge, the first
  native zero-tension self-stressed equilibrium exhibited under the
  registered law** — FTD-0800's screens (`N ≤ 6`, SC blocks) could not
  have seen it.

**And it fails as a clock.** Ringdown under the actual registered
`V(q) = −16ε(q−3/2)²(q−3/4)` (`native_wheel_clock_sim.py`) found the
constrained static energy collapsing to ~10⁻¹² at `u ≥ 0.03`: the quartic
form is `E₄ ∝ (Σκ_rim − Σκ_spoke)²` — a **squared difference** — and the
combined flex with rim-bow `u` and spoke-buckle `δ = u` is a zero-energy
**mechanism cone**. The corrected full-flex gate (G5) confirms it
structurally: the blocking form on the 28-dimensional nontrivial flex
space is sign-indefinite (eigenvalues −1.467 … +0.743). Verdict:
`STATIC_STRESSED_EQUILIBRIUM_ONLY`. The physical root is general:

> **The buckling criterion.** Chain stations are free hinges, so a
> *compressed* chain of length ≥ 2 buckles at zero energy; only tensioned
> chains are straight-stable. A native `n = 4` clock therefore needs, on
> top of FTD-0789's criterion: **(1)** a self-stress, **(2)** a blocking
> form definite on the *whole* flex space, and **(3)** every compressed
> member a **single bond**. The wheel fails (3) (spokes are compressed
> length-2 chains) and hence (2). This is FTD-0787's failure family one
> level up: the verified per-chain quartic was a chord across a flat
> valley of the coupled flex space.

An all-tension self-stress is impossible on a finite framework (the
extreme-vertex balance), so condition (3) forces **mixed structures:
single-bond struts + straight tensioned chains** — integer "unit-strut
tensegrities". The first such family (the axial lens: strut `U–W` at span
1, isoceles tension cables `k, k` to an apex, apex strut to a counter-joint
with cables `m, m` back) is killed exactly: closure requires
`s² − 4k² = −1`, impossible mod 4. The native question is now:

> **[OPEN — native C3, third formulation]** does a finite integer
> unit-strut tensegrity exist under the registered law (struts = single
> unit bonds carrying compression; straight integer-span tension chains;
> polarity, floor, and `q < 3/2` clearances), with the blocking form
> definite on its full flex space?

No enumerated candidate realizes it; no impossibility proof exists either.

## 4. Arm 2 — the minimum viable carrier, exactly

**Definition (the two-scale 4-chain).** Bodies `A⁺ B⁻ C⁺ D⁻` collinear at
positions `0, 1, 2, 3`; species-1 bonds `AB, BC, CD` at their minimum
`r0 = 1` (curvature `k₁`); one species-2 bond `AD` at its minimum `3r0`
(curvature `k₂`). All four bonds at zero tension.

Exact results (`mvc_fourchain_clock.py`, sympy):

- `rank(R) = 3`; **self-stress space is 1-dimensional**, `ω = (1, 1, 1, −1)`
  in stretch-rate convention — tension capacity in the short bonds balanced
  by compression capacity in the long one.
- Nontrivial flex space: 4-dimensional (transverse profiles mod
  translation + rotation). The blocking form `ω(q,q) = Σ_e ω_e|Δq_e|²/ℓ_e`
  has eigenvalues `{2, 10/3, 0, 0}` with kernel exactly the trivial
  profiles — **positive definite on every nontrivial flex** (the
  Cauchy–Schwarz structure `|q_A − q_D|² ≤ 3·Σ(consecutive diffs)²` with
  equality only at rigid rotation). The chain is prestress-stable at zero
  tension: **`n = 4` in every flex direction.**
- Quartic coefficients (per unit transverse amplitude): mirror-even
  zero-momentum mode `q = (−1, +1, +1, −1)·U`:

  > `E(U) = λ_eff·U⁴`,  **`λ_eff = 8k₁k₂/(k₁ + 3k₂)`**,  `m_eff = 4`,

  and `E₄ > 0` likewise for the BC-symmetric, antisymmetric, end-pair, and
  single-end flexes (`k₁k₂/(2(k₁+3k₂))`, `9k₁k₂/(2(k₁+3k₂))`,
  `k₁k₂/(2(k₁+3k₂))`, `k₁k₂/(18(k₁+3k₂))`). Rigid-closure limit
  `k₂ → ∞`: `λ_eff → 8k₁/3`.
- **Closure (C8):** the mirror-even zero-momentum manifold is dynamically
  invariant (mirror symmetry × single transverse axis × zero momentum), so
  the mode obeys `Q̈ = −(4λ_eff/m_eff)·Q³` exactly — a 1-DOF pure quartic
  oscillator with period law

  > `T·A = √π · G* · √(m_eff/(2λ_eff)) = √π · G* · √(k₁+3k₂)/(2√(k₁k₂))`.

**The MVC passes the sharpened criterion the wheel fails.** Its compressed
member — the range-3 closure — is a **single bond** (a genuine strut,
resisting through its own `V″ > 0`, no internal hinge to buckle), and its
blocking form is positive-definite on the *entire* nontrivial flex space
(eigenvalues `{2, 10/3}`, kernel exactly the trivial motions). The 4-chain
is genuinely second-order rigid; the wheel was not. This is now a
load-bearing property of the design, not an accident.

**Simulation.** Full 12-DOF conservative dynamics (velocity-Verlet,
constrained-relaxed initial conditions, `k₁ = k₂ = 1`): `T·A` constant to
0.33% across a 6× amplitude range with the expected `O(A²)` finite-amplitude
drift, and the `A→0` extrapolation gives

> **`G*_exp = 2.95868` vs `G* = 2.958675119…` — 2×10⁻⁶ relative, no
> fitted scale.**

The two-scale 4-chain **is** a `G*` clock, in the exact sense the edge-clock
paper's period law demands.

**Minimality (finite enumeration, exact).** At `N ≤ 3`: opposite-polarity
graphs are trees (≤ 2 bonds) — no stress. At `N = 4` (necessarily 2+2): the
maximal graph is `K(2,2)`; single-scale embeddings of it have no stressed
realization (non-collinear ⇒ degree-2 kill; collinear ⇒ `B = D`
coincidence, capacity-forbidden). The unique stressed zero-tension
architecture on four bodies is the collinear chain with end-to-end closure,
and polarity alternation forces the closure span to be **odd**, hence
`= 3r0` at minimum. Equivalently: the MVC is `K(2,2)` embedded
degenerate-collinear, with the fourth edge realized by the second species.
**Four bodies, one new interaction species at range 3, is the floor.**

## 5. The price sheet

| Line | Content |
|---|---|
| **Adopted type** | one additional opposite-polarity compact interaction species: well at `3r0`, curvature `k₂`, support ⊂ (2, 4) (disjoint from the unit well) |
| **Currency** | 1 selected type + its scale parameters (`k₂`; and the C2 parity demand below) |
| **What it buys** | C3 exactly (first candidate ever to satisfy it); C1, C7, C8, C9, C10 ✓; C4 plausible (bounded 4-body object; field-coat closure unverified) |
| **What it does not buy** | C2/C5/C6 remain open and are the same energy-scale wall the spec already isolated, now quantified: with `k₁ = k₂ = ε`, band clearance `Ω(A) > ω_B` needs `ε·A_max² > 1.0556` against the field one-axis top (`2 arcsin(1/√3) = 1.230959`) and `> 2.786` against the acoustic/wave top (`2.000`) — band values per `DERIV_FLEXURAL_QUARTIC_MECHANISM_v1.md`. At `A_max = √(w/2)`, `w = 0.5`: `ε > 4.22` (field) / `11.15` (wave) — beyond every ε the framework has ever motivated (selected `0.01`; the refuted lattice-quantum candidate `0.516`). The honest reading: **the minimum viable carrier's true price is matter stiffness at field-stiffness parity**, purchasable only as an explicit scale adoption or a much wider declared well (field-band clearance at `ε = 0.516` needs `A_max ≈ 1.43`, i.e. `w ≈ 4.1` — a well as wide as the chain itself). `A_drain` (C5) is unevaluated — it needs the coupled campaign the spec flags. |
| **Falsifier** | after preregistration (C12): a seeded 4-chain's transverse mode shows the anti-pendulum signature `ω ∝ A` (slope-1 log f–log A) and `T·A·√(2λ_eff/m_eff)/√π = G*` to declared tolerance, in a profile where no imposed phenomenology does the work; failure of either kills the candidate |
| **Retirement path** | derive the range-3 well from the native field-mediated interaction (FTD-0575 `[OPEN]`): a secondary minimum at odd range in the dressed pair force retires the adoption to `[DERIVED]`; a proof of monotonicity makes the import permanent and priced |

## 6. Scope guards

- **No native carrier is claimed.** Arm 1's screen is negative in its
  scope; the single-scale no-go remains `[OPEN]` as a theorem target.
- **`G*` still enters by choice** (FTD-0794): the MVC realizes the quartic
  clock *given* the declared species; it derives nothing about why the
  substrate would provide one.
- **FTD-0784 stands:** a passing carrier delivers `G*`, never the FC-W surd
  `√(G*(4G*−1))`; W stays external.
- **No engine claim is made** — C12 requires a fresh preregistration before
  any engine campaign; this document contains simulation of the *candidate
  model*, not of the production engine.
- Bookkeeping: this analysis and both scripts are uncommitted work product;
  a LEDGER row (and any INDEX refresh) is the owner's booking decision.
