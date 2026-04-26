# Phase-4i: Color Binding Structure + Honest Assessment of m_e Promotion to [THEOREM]

**Date:** 2026-04-24
**Status:** [MEASURED] for binding + color structure; [SELECTION] status of $m_e$ formula UNCHANGED (promotion blockers identified)
**Ledger row:** FTD-0077

Three distinct results in one document:

1. **3-quark binding test** — does an RGB triad form a color-singlet bound state?
2. **Color transformation structure** — does FTD "color" transform as SU(3)?
3. **$m_e$ → [THEOREM] assessment** — can the electron mass formula be promoted?

---

## 1. 3-Quark Binding Test (Task 1)

### Protocol

Stamp three quarks with the same charge (+1) and same spin (+1) on an equilateral triangle of side $r = 2$ lattice units, with all forces enabled: Coulomb, strong, exchange, triad_binding, movement. Three color configurations:

- **(A) RGB** — color-singlet candidate in SU(3)
- **(B) RRR** — single-color, should be color-forbidden if SU(3) is physical
- **(C) RGG** — mixed non-singlet

Run for $N_{\mathrm{TICKS}} = 60$, measure final RMS pairwise separation.

### Result

| Config | Initial sep | Final sep | Binding ratio $(d_0 / d_f)$ |
|---|---|---|---|
| (A) RGB | 3.742 | 2.160 | 1.732 (moderate bind) |
| (B) RRR | 3.742 | 0.000 | $\infty$ (collapse/merge) |
| (C) RGG | 3.742 | 1.155 | 3.240 (tight bind) |

**Verdict.** All three configurations bind. Notably, RGG binds *tighter* than RGB, and RRR collapses entirely. **FTD's triad + strong + exchange machinery does NOT implement SU(3) confinement** — binding depends on charge sign and proximity, not on color-singlet structure. Color labels are dynamically *decorative* in the current engine.

This is an important honest finding: FTD's "color" is a label the engine tracks but does not enforce as an SU(3) gauge charge. The strong/triad dynamics bind same-sign charges regardless of color assignment.

## 2. Color Transformation Structure (Task 2)

### Protocol

Same (A) RGB configuration above, then the color-permuted (G, B, R) at the same positions. 120° body-diagonal rotation of the cubic lattice permutes $x \to y \to z \to x$, which is exactly the R→G→B→R color cycle. If FTD color is even O_h-symmetric (let alone SU(3)-symmetric), the two configurations must produce identical observables.

### Result

| Configuration | Final RMS sep |
|---|---|
| (R, G, B) | 2.160 |
| (G, B, R) | 2.160 |
| $|\Delta|$ | **0.0000** (exact) |

### Analysis

**O_h permutation symmetry (cyclic $C_3 \subset S_3 \subset O_h$): CONFIRMED to machine precision.** The engine's color dynamics are invariant under the discrete permutation of axis labels that corresponds to the body-diagonal rotation.

However:

- **Continuous SU(3) is structurally impossible** at the FTD engine level. The color field is stored as `int8_t color ∈ {0, 1, 2, 3}`. Continuous SU(3) rotations mix R, G, B into arbitrary complex superpositions; these cannot be represented in a 2-bit discrete space.
- **FTD color therefore transforms as the discrete subgroup $S_3 \subset SU(3)$**, or more generally the finite cubic group $O_h$, not as full continuous SU(3).

### Verdict

FTD's color implementation has the **discrete-permutation** subgroup of SU(3) as a symmetry, but not full continuous SU(3). For the emergent-QCD story, this is enough to give the *N_c = 3* integer and to satisfy color-permutation identities, but it does **not** reproduce the full continuous SU(3) gauge structure of QCD. Combined with the Task-1 binding result, **FTD's color is a label with cyclic $C_3$/$S_3$ symmetry but not a genuine SU(3) gauge charge**.

This is consistent with the existing [SELECTION] tag on "FTD color = SU(3) color" — the identification is motivational, not proven.

