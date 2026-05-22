# DERIV — Lattice Hodge Duality on FTD's Vertex-Centered Stencil

**Document type:** Derivation
**Status:** [DERIVED] — kinematic identity; standard math applied to FTD's specific stencil family
**Created:** 2026-04-30
**Provenance:** Q2 of the Maxwell-exploit thread opened with FTD-0113
**Related:** `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` (FTD's vertex-centered flux-field
ontology, G18 stencil family, longitudinal/transverse decomposition);
`DERIV_RETARDED_GREEN_LATTICE.md` (FTD-0113, parallel result on the
wave-equation side);
`scripts/proofs/proof_lattice_hodge_duality.py` (numerical sanity check)

---

## 0 · Statement

**Theorem (lattice Bianchi identities).** On the vertex-centered cubic
lattice with centered-difference operator

```
(∂_i f)(x) := [f(x + e_i) − f(x − e_i)] / 2,
```

the discrete exterior derivative `d` satisfies `d² = 0` identically.
Equivalently, for any vector field `A` and scalar field `φ` on the
lattice:

```
∇·(∇×A) = 0       (Bianchi I — no magnetic monopoles)        (★)
∇×(∇φ) = 0        (Bianchi II — gauge invariance compatible) (★★)
```

both hold **exactly at every lattice site**, with no `O(a²)`
discretization error.

**Stencil-independence corollary.** Identities (★)/(★★) depend only on
the difference operator being centered. They are **independent of the
choice of Laplacian stencil** — G6, G18, G26, or any element of the
isotropic family `(a, b, c)` with `a + 4b + 4c = 1` and `6b + 12c = 1`
preserves them. Therefore Hodge duality `F ↔ ⋆F` is preserved on FTD's
lattice for the entire G18-family of canonical stencils, including the
production engine choice `c = 0`.

---

## 1 · Three-line proof

For centered differences, the second mixed derivative is

```
(∂_i ∂_j f)(x) = (1/4) [
    f(x + e_i + e_j) − f(x + e_i − e_j)
  − f(x − e_i + e_j) + f(x − e_i − e_j) ].
```

Inspection of the four terms: the expression is **symmetric in (i, j)**
at every lattice site. So `∂_i ∂_j = ∂_j ∂_i` as operators.

Then:

```
∇·(∇×A) = ∂_i (ε_{ijk} ∂_j A_k) = ε_{ijk} ∂_i ∂_j A_k.
```

The Levi-Civita symbol `ε_{ijk}` is antisymmetric in `(i, j)`; the
operator `∂_i ∂_j` is symmetric in `(i, j)`. The contraction of an
antisymmetric tensor with a symmetric tensor over the antisymmetric
indices is identically zero. ∎

The same argument with `A_k → ∂_k φ` yields `∇×(∇φ) = ε_{ijk} ∂_j ∂_k φ
= 0`.

---

## 2 · What this means

### 2.1 · The lattice has the right algebraic backbone for Maxwell

Maxwell's equations decompose into two halves:

- **Bianchi (homogeneous) half**: `∇·B = 0` and `∇×E = −∂_t B`.
  These are *identities* — they say `F = dA` for some `A`.
- **Source (inhomogeneous) half**: `∇·E = ρ` and `∇×B − ∂_t E = J`.
  These are *equations of motion* sourced by `(ρ, J)`.

The Bianchi half is **automatic** as soon as the field strength is
written `F = dA` and `d² = 0`. The lattice version of this is identity
(★)/(★★) above. So FTD's lattice has the algebraic structure needed
for the "no magnetic monopoles" / "gauge invariance" half of Maxwell
to hold without any additional dynamics.

The source half is engine-dynamics-specific: it depends on which
Laplacian stencil is used (G6/G18/G26), on the time-step convention,
and on the source coupling. That half is the subject of the EFT
recovery program (Phase F/G/H), not this document.

### 2.2 · Decoupling of "exterior calculus" from "Laplacian stencil"

A non-obvious consequence: FTD's choice of canonical stencil G18
(c = 0) versus any other element of the isotropic family
(c ∈ [0, 1/12]) **does not affect** Hodge duality. The choice affects
the dispersion relation and finite-k isotropy of the Laplacian, but
the exterior calculus is preserved across the entire family.

This means that future stencil-change debates (G18 vs G26_iso_mid vs
G26_iso_corner, etc.) are *energy/dispersion* debates, not
*Maxwell-structure* debates. The Maxwell algebra is robust.

### 2.3 · Lattice Helmholtz decomposition is well-defined

The decomposition `J = J_L + J_T` with `∇·J_T = 0` requires that
divergence-free vector fields exist on the lattice — which requires
identity (★) (so that `J_T = ∇×A` is automatically divergence-free for
some auxiliary `A`). Without (★), the Helmholtz decomposition would
need a regularization at lattice scale; with (★), it is exact.

This is the algebraic reason FTD's native ED (`SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`
§3) can use `J = J_L + J_T` without introducing lattice-scale
correction terms.

### 2.4 · No hidden duality-breaking parameter

A potential worry was that FTD's stencil might break `F ↔ ⋆F` at
lattice scale, introducing a duality-breaking constant analogous to
the way some lattice gauge theories break chiral symmetry at lattice
scale. **There is no such parameter for FTD.** The Bianchi identities
are exact, so the duality is unbroken. This is recorded as a
*negative result* worth knowing — no anomalous lattice-scale
duality-breaking constant exists for the engine to measure.

---

## 3 · Comparison to Yee's scheme

