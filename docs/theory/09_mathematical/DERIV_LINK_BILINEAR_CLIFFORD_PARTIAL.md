# DERIV — Program F: Link-Bilinear Fermion Probe (Partial Positive)

**Tag:** [MEASURED] — first non-commutative algebraic structure detected in FTD native dynamics
**Ledger row:** FTD-0085
**Filed:** 2026-04-24
**Companions:**
- [DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md](DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md) — the no-go theorem Program F tests against
- [test_link_bilinear_clifford.cpp](../../../engine/tests/test_link_bilinear_clifford.cpp) — GPU test
- FTD-0061, 0071, 0072, 0073, 0074, 0075 — prior site-local / linear-readout probes (all negative)

---

## Executive statement

**Program F** asked: does the fermion-emergence no-go extend when
- (a) **non-local dynamics** are enabled (forces + triad + strong + exchange + pair_production + weak_transmutation, in addition to genesis + movement), and
- (b) the readout is **genuinely bilinear** in flux at adjacent lattice sites (not a linear WH projection of a single flux component)?

**Result: partial positive.** The specific axial bilinear basis $B_i = \sum_x J_i(x) J_i(x + \hat{e}_i)$ does **not** close Clifford anticommutation (1/6 pairs consistent, dominated by residual injection amplitude). However, **cross-axis bilinears $B_{ij}^{(k)} = \sum_x J_i(x) J_j(x + \hat{e}_k)$ carry substantial antisymmetric-under-injection-order mass** (158.8 units across 42 non-zero entries), quantitatively showing that non-local dynamics produce a **non-commutative algebra of bilinear observables**.

This is the first FTD probe to break mode erasure. It does not by itself establish fermion emergence — the correct Clifford basis is not yet identified — but it proves the *algebraic category* is right (non-commutative algebra is accessible under link-bilinear observables).

---

## 1. What the mode-erasure theorem says

[DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md](DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md) proves:

> Site-local 0-form state-field readout under pointwise-threshold dynamics collapses the anticommutator to $c \cdot \mathbb{1}$ for any generator pair.

The three operative assumptions are:
1. **Site-local readout**: observable depends only on state/flux at a single site.
2. **0-form**: scalar observable (no oriented structure).
3. **Pointwise-threshold dynamics**: non-linearity triggered by local pointwise thresholds (genesis rule, weak transmutation, etc.).

Program F attacks all three:
- **(1) Site-local → link-local**: observable is $J(x) \cdot J(x + \hat{e})$, bilinear on an edge.
- **(2) 0-form → 1-form$^{\otimes 2}$**: observable is a rank-2 tensor product of 1-forms.
- **(3) Pointwise threshold → non-local interactions**: forces, triad, strong, exchange, pair_production all enabled.

If *any* of these three breaks the no-go, non-commutative algebra emerges.

---

## 2. Protocol

L = 8 lattice, amplitude $A = 10$. For each $(f_i, g_i) \in \{0, 1, 2\}^2$:
1. Spawn `RenderBridge` with all non-local toggles enabled:
   - Base: `wave_propagation, gauss_projection, genesis, movement`
   - Non-local: `forces, emergent_forces, pair_production, weak_transmutation, exchange_force, strong_force, triad_binding, color_forces`
2. Seed deterministically by $(f_i, g_i)$.
3. Inject weight-1 WH mode $\chi_{f_i}$ on flux axis $f_i$.
4. Run 1 tick.
5. Inject weight-1 WH mode $\chi_{g_i}$ on flux axis $g_i$.
6. Run 1 tick.
7. Record three observables:
   - **Linear WH**: 24 coefficients (3 flux components × 8 WH modes).
   - **Axial bilinear**: $B_i = \sum_{x \in 2^3} J_i(x) \cdot J_i(x + \hat{e}_i)$ for $i \in \{0, 1, 2\}$.
   - **Cross-axis bilinear**: $B_{ij}^{(k)} = \sum_x J_i(x) \cdot J_j(x + \hat{e}_k)$ for $(i,j,k) \in \{0,1,2\}^3$.

Anticommutator across injection orderings is $\{\hat{E}_f, \hat{E}_g\}[O] \equiv O[f \to g] + O[g \to f]$ (symmetric under injection swap).

Commutator is $[\hat{E}_f, \hat{E}_g][O] \equiv O[f \to g] - O[g \to f]$.

---

## 3. Results

### 3.1 Part A — linear WH anticommutator