## 3. $m_e$ Formula: Factor-by-Factor Promotion Assessment (Task 3)

The current formula is

$$ m_e = m_P \cdot \sqrt{2\pi} \cdot \frac{16}{3} \cdot \alpha^{11} $$

matching PDG to 0.19%. The question: can it be promoted from [SELECTION] to [THEOREM]?

### Factor-by-factor current status (from `DERIV_ELECTRON_MASS_MOTIVATION.md`)

| Component | Current tag | Basis |
|---|---|---|
| Dimensional form $m \propto m_P \cdot \alpha^n$ | [IMPOSED] | Standard template |
| $\sqrt{2\pi}$ | [THEOREM] | Gaussian J-integral, exact |
| $16 = |\mathrm{Aut}(E_i)|^2$ | [THEOREM] | Faddeev–Popov on $O_h/\mathbb{Z}_3$ |
| $3 = D$ (spatial dimension) | [THEOREM] | $16 = 2^D \cdot (D-1)!$ algebraic uniqueness |
| Exponent $n = 11$ | **[SELECTION]** | Ladder walk $4 + 4 + 3$ |
| Combined formula | **[SELECTION]** | Not derived from FTD action |

**Three of the six components are already [THEOREM].** Two are [SELECTION], one is [IMPOSED].

### What [THEOREM] promotion requires

**Upgrade path A: Derive the exponent 11 uniquely.**
The ladder rule in `FOUND_LADDER_GENERATING_RULE.md` states that the walk $\{4, 4, 3, 3, 6\}$ sums to $16 = k_{\mathrm{phys}}$ (master quadratic coefficient). This is a **structural identity** [THEOREM]. What's [SELECTION] is the *ordering* that assigns specific particles to specific cumulative-sum positions:

```text
n = 4 → perturbative boundary
n = 8 → Higgs VEV       (+ N_base = 4)
n = 11 → electron mass   (+ N_c = 3)
n = 14 → neutrino mass   (+ N_c = 3)
n = 20 → gravity scale   (+ N_f = 6)
```

The ordering is physically motivated (mass needs Higgs first, stable hadrons next, neutrino seesaw requires two colors, gravity sees all flavors), but **no first-principles FTD calculation uniquely forces this specific order**. A derivation of the exponent 11 would require:

1. Deriving the walk addends $\{4, 3, 3, 6\}$ as a multiset (already [THEOREM]).
2. Deriving the specific *ordering* of addends (currently [SELECTION]).
3. Deriving that the electron sits at exactly step 2 in the walk, i.e., that $n_e = 4 + 4 + 3 = 11$ and not any other cumulative partial sum.

Steps 2 and 3 are where the promotion gets stuck. Without an independent proof that "the electron's mass scale receives contributions from exactly EM + EW + QCD gauge sectors, and not from flavor", the exponent 11 remains selected.

**Upgrade path B: First-principles pole-mass calculation from the FTD action.**
This route is **blocked** by FTD-0075: the native FTD flux field's 2-point correlator is flat (ultralocal), not a conventional propagator. Electron-like excitations don't have a Klein-Gordon pole mass on the Langevin ensemble. Pole-mass extraction would require either a different generating ensemble or an analytic continuation not yet in hand.

**Upgrade path C: Matching to a separate QED EFT sector.**
Compute the pole mass in a standard lattice QED (not FTD) and argue that FTD reproduces it via matching at the Branch-B level. This is a Branch-B project, and even if successful it would promote *electron mass in projected QED*, not *electron mass in native FTD*. The [SELECTION] tag on the *combined FTD formula* would remain.

### Honest conclusion

**$m_e$ cannot be promoted to [THEOREM] with existing FTD tooling.** The three prefactor components (√(2π), 16, D=3) are already [THEOREM] — that gives the structural prefactor $16\sqrt{2\pi}/3$ rigorously. The remaining blockers:

1. **Exponent 11** requires uniquely fixing the ladder ordering, which is currently [SELECTION].
2. **Combined formula** requires a Lagrangian-level derivation, which is blocked by the flux field's ultralocality (FTD-0075).