The standard Yee scheme (1966) for lattice Maxwell places:
- `E` on edges (1-forms)
- `B` on faces (2-forms)
- `ρ` on vertices (0-forms)
- `J` on edges (1-forms)

This gives a clean discrete differential complex with `d² = 0`
automatic by geometric placement (the boundary of a boundary is
empty). Yee preserves Hodge duality by construction.

FTD differs: `J` lives at vertices (collocated with `ρ`), not at
edges. The natural lattice exterior derivative is the centered
difference `∂_i`, which satisfies `d² = 0` algebraically rather than
geometrically. The Maxwell algebra is the same; the placement
convention differs.

**Implication:** FTD's vertex-centered formulation is mathematically
equivalent to Yee's at the level of the Bianchi identities, even
though the geometric placement of fields differs. Either formulation
preserves Hodge duality. FTD's choice is a convention motivated by
the flux-field ontology (`J` is fundamental, not derived from `A`),
not a structural compromise.

---

## 4 · What this is NOT

- **NOT a derivation of Maxwell from FTD.** Identity (★)/(★★) is the
  *algebraic backbone* required by Maxwell's homogeneous half. The
  source half (`∇·E = ρ`, etc.) requires engine-dynamics-specific
  derivations that are not addressed here.

- **NOT a uniqueness claim.** Other lattice prescriptions (forward
  differences, backward differences, asymmetric stencils) would break
  `∂_i ∂_j` symmetry and hence break `d² = 0`. The result holds for
  centered differences specifically. FTD uses centered differences;
  the result applies to FTD.

- **NOT new mathematics.** The fact that `d² = 0` follows from
  symmetry of mixed centered differences is well-known in
  computational electromagnetics (e.g., Bossavit's lattice exterior
  calculus literature). What's new is the explicit statement for
  FTD's G18-family stencil and the observation that Hodge duality
  is independent of the Laplacian stencil choice within the family.

- **NOT a result about Lorentz invariance.** Hodge duality `F ↔ ⋆F`
  is a discrete-differential-geometry property; Lorentz invariance is
  a separate question about the relationship between space and time
  derivatives. Lorentz anisotropy at lattice scale is being audited
  separately (`AUDIT_LORENTZ_ANISOTROPY.md`).

- **NOT a new spine theorem.** Filed as FTD-0114 [DERIVED],
  subsidiary to FTD-0113 / Phase G / EFT-recovery program. The spine
  count is unchanged — nine numbered results, six theorem-grade +
  three honestly-tiered (see `SPEC_ALGEBRAIC_SPINE.md` §0).

---

## 5 · Open follow-ups

1. **Source-half consistency.** Identity (★)/(★★) handles Maxwell's
   homogeneous half. The inhomogeneous half (`∇·E = ρ`, `∇×B − ∂_t E
   = J`) requires the engine's gauss-projection + wave-propagation
   toggles to give the right relation between `J_L` and `ρ`. The
   gauss-projection step in the engine already enforces `∇·J_L = ρ`
   to high accuracy (Ward floor 1e-8 after Day-2 EFT campaign); a
   parallel audit for the wave-equation source half is the natural
   continuation.

2. **Engine cross-check.** Implement a CTest that:
   - Initializes a random vector field `A` on the lattice
   - Computes `B = ∇×A` and `ρ_check = ∇·B` using the engine's
     centered-difference operators
   - Asserts `‖ρ_check‖_∞ < 10⁻¹³` (machine precision)
   This should pass trivially given the proof, but it's a useful
   sanity check on the engine's actual implementation of `∇×` and
   `∇·`.

3. **Structural connection to Phase G.** Phase G's `α_r = 2r·G_L(r)`
   is purely geometric (Coulomb part, longitudinal `J_L`). The
   present result is purely algebraic (Bianchi part, transverse
   `J_T`). Together they cover both halves of Maxwell's *kinematic*
   structure on the lattice. Whether the *dynamical* structure
   (relation between `J_L` and `J_T` under the engine's tick cycle)
   admits a similarly clean characterization is open.

4. **Magnetic-monopole engine audit.** A non-trivial corollary of
   identity (★) is that the engine cannot host magnetic monopoles
   *kinematically* — any `B = ∇×A` is automatically divergence-free.
   This is a structural fact, but the engine has scenarios that
   inject "monopole-like" structures (e.g., the
   `s0-seed-monopole` scenario in FTD-0104). What those scenarios
   actually inject is a *flux configuration that approximates* a
   monopole over coarse-grained scales; the lattice itself cannot
   carry a true monopole. This is worth documenting in the scenario
   description.

---

## 6 · LEDGER status

This document files **FTD-0114** at the [DERIVED] tag, subsidiary to
FTD-0004 (Phase G, kinematic geometry side) and FTD-0113 (retarded
extension, wave-equation side). Together FTD-0004 + FTD-0113 + FTD-0114
constitute the algebraic-kinematic backbone of FTD's lattice
electrodynamics — covering static Coulomb, retarded radiation, and
Bianchi identities. The dynamical content (`α`, source coupling,
running) remains separate and is the subject of the EFT recovery
program.

---

## 7 · Verification

Numerical sanity check at L = 8 in `scripts/proofs/proof_lattice_hodge_duality.py`:

- Generates a random vector field `A` of shape `(L, L, L, 3)`
- Computes `B = ∇×A` via engine-style centered differences
- Computes `ρ_check = ∇·B`
- Asserts `‖ρ_check‖_∞ < 10⁻¹³` (machine precision)
- Repeats with `A → ∇φ` for random scalar `φ`, computing `∇×(∇φ)`
  and asserting `‖∇×(∇φ)‖_∞ < 10⁻¹³`

---

*End of derivation.*
