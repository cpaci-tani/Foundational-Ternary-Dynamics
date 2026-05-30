# Derivation: Walsh–Hadamard Algebra on a 2³ Block Is Not Clifford

**Date:** 2026-04-24
**Status:** [THEOREM] (structural no-go); [CONJECTURE] narrowed for dynamical emergence
**Companion of:** [EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md](EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md)
**Ledger row:** FTD-0061

This note carries out the multiplication tables and the ring-theoretic argument in full, so that the no-go is fully explicit and the question "what dynamical ingredient would be needed for Clifford emergence" is sharp.

---

## 1. Setup

Let $V = \{-1, 0, +1\}^8$ be the ternary configuration space of a $2 \times 2 \times 2$ block. Label the 8 sites by $x = (x_1, x_2, x_3) \in \{0, 1\}^3$. For each $v \in (\mathbb{Z}/2)^3$ define the Walsh–Hadamard character

$$ \chi_v : \{0,1\}^3 \to \{\pm 1\}, \qquad \chi_v(x) = (-1)^{v \cdot x} = (-1)^{v_1 x_1 + v_2 x_2 + v_3 x_3}. $$

The 8 characters $\{\chi_v : v \in (\mathbb{Z}/2)^3\}$ are an orthonormal basis of $\mathbb{R}^8$ under the inner product $\langle f, g \rangle = \tfrac{1}{8} \sum_x f(x) g(x)$.

The **Walsh–Hadamard decomposition** of an observable $s : \{0,1\}^3 \to \mathbb{R}$ is

$$ s(x) = \sum_v \hat s(v) \chi_v(x), \qquad \hat s(v) = \frac{1}{8} \sum_x s(x) \chi_v(x). $$

The 8 Fourier coefficients $\hat s(v)$ split by Hamming weight $|v|$:

| $|v|$ | count | interpretation |
|---|---|---|
| 0 | 1 | scalar (mean density) |
| 1 | 3 | vector (3 axis-aligned dipoles) |
| 2 | 3 | bivector (3 face-diagonal quadrupoles) |
| 3 | 1 | pseudoscalar (body-diagonal) |

This is the "1 + 3 + 3 + 1" decomposition.

## 2. The natural product: group algebra

Under pointwise multiplication of functions on $\{0,1\}^3$:

$$ (\chi_u \cdot \chi_v)(x) = (-1)^{u \cdot x} (-1)^{v \cdot x} = (-1)^{(u \oplus v) \cdot x} = \chi_{u \oplus v}(x), $$

where $\oplus$ is componentwise XOR.

This is the **group algebra** structure

$$ A_{\mathrm{WH}} := \mathbb{R}[(\mathbb{Z}/2)^3] $$

with basis $\{\chi_v\}_v$ and multiplication $\chi_u \cdot \chi_v = \chi_{u \oplus v}$.

Label the 8 basis vectors $e_v$ for $v \in \{000, 100, 010, 001, 110, 101, 011, 111\}$ (writing $v$ as a 3-bit string). The multiplication table is:

| $\cdot$ | $e_{000}$ | $e_{100}$ | $e_{010}$ | $e_{001}$ | $e_{110}$ | $e_{101}$ | $e_{011}$ | $e_{111}$ |
|---|---|---|---|---|---|---|---|---|
| $e_{000}$ | $e_{000}$ | $e_{100}$ | $e_{010}$ | $e_{001}$ | $e_{110}$ | $e_{101}$ | $e_{011}$ | $e_{111}$ |
| $e_{100}$ | $e_{100}$ | $e_{000}$ | $e_{110}$ | $e_{101}$ | $e_{010}$ | $e_{001}$ | $e_{111}$ | $e_{011}$ |
| $e_{010}$ | $e_{010}$ | $e_{110}$ | $e_{000}$ | $e_{011}$ | $e_{100}$ | $e_{111}$ | $e_{001}$ | $e_{101}$ |
| $e_{001}$ | $e_{001}$ | $e_{101}$ | $e_{011}$ | $e_{000}$ | $e_{111}$ | $e_{100}$ | $e_{010}$ | $e_{110}$ |
| $e_{110}$ | $e_{110}$ | $e_{010}$ | $e_{100}$ | $e_{111}$ | $e_{000}$ | $e_{011}$ | $e_{101}$ | $e_{001}$ |
| $e_{101}$ | $e_{101}$ | $e_{001}$ | $e_{111}$ | $e_{100}$ | $e_{011}$ | $e_{000}$ | $e_{110}$ | $e_{010}$ |
| $e_{011}$ | $e_{011}$ | $e_{111}$ | $e_{001}$ | $e_{010}$ | $e_{101}$ | $e_{110}$ | $e_{000}$ | $e_{100}$ |
| $e_{111}$ | $e_{111}$ | $e_{011}$ | $e_{101}$ | $e_{110}$ | $e_{001}$ | $e_{010}$ | $e_{100}$ | $e_{000}$ |

