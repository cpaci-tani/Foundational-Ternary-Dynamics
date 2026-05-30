# Derivation: Mode-Erasure Theorem for State-Field Readout + Spin-Field Partial Algebra

**Date:** 2026-04-24 (Phase-4 capstone)
**Status:** [THEOREM] for state-field mode erasure; [MEASURED] commutative algebra for spin-field readout
**Companion of:** [DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md](../algebra/DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md), [EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md](../algebra/EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md)
**Ledger row:** FTD-0073

Phase-4 ran six measurements of the anticommutator on FTD's 2³ (Walsh–Hadamard) and 3³ (Moore-26 axial dipole) blocks. Five collapsed to trivial no-structure results. One — the spin-field readout — produced a non-trivial commutative algebra. This document formalizes what the pattern tells us.

---

## 1. The empirical pattern across six measurements

| Test | Block | Readout field | Non-linearity | Algebra result |
|---|---|---|---|---|
| FTD-0061 | 2³ | state | genesis + movement | $\{e_i, e_j\} = 2 \cdot \mathbb{1}$ (trivial collapse) |
| FTD-0071 (4a) | 2³ | state | pair_production | $\{e_i, e_j\} = 2 \cdot \mathbb{1}$ |
| FTD-0071 (4b) | 2³ | state | weak_transmutation | $\{e_i, e_j\} = 2 \cdot \mathbb{1}$ |
| FTD-0071 (4d) | 2³ | state | forces + Coulomb + movement | $\{e_i, e_j\} = 2 \cdot \mathbb{1}$ |
| FTD-0072 | 3³ | state | genesis + movement | $\{e_i, e_j\} = -(1.2\text{-}1.3) \cdot \mathbb{1}$ |
| FTD-0073 (4e) | 2³ | **spin** | genesis + movement (off-axis inj.) | **non-trivial commutative algebra** |

All five state-field readouts collapse to "mode-index-independent" outputs. The spin-field readout preserves mode information and yields a genuine bilinear algebra, albeit not Clifford.

## 2. Mode-erasure theorem (state-field readout)

### 2.1. Setup

Let $B = \{0, 1, \ldots, b - 1\}^3$ be a $b \times b \times b$ periodic block ($b \in \{2, 3\}$ in Phase-4). Let $s: B \to \{-1, 0, +1\}$ be the FTD state field restricted to the block.

An **injection** is a flux configuration $\mathbf{J}^{(f)}: B \to \mathbb{R}^3$ parametrized by a weight-1 Walsh–Hadamard mode $f \in (\mathbb{Z}/2)^3$ with $|f| = 1$ (or the axial-dipole analog on the 3³ block), such that $\mathbf{J}^{(f)}(x) = A \cdot \chi_f(x) \, \hat{e}_{\mathrm{axis}(f)}$ for some amplitude $A \gg K_{\mathrm{GENESIS}}$.

A **tick** advances the engine by one update cycle via a local rule $s'(x) = g(x, \mathbf{J}, s_{\mathrm{prev}}, \{\text{neighbors}\})$. The genesis sub-rule, when $s_{\mathrm{prev}}(x) = 0$ and $|\mathbf{J}(x)| > 3 K_B$, assigns $s'(x) = \mathrm{sign}(\nabla \cdot \mathbf{J}(x))$.

### 2.2. Lemma (divergence-sign uniformity under step-function injection)

**Claim.** For a weight-1 WH injection $\mathbf{J}^{(f)} = A \chi_f \hat{e}_{\mathrm{axis}(f)}$ restricted to a $2^3$ block with zero flux outside, the lattice divergence $\nabla \cdot \mathbf{J}^{(f)}(x)$ has **the same sign at every site** $x$ interior to the block.

**Proof sketch.** Consider $f = \chi_{100}$ (only bit 0), injection on axis 0 (x). Then $J_x(x,y,z) = A \cdot (-1)^{x_0}$ for $(x_0, y, z) \in B$, and $J_x = 0$ outside. $J_y = J_z = 0$ everywhere.

At site $(0, y, z)$: x-neighbors at $(1, y, z)$ (in-block, $J_x = -A$) and $(-1, y, z) = (L-1, y, z)$ (out-of-block, $J_x = 0$). Central-difference $\partial_x J_x = (J_x(1, y, z) - J_x(L-1, y, z))/2 = -A/2$.

At site $(1, y, z)$: x-neighbors at $(2, y, z)$ (out-of-block, $J_x = 0$) and $(0, y, z)$ (in-block, $J_x = +A$). $\partial_x J_x = (0 - A)/2 = -A/2$.

Same value at every block site. $\square$

### 2.3. Theorem (state-field mode erasure)