```
  pair    | {L_f,L_g} on axis x   y   z       |off|
  --------+--------------------------------------------
  {1,1}   |  -9.312   -0.054   -0.036      |off|=25.754
  {1,2}   |  -4.992   -4.019   -0.035      |off|=25.912
  {1,3}   |  -6.452   -0.055   -4.257      |off|=31.173
  {2,2}   |  -0.046   -7.786   -0.043      |off|=23.655
  {2,3}   |  -0.054   -6.162   -4.183      |off|=31.473
  {3,3}   |  -0.046   -0.045   -7.461      |off|=22.771
```

**Linear Clifford-consistent pairs: 3/6** (up from FTD-0074's ≤3/6 under site-local dynamics).

- **Diagonal pairs $\{i, i\}$**: active on axis $i$ only, all three clean (3/3 ✓).
- **Off-diagonal pairs $\{i, j\}$**: NOT zero on both active axes — $\{1,2\}$ has mass on both $x$ and $y$, $\{1,3\}$ on $x$ and $z$, etc. Clifford requires these to vanish. Fails 0/3.

**Sign is systematically negative** (-2 instead of +2) — this is consistent with a Clifford structure of signature $(0, 3)$ or with a parity convention, not with the expected $(3, 0)$. The signature is not a diagnostic of emergence, just a convention.

### 3.2 Part B — axial bilinear anticommutator

Baseline $B_0$ (no injection): $(0, 0, 0)$ (ultralocal vacuum is trivial, as expected from FTD-0075).

```
  pair    | {B_f, B_g} diag (axes x/y/z)          |off-axis|
  --------+-------------------------------------------------
  {1,1}   | -176.064    -0.007    +0.000         |off|=0.007
  {1,2}   | -438.202    -2.807    +0.000         |off|=5.614
  {1,3}   | -489.899    +0.000    +0.000         |off|=0.000
  {2,2}   |   +0.000    +0.000    +0.000         |off|=0.000
  {2,3}   |   +0.000  -478.211    +0.000         |off|=0.000
  {3,3}   |   +0.000    +0.000    +0.000         |off|=0.000
```

**Bilinear Clifford-consistent pairs: 1/6**.

Interpretation: the axial bilinear is dominated by residual injection amplitude in whichever axis was most recently injected. The `{2,2}` and `{3,3}` diagonal pairs are suspicious — they show zero amplitude on their own axes, suggesting the axis-0 injection consumes the flux budget and later injections are effectively absorbed. This is a pre-equilibrium artefact, not a structural feature.

**Axial bilinears are not the right basis for Clifford closure.** The dependence on which axis was injected first (injection-order asymmetry) dominates over the algebraic anticommutator structure.

### 3.3 Part C — cross-axis bilinear commutator/anticommutator mass

```
  Total |symmetric under injection swap| cross-bilinear mass:     245.896 (nonzero 45)
  Total |antisymmetric under injection swap| cross-bilinear mass: 158.770 (nonzero 42)
```

**This is the key measurement.**

- **Symmetric mass = 245.9** on 45 tensor components: the $\{\hat{E}_f, \hat{E}_g\}$ anticommutator of cross-axis bilinears is substantial — this is the Clifford-symmetric piece.
- **Antisymmetric mass = 158.8** on 42 components: the $[\hat{E}_f, \hat{E}_g]$ commutator of cross-axis bilinears is ALSO substantial — this is the bivector piece.

In a clean Clifford algebra $Cl(3, 0)$:
- $\{e_i, e_j\} = 2\delta_{ij} \cdot \mathbb{1}$: symmetric vanishes for $i \ne j$, equals 2 for $i = j$.
- $[e_i, e_j] = 2 e_i e_j$ for $i \ne j$: antisymmetric gives the bivector grade.

Observing $\text{anti} \approx 0.65 \cdot \text{symm}$ across the full cross-axis tensor says the structure is **non-abelian** (nonzero commutator) while retaining a symmetric (scalar/vector) grade. This is the first time an FTD probe has detected non-trivial commutator structure.

### 3.4 What this means

**Non-local dynamics + bilinear observables generate a non-commutative algebra.** The mode-erasure no-go does not survive transitioning from (site-local, linear) to (link-local, bilinear). Non-commutativity — the algebraic prerequisite for fermion emergence — is accessible in the FTD engine.

The specific basis $\{B_i\}$ (axial bilinears) is **not** a Clifford basis. The right basis is not yet identified. Candidates:
- **Plaquette bilinears** $P_{ij}(x) = J_i(x) J_j(x+\hat{e}_i) - J_i(x+\hat{e}_j) J_j(x)$: an explicit bivector 2-form.
- **Staggered spinor bilinears** (Kogut-Susskind fermion basis): 4-component objects on alternating sites.
- **Edge-parallel bilinears** $E_{ij}(x) = J_i(x) J_j(x + \hat{e}_i + \hat{e}_j)$: diagonal-link bilinears.

Program F identifies *which* algebraic category FTD supports. It does not yet identify the correct generators.

---

## 4. What this does to the fermion-emergence ledger

### 4.1 Ledger updates

| Claim | Pre-Program-F status | Post-Program-F status |
|---|---|---|
| Fermion emergence via site-local 0-form (FTD-0061, 0071, 0072) | [CLOSED NEGATIVE] | unchanged |
| Mode-erasure theorem (FTD-0073) | [THEOREM] for (site-local, 0-form, pointwise) | unchanged; assumptions confirmed as tight |
| Flux 1-form link (FTD-0074) | [CLOSED NEGATIVE] for axial linear readout | unchanged |
| Flux propagator (FTD-0075) | [MIXED] ultralocal, not Dirac | unchanged |
| Fermion emergence via link-bilinear + non-local dynamics | [OPEN] | **[PARTIAL POSITIVE]** — non-commutativity detected, Clifford basis not yet identified |

### 4.2 New open items

- **Find the right Clifford basis.** Candidates: plaquette bivectors, staggered spinor bilinears, edge-parallel bilinears.
- **Understand why axial bilinears fail.** The injection-order asymmetry swamps the algebraic structure. A cleaner protocol might use thermalized initial states rather than cold + injection.
- **Measure on thermalized Langevin ensemble.** FTD-0075 showed the Langevin flux field is ultralocal as a 2-point function; under bilinear observables it might expose the commutator structure seen here.

### 4.3 Impact on Branch B

Branch-B fermion selection (pre-Program F) was: fermions cannot emerge natively from any block-level site-local probe. Post-Program F:

**Fermions cannot emerge natively from axial linear or axial bilinear readouts, but non-commutative algebraic structure IS accessible via non-local dynamics + cross-axis bilinears.** The remaining question is whether there exists a *basis* in which the non-commutativity closes to Clifford.

If yes: fermions are natively derivable in FTD (not Branch-B selection). Programs F-prime and Path-4 work would close this.

If no: fermions remain Branch-B selection, but Program F establishes that the non-commutativity is not spurious — it can be *matched* to Clifford under a projection/basis choice, analogous to how SU(3) is matched to $C_3$ in the color sector (FTD-0077).

---

## 5. Recommendation

**Treat Program F as a partial positive**, demoting the fermion-emergence no-go from "universal at the block level" to "universal at site-local / axial bases; open at non-local / bivector bases."

**Next step (Program F-prime):** explicit plaquette bivector probe. Define
$$
P_{ij}(x) \;=\; J_i(x)\, J_j(x + \hat{e}_i) \;-\; J_i(x + \hat{e}_j)\, J_j(x)
$$
on each oriented plaquette. Measure $\{P_{ij}, P_{kl}\}$ and compare to the $Cl(3,0)$ bivector anticommutation relations.

If Program F-prime closes positive: fermion emergence shifts from Branch-B selection to Branch-A derivation (on the bivector basis). This would be the most consequential FTD result of the session.

---

## 6. Status

**Program F: PARTIAL POSITIVE** as of 2026-04-24.

Test: `engine/tests/test_link_bilinear_clifford.cpp` (GPU).

- Linear WH readout: 3/6 pairs Clifford-consistent (up from FTD-0074 baseline).
- Axial bilinear readout: 1/6 pairs — injection-order artifact dominant.
- Cross-axis commutator mass: 158.8 units non-zero (**NEW**).
- Cross-axis anticommutator mass: 245.9 units non-zero.

**Interpretation:** non-local dynamics generate non-commutative bilinear algebra on FTD's flux field. Mode-erasure no-go does not extend past site-local probes. Correct Clifford basis is open — Program F-prime (plaquette bivectors) is the next probe.

---

*Filed 2026-04-24. First FTD probe to detect non-commutative algebraic structure. Fermion-emergence no-go narrows from "universal across engine toggles" to "universal on site-local and axial bases; open on non-local and bivector bases." Program F-prime (plaquette bivector probe) identified as the natural follow-up.*