Two structural features to notice:

1. The table is **symmetric** across the diagonal: $e_u e_v = e_v e_u$ for all $u, v$. The algebra is **abelian**.
2. $e_v^2 = e_{000} = \mathbb{1}$ for every $v$. Every element is its own inverse.

As a ring, $A_{\mathrm{WH}}$ is isomorphic to $\mathbb{R}^8$ (direct product of 8 copies of $\mathbb{R}$), with the isomorphism given by evaluation at the 8 elements of $(\mathbb{Z}/2)^3$ (equivalently: the Fourier transform).

## 3. The target: $\mathrm{Cl}(3,0)$ multiplication table

Choose generators $\gamma_1, \gamma_2, \gamma_3$ with $\gamma_i^2 = +1$ and $\gamma_i \gamma_j = -\gamma_j \gamma_i$ for $i \ne j$. Basis:

$$ \{1, \gamma_1, \gamma_2, \gamma_3, \gamma_1 \gamma_2, \gamma_1 \gamma_3, \gamma_2 \gamma_3, \gamma_1 \gamma_2 \gamma_3\}. $$

Let $\omega = \gamma_1 \gamma_2 \gamma_3$ (the pseudoscalar). Basic identities:

$$ \omega^2 = \gamma_1 \gamma_2 \gamma_3 \gamma_1 \gamma_2 \gamma_3 = -\gamma_1 \gamma_1 \gamma_2 \gamma_2 \gamma_3 \gamma_3 \cdot \text{(sign factor from reordering)} $$

Carefully: $\gamma_1 \gamma_2 \gamma_3 \gamma_1 = -\gamma_1 \gamma_2 \gamma_1 \gamma_3 = \gamma_1 \gamma_1 \gamma_2 \gamma_3 = \gamma_2 \gamma_3$.
So $\omega^2 = \gamma_2 \gamma_3 \gamma_2 \gamma_3 = -\gamma_2 \gamma_2 \gamma_3 \gamma_3 = -1$.

Thus $\omega^2 = -1$.

Partial multiplication table (just the non-commutative sector):

| $\cdot$ | $\gamma_1$ | $\gamma_2$ | $\gamma_3$ |
|---|---|---|---|
| $\gamma_1$ | $1$ | $\gamma_1\gamma_2$ | $\gamma_1\gamma_3$ |
| $\gamma_2$ | $-\gamma_1\gamma_2$ | $1$ | $\gamma_2\gamma_3$ |
| $\gamma_3$ | $-\gamma_1\gamma_3$ | $-\gamma_2\gamma_3$ | $1$ |

Off-diagonal entries above and below the diagonal differ in sign.

As a ring: $\mathrm{Cl}(3,0) \cong M_2(\mathbb{R}) \oplus M_2(\mathbb{R})$. The isomorphism sends $\omega \mapsto$ the central idempotent that splits the two copies, and each copy is 4-dim = $M_2(\mathbb{R})$.

## 4. The no-go, in full

**Theorem (No-go).** *$A_{\mathrm{WH}} \ncong \mathrm{Cl}(3,0)$ as rings.*

**Proof.**