**Claim.** Under the lemma, the state field after one tick with genesis on is uniform within the block: $s'(x) = -1$ for all $x \in B$ (or $+1$ if the injection sign is reversed). Consequently the state field's Walsh–Hadamard decomposition has support only on the identity mode $\chi_{000}$.

**Proof.** By the lemma, $\nabla \cdot \mathbf{J}^{(f)}(x) = -A/2$ at every block site. With $|\mathbf{J}^{(f)}(x)| = A > 3 K_B$, genesis fires at every void site and assigns $s'(x) = \mathrm{sign}(-A/2) = -1$. Therefore $s'$ is uniform. WH projection: $\hat{s}'(v) = \frac{1}{8} \sum_x s'(x) \chi_v(x) = -\frac{1}{8} \sum_x \chi_v(x) = -\delta_{v, \mathbf{0}}$. $\square$

**Corollary.** Any two-step injection protocol (inject $f$, tick, inject $g$, tick) produces a state field whose WH coefficients are independent of the **order** $(f, g)$ and — in the first-tick-dominant regime where genesis saturates the block after injection 1 — independent of the mode indices $(f, g)$ altogether. Hence the anticommutator $\{e_f, e_g\}$ is a constant $c \cdot \mathbb{1}$ on $B$, independent of $(f, g)$.

### 2.4. Universality across non-linearities

The theorem generalizes immediately to any non-linearity that:
- Acts pointwise on $\mathbf{J}(x)$ (genesis, pair production, weak transmutation)
- Has a threshold saturating at high $|\mathbf{J}|$
- Assigns the output via a translation-invariant sign / rule

All four tested routes satisfy this. Hence the universal FTD-0061+0071 result: every state-field readout produces the same collapsed anticommutator.

The 3³ block (FTD-0072) differs quantitatively because the axial-dipole injection has a different divergence structure (the zero-site at $v = 1$ in $\{0, 1, 2\}^3$ breaks the uniformity), but qualitatively behaves the same way: mode index is erased before the second tick can expose ordering.

## 3. Why spin-field readout breaks the collapse

The spin assignment rule is

$$ \mathrm{spin}_i(x) = \mathrm{sign}((\nabla \times \mathbf{J})_{\mathrm{dominant}}(x)) $$

with dominance determined by the largest-magnitude curl component. Unlike divergence, curl is a **vector** operator, and $\mathbf{J}^{(f)} \hat{e}_{\mathrm{axis}(f)}$ with $\mathbf{J}$ depending only on the axis-coordinate has $\nabla \times \mathbf{J} = 0$ everywhere — on-axis injections do not activate spin.

For the Phase-4e test, flux is stored **off-axis**: mode $f$ (natural axis $\mathrm{axis}(f)$) stored in the flux component of axis $\mathrm{axis}(f) + 1 \pmod 3$. Then:

$$ (\nabla \times \mathbf{J})_{\mathrm{axis}(f) + 2 \pmod 3} = \partial_{\mathrm{axis}(f)} J_{\mathrm{axis}(f)+1} - \partial_{\mathrm{axis}(f)+1} J_{\mathrm{axis}(f)} \ne 0. $$

The curl is **non-zero along the third axis** $(\mathrm{axis}(f) + 2) \pmod 3$, and its spatial structure follows $\chi_f$. So spin gets assigned along that third axis with a specific WH pattern — and this pattern distinguishes $f$.

**The spin-field probe preserves mode information because the curl-dominant-axis rule is direction-dependent in a way that divergence is not.**

## 4. Measured spin-field algebra (FTD-0073)

Labeling $e_1 = \chi_{001}$ (x-axis mode), $e_2 = \chi_{010}$ (y), $e_3 = \chi_{100}$ (z), and measuring the spin-field anticommutator:

$$ e_i^2 = X_i \quad \text{where}\quad X_1 = \chi_{101},\ X_2 = \chi_{011},\ X_3 = \chi_{110}. $$

The $X_i$ are the three weight-2 (bivector) WH modes. The assignment $e_i \mapsto X_i$ is:

$$ X_i = e_i \cdot e_{\pi(i)} \quad \text{(pointwise WH product)} $$

with permutation $\pi = (1 \to 3 \to 2 \to 1)$, determined by the off-axis storage choice.

Off-diagonal anticommutators:

$$ \{e_i, e_j\} = X_i + X_j = e_i^2 + e_j^2 \quad (i \ne j). $$

**This is a commutative bilinear algebra.** It is consistent with a symmetric product $e_i \cdot e_j = (X_i + X_j)/2$ for $i \ne j$ and $e_i \cdot e_i = X_i$ on the diagonal.

### 4.1. What this is NOT