Honest tag table after this audit:

| Component | Tag |
|---|---|
| Prefactor $16 \sqrt{2\pi}/3$ | **[THEOREM]** (unchanged, already theorem) |
| Combined with $\alpha^n$ for unknown $n$ | [IMPOSED template] |
| Specific $n = 11$ | [SELECTION] (bottleneck) |
| Therefore $m_e$ formula | [SELECTION] (bottleneck) |

**Epistemic recommendation:** do **not** promote $m_e$ to [THEOREM] at this time. The existing [SELECTION] tag is the correct status. Promotion requires new derivation work — specifically, a first-principles derivation of the ladder-position assignment that places the electron at $n = 11$, not post-hoc matching to the PDG value. Attempting to promote without that work would violate the project's epistemic discipline (cf. CLAUDE.md §Epistemic Discipline on substitution identities).

This document therefore **preserves** the [SELECTION] tag rather than promoting. The honest accounting is that $m_e$ is a structurally-motivated near-miss with most of its factors rigorous but the key exponent requiring additional work to upgrade.

### What promotion would look like, should it become available

If a future derivation fixes the ladder ordering rigorously (e.g., via a topological counting argument or an action-level derivation of mass-scale contributions from gauge sectors), the promotion would proceed as:

```text
Current:
  m_e = m_P · [THEOREM prefactor 16√(2π)/3] · α^[SELECTION 11]     → [SELECTION]

Future (if 11 becomes [THEOREM]):
  m_e = m_P · [THEOREM prefactor] · α^[THEOREM 11]                  → [THEOREM up to m_P scale]

Full [THEOREM]:
  Additionally requires pole-mass calculation from FTD action.
  Blocked by FTD-0075 until a different generating ensemble is found
  or an analytic continuation supplies a conventional propagator.
```

The first step (exponent promotion) is a theoretical research program, not a simple upgrade. The second step may be fundamentally blocked by the engine's ultralocality.

## 4. Summary of Phase-4i outcomes

| Task | Measurement | Verdict |
|---|---|---|
| 3-quark binding | RGB sep 2.16, RRR sep 0, RGG sep 1.15 | **All three bind; color not SU(3)-enforced** |
| Color transformation | (R,G,B) vs (G,B,R) sep Δ=0.0000 | **$C_3 \subset O_h$ symmetry confirmed; continuous SU(3) structurally impossible** |
| $m_e$ promotion | Factor-by-factor audit | **[SELECTION] preserved; promotion blocked by exponent and ultralocality** |

The FTD emergence story, with this Phase-4i addendum, is now:

- **Quark-like single-voxel emergents from genesis** (FTD-0076)
- **Color labels have $C_3 / O_h$ symmetry but are not SU(3)-enforced dynamically** (this doc)
- **3-quark triads bind via same-sign + proximity, not by color singlet** (this doc)
- **Electron mass formula has [THEOREM] prefactor, [SELECTION] exponent, remains [SELECTION] overall** (this doc)

This is the honest state. No overclaim, no promotion without derivation.

## 5. Epistemic tags (this document)

| Piece | Tag |
|---|---|
| RGB/RRR/RGG binding pattern from engine measurement | [MEASURED] |
| FTD color does not enforce SU(3) singlet confinement | [THEOREM] (code + measurement) |
| FTD color transforms as $C_3 \subset O_h$ | [MEASURED], [THEOREM] for $C_3$ level |
| FTD color is not continuous SU(3) | [THEOREM] (int8_t storage limit) |
| $m_e$ formula cannot be promoted to [THEOREM] without new derivation work | [THEOREM] (audit result) |
| Exponent 11 requires independent derivation for promotion | [OBSERVATION] |

---

*Filed 2026-04-24 as Phase-4i. Preserves epistemic discipline: reports what is measured, classifies the binding and color structure honestly, and declines to promote $m_e$ without the required derivation. Future promotion path explicitly outlined.*