1. Commutativity: $A_{\mathrm{WH}}$ is abelian (every entry of §2's table is symmetric). $\mathrm{Cl}(3,0)$ is not (e.g. $\gamma_1 \gamma_2 = -\gamma_2 \gamma_1 \ne \gamma_2 \gamma_1$).

2. A ring isomorphism preserves commutativity. Hence no ring isomorphism can exist. $\square$

**Corollary.** *No bijection $\phi$ on the 8 basis vectors (a permutation $\sigma \in S_8$) converts the WH multiplication table into the $\mathrm{Cl}(3,0)$ table.*

**Proof.** Permutations don't change whether a table is symmetric. $\square$

**Stronger corollary.** *No change of basis (an element of $GL_8(\mathbb{R})$) on the 8-dim vector space converts the WH product into the $\mathrm{Cl}(3,0)$ product, because change of basis preserves the ring structure up to isomorphism.*

In other words: one cannot recover Clifford structure from the WH decomposition alone by any choice of labels or linear recombinations. The product has to come from somewhere else.

## 5. Where must an additional dynamical ingredient come from?

The FTD tick cycle operates on the block via:

(a) **Phase-read Laplacian.** $\Delta_{18}$ is a linear, translation-invariant operator on functions on the lattice. Restricted to a 2³ block with periodic boundary conditions, it is **diagonal in the WH basis** — each character $\chi_v$ is an eigenfunction with eigenvalue

$$ \lambda_v = -2 \sum_i (1 - \cos(\pi v_i)) + (\text{off-axis Moore terms}). $$

Diagonal linear operators commute with every basis element and hence cannot generate a Clifford product.

(b) **Phase-write leapfrog.** Still linear, diagonal in WH. Same conclusion.

(c) **Gauss projection.** Linear solve for a potential given a source. Linear in WH.

(d) **Ternary clamp / state field.** The **only non-linear ingredient** in the tick cycle. The rule $s = \mathrm{sign}_T(|J|)$ (manifestation threshold) is a projection $\mathbb{R}^8 \to \{-1, 0, +1\}^8$. This is the candidate for an emergent bilinear product: two flux configurations $J^{(1)}$ and $J^{(2)}$ do **not** satisfy $s(J^{(1)} + J^{(2)}) = s(J^{(1)}) + s(J^{(2)})$ in general.

(e) **Movement.** The integer-displacement rule transports sources between sites. This is bilinear in the pair (source, velocity) and can be an additional source of emergent product structure.

So the only routes through which a Clifford product could emerge from the existing engine are the **ternary clamp** and the **movement rule**. A derivation of fermion emergence from the b=2 block would need to show that one of these (or their composition) produces, on the 8-dim WH space, a product satisfying $\{e_i, e_j\} = 2\delta_{ij}$ for three distinguished basis elements.

## 6. A sharper open question

Label the three weight-1 WH basis vectors $e_1 = \chi_{100}$, $e_2 = \chi_{010}$, $e_3 = \chi_{001}$. On the **linear** lattice dynamics they satisfy $e_i e_j = e_j e_i = e_{i \oplus j}$ (via §2). Define the **engine-induced product** $\star$ on pairs of WH modes by

$$ (f \star g)(x) := \mathrm{sign}_T(J_{fg}(x)), \quad J_{fg} := \text{one-tick flux response to combined mode } (f, g). $$

(This is a schematic definition — the precise bilinear extraction from the engine's update rule on a 2³ block is worked out below only in outline.)

The sharp open question is:

> **[OPEN]** *Does there exist a labelling of three weight-1 modes $e_1, e_2, e_3$ such that the engine-induced product $\star$ on the 8-dim WH space satisfies $e_i \star e_j + e_j \star e_i = 2\delta_{ij} \mathbb{1}$?*

If yes: Clifford emergence from FTD dynamics would be **derived**, and the PDF's Theorem 1 would have a real proof. If no: the b=2 block does not carry Dirac structure even dynamically, and fermion emergence would need to be sought elsewhere (e.g., at the 18-point Moore scale rather than the 2³ block, or from the Moore layer decomposition of SC + FCC + BCC).

## 7. Outline of a falsifiable program

**Step 1.** Define $\star$ explicitly. Take two flux configurations $J^{(f)}, J^{(g)}$ on the 2³ block, supported respectively on WH modes $f$ and $g$. Run one engine tick with ternary clamp and movement active. Compute the resulting flux $J_{fg}$. Project $\mathrm{sign}_T(J_{fg})$ back onto WH modes. The coefficient on WH mode $h$ is $(f \star g)(h)$.

**Step 2.** Compute the anticommutator $(f \star g + g \star f)(h)$ for all ordered triples of weight-1 modes. This is 27 scalar evaluations per tick.

**Step 3.** Check whether there exists a permutation of $(e_1, e_2, e_3)$ such that the anticommutator equals $2\delta_{ij} e_0$ (the constant mode $e_0 = \chi_{000} = \mathbb{1}$).

**Step 4.** Repeat with the L/R chiral decomposition, the dual-substrate path, and with small external source to break degeneracies.

**Step 5.** If the anticommutator is nowhere close to $2\delta_{ij} \mathbb{1}$, the conjecture is falsified. If it is close but not exact, investigate whether it matches a different algebra (exterior, quaternion, octonion, ... ).

This is a concrete simulation protocol — 27 to ~100 tick runs on a 2³ block with the engine. It is cheap enough to do and would settle the question either way.

## 8. Measurement (executed 2026-04-24)

Steps 1–3 carried out via `engine/tests/test_wh_clifford_anticommutator.cpp`, registered as CTest `wh_clifford_anticommutator` (labels: gpu native eft). Ran on the CUDA backend (L=8 lattice, 2³ injection block at origin, amplitude $A = 10$ to make genesis deterministic, matched RNG seed across orderings).

The resulting anticommutator matrix on the three weight-1 modes $e_1 = \chi_{100}$, $e_2 = \chi_{010}$, $e_3 = \chi_{001}$:

| pair | ident | $v$=x | $v$=y | xy | $v$=z | xz | yz | xyz |
|---|---|---|---|---|---|---|---|---|
| $\{e_1, e_1\}$ | +2.000 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| $\{e_1, e_2\}$ | +2.000 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| $\{e_1, e_3\}$ | +2.000 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| $\{e_2, e_2\}$ | +2.000 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| $\{e_2, e_3\}$ | +2.000 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| $\{e_3, e_3\}$ | +2.000 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

**Reading.** Every anticommutator, diagonal and off-diagonal, is $2 \cdot \mathbb{1}$. Clifford requires $\{e_i, e_j\} = 2 \delta_{ij} \cdot \mathbb{1}$, which agrees on the diagonal but demands **zero** off-diagonal. The measured off-diagonal is $+2 \cdot \mathbb{1}$ — maximally wrong.

**Root cause.** The genesis rule sets $s_\text{new} = \mathrm{sign}(\nabla \cdot J)$ only at sites with $|J| > 3 K_B$ and $s_\text{prev} = 0$. For any of the three weight-1 modes injected on its matching axis, the divergence at every site in the 2³ block has the same sign (the injection is sharp-edged at the block boundary and smooth inside). Every weight-1 mode therefore produces the same uniform scalar state, losing the mode index before the second tick. $f \star g$ is independent of $(f, g)$ — the operation is not a Clifford product at all; it is a constant map to the identity mode.

**Conclusion (FTD-0061 extension).** Clifford emergence on the 2³ block via the ternary-clamp + movement non-linearity is **falsified**. The anticommutator is $\{e_i, e_j\} = 2 \cdot \mathbb{1}$ everywhere, not $2 \delta_{ij} \cdot \mathbb{1}$.

**What the conjecture becomes.** The residual routes to fermion emergence within FTD are:

1. **Pair production.** `toggles.pair_production` has a different threshold and sign rule than genesis. Repeat the measurement with pair_production enabled instead.
2. **Weak transmutation.** `toggles.weak_transmutation` is a stress-threshold polarity flip on already-manifested sites. Could provide an order-dependent mechanism that doesn't collapse on first tick.
3. **Moore-26 layer decomposition.** The SC + FCC + BCC structure of [THEOREM_MOORE_LAYER_DECOMPOSITION.md](../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md) carries richer algebraic content than the 2³ block. The 26-site stencil's grade structure could host Clifford generators that the 8-site block cannot.
4. **Velocity-driven movement.** Enable `toggles.forces` so phase_forces drives velocity, which activates phase_movement's annihilation branch. Test whether annihilation between sequential injections is order-dependent.

Each is a separate measurement protocol with the same test harness. Under the current falsification the "fermion emergence from b=2 block" direction is closed; the "fermion emergence from FTD" direction remains open through the four routes above.

## 9. Phase-4 alt-route measurements (executed 2026-04-24)

All four routes tested on GPU:

### 9.1. Routes 4a/4b/4d on the 2³ block

`engine/tests/test_wh_clifford_alt_routes.cpp` (ctest `wh_clifford_alt_routes`) — identical protocol to §8, varying only the non-linearity:

| Route | Non-linearity | Clifford pairs | Pattern |
|---|---|---|---|
| Original (FTD-0061) | genesis + movement | 3 / 6 | $\{e_i,e_j\} = 2\mathbb{1}$ for all (i,j) |
| 4a | pair_production + preseeded state | 3 / 6 | Identical collapse |
| 4b | weak_transmutation on dual substrate + preseeded state | 3 / 6 | Identical collapse |
| 4d | forces + poisson_coulomb + movement + genesis | 3 / 6 | Identical collapse |

**Universal result: every engine non-linearity collapses every weight-1 WH mode injection to the same uniform ±1 state on the 2³ block.** The anticommutator is $\{e_i, e_j\} = 2 \cdot \mathbb{1}$ independent of (i, j), independent of which toggle drives the dynamics. The mode index is erased before the second tick can expose ordering asymmetry. **Clifford emergence from the 2³ block is universally falsified across the engine-toggle catalog.**

### 9.2. Route 4c on the Moore-26 / 3³ block

`engine/tests/test_moore26_clifford_test.cpp` (ctest `moore26_clifford_test`) — axial-dipole "sawtooth" modes $e_x(x,y,z) = \mathrm{sign}(1 - x)$ etc. on a 3³ block, genesis+movement non-linearity at L=8 ambient lattice:

| pair | $\{e_f, e_g\}$ on identity | on $e_x$ | on $e_y$ | on $e_z$ |
|---|---|---|---|---|
| $\{e_1, e_1\}$ | $-1.333$ | 0 | 0 | 0 |
| $\{e_1, e_2\}$ | $-1.222$ | $-0.056$ | 0 | 0 |
| $\{e_1, e_3\}$ | $-1.222$ | $-0.056$ | 0 | 0 |
| $\{e_2, e_2\}$ | $-1.333$ | 0 | 0 | 0 |
| $\{e_2, e_3\}$ | $-1.185$ | 0 | 0 | 0 |
| $\{e_3, e_3\}$ | $-1.333$ | 0 | 0 | 0 |

Diagonal value $-1.33 = -4/3$ (not $+2$): the dipole injection's divergence pattern drives the block to mostly $-1$ rather than uniform $+1$. Off-diagonal $-1.22$ nearly matches diagonal (difference $\approx 0.11$): mode information is mostly but not entirely erased (the small $-0.056$ on $e_x$ off-diagonal is the residue of order-dependence).

Clifford pairs: 0 / 6 at the required tolerance. The Moore-26 / 3³ block also does not support Clifford structure on axial-dipole modes under genesis+movement.

### 9.3. Combined Phase-4 verdict

Five independent measurement protocols (original FTD-0061, plus 4a, 4b, 4c, 4d) all falsify Clifford emergence from direct grade-structure bases. The residual routes to fermion emergence within FTD are narrowed to:

- **Projected-QED Dirac completion** (Branch B matter sector, Gate 6): Dirac structure imposed by matching to a continuum EFT, not derived from block decomposition. Still [SELECTION].
- **Non-block structures**: constructions that do not rely on spatial grade decomposition (e.g., worldline constructions with intrinsic spin labels, or tensor-category data attached to edges/plaquettes rather than vertices). Still [OPEN].

The "fermion emergence from FTD dynamics" direction is **not closed**, but every direct grade-decomposition candidate has now been tested and falsified. Further progress requires a genuinely different construction, not a new non-linearity on the same block geometry.

## 8. Summary

| Piece | Status |
|---|---|
| b=2 WH basis splits 1+3+3+1 by Hamming weight | [THEOREM] (character theory) |
| Pointwise product gives $\mathbb{R}[(\mathbb{Z}/2)^3]$, abelian | [THEOREM] (§2) |
| $\mathbb{R}[(\mathbb{Z}/2)^3] \ncong \mathrm{Cl}(3,0)$ as rings | [THEOREM] (§4) |
| No linear redefinition recovers Clifford product from WH | [THEOREM] (corollary §4) |
| Only non-linear engine ingredients are ternary clamp + movement | [OBSERVATION] |
| Engine-induced $\star$ satisfies Clifford anticommutator on weight-1 modes | [OPEN / FALSIFIABLE] |
| Dirac structure additionally requires Cl(1,3), not Cl(3,0) | [OBSERVATION] |

The first four rows are permanent. The fifth is a pointer. The sixth is the live research question and is decidable by a short simulation.

---

*This derivation replaces the unqualified "Theorem 1" in the PDF draft with: (i) a theorem about what the WH decomposition cannot do by itself, and (ii) a falsifiable protocol that could still rescue the fermion-emergence direction if the ternary-clamp-induced product turns out to be Clifford. Neither step has been done yet; both are now well-posed.*