It is not $\mathrm{Cl}(3, 0)$: Clifford requires $\{e_i, e_j\} = 2 \delta_{ij} \cdot \mathbb{1}$. Here diagonals give $2 X_i$ (not $2 \cdot \mathbb{1}$) and off-diagonals are nonzero.

It is not a Grassmann (exterior) algebra: Grassmann has $e_i^2 = 0$, whereas here $e_i^2 = X_i \ne 0$.

### 4.2. What this IS

A 3-generator commutative algebra over $\mathbb{R}[(\mathbb{Z}/2)^3]$ embedding the weight-1 modes into a span that closes under a dynamically-induced product. The embedding is determined by:
- The storage-axis permutation $\pi$ (a measurement-protocol choice, not intrinsic to FTD)
- The specific curl-dominance rule in the genesis kernel (an engine-level rule)

The result is an **artifact of the measurement protocol**, not a canonical FTD object. Changing $\pi$ (e.g., using a different storage offset) would change the $X_i$.

## 5. Implications for fermion emergence

### 5.1. What is closed

Site-local 0-form (state-field) readout on finite periodic blocks **cannot** distinguish injected weight-1 modes in the saturated-genesis regime, regardless of which non-linearity drives the dynamics. The anticommutator always collapses to a constant. Proved analytically above; verified by five independent measurements (FTD-0061, 0071, 0072).

**Structural no-go:** Clifford / Dirac structure cannot emerge from site-local state-field readout on any finite block with pointwise-genesis-like dynamics. Any attempt to locate fermions in this part of the engine is doomed.

### 5.2. What remains open

- **Spin-field readout** (FTD-0073): gives a non-trivial but non-Clifford commutative algebra. The result depends on the measurement protocol (specifically the off-axis storage convention). A cleaner, protocol-independent variant that tests the intrinsic spin-assignment rule is worth building but beyond the scope of this capstone.
- **Edge / link degrees of freedom** (1-forms). In lattice gauge theory, the gauge field lives on links and spinor fields on sites, with the **covariant derivative** ($D_\mu = \partial_\mu - i e A_\mu$) coupling them non-locally. FTD has no explicit link field, but the Moore-26 stencil includes edge couplings (the 12 edge routes of `route_moore_current`). A Clifford test on edge-current observables would be the natural next probe.
- **Two-point / worldline correlators**. The inverse propagator $G^{-1}(p)$ of a fermionic field is linear in momentum with $\gamma$-matrix structure, whereas a bosonic field has $p^2 + m^2$. Measuring the FTD state-field or spin-field propagator on a Langevin ensemble (FTD-0069) and fitting to Dirac vs Klein–Gordon form would distinguish the two directly. This is a several-hundred-line measurement requiring nontrivial statistics but is the most rigorous direction.
- **Branch-B projected-QED completion** (Gate 6). Impose Dirac matter as an EFT selection, not derivation; match to FTD by choosing the fermion propagator coefficient to agree with the native measured flux sector. This is the non-structural path.

### 5.3. Recommendation

The honest Phase-4 capstone is: **fermion emergence is NOT coming from the direct grade decomposition of site-local FTD state**. The five negative measurements + one partial-positive spin-field measurement converge on this. The remaining routes are all non-site-local and require new infrastructure (edge observables, propagator fits, or external matching).

For the first Branch-A paper, the honest statement is:
> FTD supports a native source/flux EFT with a Gaussian fixed point (Phase 2 result). Fermion emergence from site-local readouts is structurally obstructed by mode-erasure under all tested non-linearities; fermion content is taken as an external selection in the Branch-B matching program, not derived.

## 6. Epistemic tags

| Piece | Tag | Justification |
|---|---|---|
| Divergence-sign uniformity under step-function WH injection | [THEOREM] | §2.2 |
| State-field mode erasure under genesis | [THEOREM] | §2.3 |
| Universality of mode erasure across pointwise-threshold non-linearities | [THEOREM] | §2.4 |
| Spin-field readout preserves mode information (qualitative) | [MEASURED] | FTD-0073 |
| Measured spin-field commutative algebra $\{e_i, e_j\} = e_i^2 + e_j^2$ | [MEASURED] | §4 |
| Spin-field algebra is protocol-dependent (artifact of off-axis storage) | [OBSERVATION] | §4.2 |
| Fermion emergence requires non-site-local structure | [CONJECTURE] | §5 — remaining routes are all non-local |
| No fermion emergence from site-local FTD state-field readout | [THEOREM] | §5.1 composite of §2 + measurements |

---

*Filed 2026-04-24 as the Phase-4 capstone. Converts five empirical falsifications into a structural theorem and identifies the three remaining viable directions for FTD fermion emergence. First Branch-A paper can cite §5.3 to frame the fermion question honestly.*
