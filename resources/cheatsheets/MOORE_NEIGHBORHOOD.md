# Moore Neighborhood Reference

The 26-cell Moore neighborhood is where FTD's four framework integers actually come from. This sheet is the compact reference.

## Definition

For a voxel at origin `(0, 0, 0)`, the Moore neighborhood is every lattice point within one step in any axis:

```
M = { (i, j, k) : i, j, k ∈ {−1, 0, +1} } \ { (0, 0, 0) }       |M| = 26
```

Including the origin makes `3³ = 27` cells total — the "existential unit".

## The polyhedral decomposition

The 26 neighbors partition by distance from origin into three polyhedra:

| Shell | Count | Distance | Lattice | Role |
|---|---|---|---|---|
| Face | 6 | 1 | simple cubic (SC) | axes: `(±1,0,0), (0,±1,0), (0,0,±1)` |
| Edge | 12 | √2 | face-centered cubic (FCC) | `(±1,±1,0)` + perms |
| Corner | 8 | √3 | body-centered cubic (BCC) | `(±1,±1,±1)` |

```
6 + 12 + 8 = 26 = |M|
```

- 6 faces → **octahedron**
- 12 edges → **cuboctahedron**
- 8 corners → **stella octangula** (two interpenetrating tetrahedra)

## The Moore Layer Theorem

Full statement + proof: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`.

The theorem says that the gauge structure of the Standard Model falls out of the three shells:

| Shell | Gauge group | Matter content |
|---|---|---|
| 6 faces (SC, octahedron) | `U(1)` | hypercharge |
| 12 edges (FCC, cuboctahedron) | `SU(2)` | left-handed doublets |
| 8 corners (BCC, stella octangula) | `SU(3)` | color triplets |

And the three **generations** of matter (4 fermions each) come from the three shells having 4, 4, 4 "slots" under the gerade/ungerade parity split.

- 4 fermions/generation × 3 generations = 12 (leptons + quarks)
- 4 antifermions/generation × 3 generations = 12 (antileptons + antiquarks)
- Plus 3 dark sector states per generation = **+9**
- Total: `12 + 12 + 9 = 33 − ... = 17 dark matter states` ← the 17/27 dark matter fraction.

(See full accounting in the theorem doc; this cheatsheet is compressed.)

## Where the framework integers come from

| Integer | Polyhedron role |
|---|---|
| `N_c = 3` | 8 BCC corners / gerade-ungerade parity giving SU(3) triplet structure |
| `N_base = 4` | fermions per generation per shell |
| `b_3 = 7` | QCD β-coefficient at `N_f = 6` — `(11·3 − 2·6)/3` |
| `N_eff = 13` | Fibonacci `F_7` — shells × shells index |

## BCC multiplicative structure

The 8 BCC corners form a multiplicative group under lattice diagonals. The eigenvalue of the BCC stencil is a **triple cosine product**:

```
λ_BCC = 8 cos(k_x a/2) cos(k_y a/2) cos(k_z a/2)
```

This triple product is what simultaneously generates:

- the **Watson identity** `W₃ = G*² / (2π)` — at specific Brillouin-zone integrals
- the **SU(3) gauge group** — as the symmetry preserving the triple cosine
- the **lattice correction** to 1/α — via tadpole closure on the 2/3 spacing

Derivation: `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`.

## Phase lattice

The `{−1, 0, +1}` ternary state on each voxel combined with the 26 neighbors gives `3²⁷ = 7.6 × 10¹²` possible neighborhood configurations. The engine's lookup tables (when `phase_write` uses them) operate on compressed Hamming-weight classes of this space.

Reference: `docs/theory/08_structural/EXPLR_PHASE_LATTICE_MOORE.md`.

## Stencil map (for coding)

```
Face  (6):   (±1, 0, 0), (0, ±1, 0), (0, 0, ±1)           — SC
Edge (12):   (±1, ±1, 0), (±1, 0, ±1), (0, ±1, ±1)         — FCC
Corner (8):  (±1, ±1, ±1)                                  — BCC
```

```cpp
// Standard stencil in engine/include/ftd/moore.h
constexpr int MOORE_OFFSETS[26][3] = {
    // 6 face (SC)
    { 1, 0, 0}, {-1, 0, 0}, { 0, 1, 0}, { 0,-1, 0}, { 0, 0, 1}, { 0, 0,-1},
    // 12 edge (FCC)
    { 1, 1, 0}, { 1,-1, 0}, {-1, 1, 0}, {-1,-1, 0},
    { 1, 0, 1}, { 1, 0,-1}, {-1, 0, 1}, {-1, 0,-1},
    { 0, 1, 1}, { 0, 1,-1}, { 0,-1, 1}, { 0,-1,-1},
    // 8 corner (BCC)
    { 1, 1, 1}, { 1, 1,-1}, { 1,-1, 1}, { 1,-1,-1},
    {-1, 1, 1}, {-1, 1,-1}, {-1,-1, 1}, {-1,-1,-1},
};
```

## Cross-references

- Theorem: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`
- BCC unification: `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`
- Phase lattice: `docs/theory/08_structural/EXPLR_PHASE_LATTICE_MOORE.md`
- Scale 6 (Meta) visualization: `engine/web/js/scales/scale6/` — interactive 27-cell viewer
