# AUDIT — Quadratic-coat external anchor: the B-spline de Rham complex

**Date:** 2026-07-26
**Identifier:** `FTD-0568`
**Status:** `[SYNTHESIS — EXTERNAL-LITERATURE RECONCILIATION] + [EXTERNAL — proven (tensor B-spline de Rham complex / FEEC)] + [NUMERICAL FACT — EXACT-RATIONAL CONVENTION CHECK]`
**Verdict:** `QUADRATIC_COAT_IS_STANDARD_SPLINE_DE_RHAM_INSTANCE`
**Verification:** `scripts/proofs/proof_coat_external_anchor.py` (4/4 PASS)
**Import ledger:** `SPEC_IMPORT_LEDGER.md` §3.3 row IMP-C5 (ledger rev v1.3)
**Provenance:** flagged 2026-07-26 during the FTD-0565/0566 session; verified same day against the external literature.

## 1. Claim

The FTD-0541–0551 quadratic-coat arc independently re-derived, in FTD
vocabulary and under locked pre-registered campaigns, the representation
layer of a standard compatible-discretization framework: the tensor-product
B-spline de Rham complex of isogeometric analysis, situated inside finite
element exterior calculus, together with the charge-conserving spline
current deposition of the particle-in-cell literature. The arc's structural
identities are instances of published theorems. Its `[SELECTION]` rows
therefore price the choice of a known external framework, not FTD-native
machinery; the in-house proofs stand on their own and are now doubly
anchored.

## 2. The identification

| FTD object | external object | source |
|---|---|---|
| charge coat `B2⊗B2⊗B2` (FTD-0541) | 0-form spline space `S_{2,2,2}` at uniform periodic knots; ρ treated as a 0-form | BRSV 2011; GEMPIC eq. (3.39a) and §3.1 |
| face current / E family `(B1,B2,B2)`, own direction staggered at half-integers (FTD-0541; FTD-0550 eq. 1) | 1-form space `S_{1,2,2}×S_{2,1,2}×S_{2,2,1}` (spline Nédélec analog, `p=2`); J treated as a 1-form | BSV 2010; BRSV 2011; GEMPIC eq. (3.39b) |
| edge B family `(B2,B1,B1)` (FTD-0550 eq. 5) | 2-form space `S_{2,1,1}×S_{1,2,1}×S_{1,1,2}` (spline Raviart–Thomas analog, `p=2`) | BRSV 2011; GEMPIC eq. (3.39c) |
| `B2'(u) = B1(u+1/2) − B1(u−1/2)` (FTD-0541 "matched `B2'` identity"; FTD-0550 eq. 6) | the B-spline derivative recurrence `d/dx N_j^p = D_j^p − D_{j+1}^p`, recentred to cardinal splines | de Boor spline calculus; GEMPIC eqs. (3.34)–(3.37) |
| `interp(C^T A) = curl(interp(A))` (FTD-0550 eqs. 7–8) | the complex's coefficient-level curl: curl maps 1-form splines onto 2-form splines with incidence-matrix coefficients — the commuting-diagram property | AFW 2006 (framework); BSV 2010 / BRSV 2011 (commutative spline de Rham diagram with projectors); GEMPIC eqs. (3.44)–(3.45), (3.47) |
| curl-adjoint cancellation `⟨Ē,CB̄⟩ = ⟨B̄,CᵀĒ⟩` and `ΔU_field = −⟨Ē,K⟩` (FTD-0544) | the compatible primal/dual transpose pair; the antisymmetric field block of the discretized Hamiltonian structure; the midpoint step preserving the quadratic invariant exactly | GEMPIC eqs. (3.29)–(3.30) (primal/dual sequences with `Cᵀ`), (4.27) (antisymmetric Poisson matrix), (4.42) (quadratic field energy); Yee lineage noted at GEMPIC §3.1 |
| `div C = 0` Gauss transport `ρ1−ρ0+div K = 0` (FTD-0544 eqs. 3–4) | the complex property `Im C ⊆ Ker D` / discrete continuity `dϱ/dt + Gᵀj = 0` | GEMPIC eqs. (3.29), (4.51) |
| exact straight-worldline deposit closing continuity (FTD-0541; spacetime split FTD-0542) | charge-conserving current deposition with B-spline form factors | Villasenor–Buneman 1992; Esirkepov 2001; GEMPIC eq. (4.52) |

A vocabulary caveat prevents a false-mismatch reading: FTD's "face" fields
carry E and its "edge" fields carry B, which is the swapped labeling
relative to Yee/IGA usage (E on edges as a 1-form, B on faces as a 2-form).
The spaces, staggerings, and identities are identical under the relabeling;
only the lattice-picture names differ.

## 3. External results verified

1. **A. Buffa, G. Sangalli, R. Vázquez**, *Isogeometric analysis in
   electromagnetics: B-splines approximation*, Comput. Methods Appl. Mech.
   Engrg. **199** (17–20), 1143–1152, 2010. B-spline spaces for Maxwell,
   the spline de Rham diagram. (Bibliographic record verified.)
2. **A. Buffa, J. Rivas, G. Sangalli, R. Vázquez**, *Isogeometric discrete
   differential forms in three dimensions*, SIAM J. Numer. Anal. **49** (2),
   818–844, 2011. The 3D mixed-degree spline spaces of discrete
   differential forms; projectors rendering the de Rham diagram
   commutative; Maxwell source and eigenproblems. (Bibliographic record and
   result summary verified.)
3. **D. N. Arnold, R. S. Falk, R. Winther**, *Finite element exterior
   calculus, homological techniques, and applications*, Acta Numerica
   **15**, 1–155, 2006. The FEEC framework: subcomplexes of the de Rham
   complex with commuting projections — the commuting diagram is the
   framework's first building block. (Existence verified; the framing is
   quoted as such by GEMPIC §3.2, read directly.)
4. **M. Kraus, K. Kormann, P. J. Morrison, E. Sonnendrücker**, *GEMPIC:
   geometric electromagnetic particle-in-cell methods*, J. Plasma Phys.
   **83** (4), 2017 (arXiv:1609.03053). Read directly (pp. 10–27 of the
   PDF of record): the spline complex eq. (3.39a–d); the derivative
   recurrence (3.34)–(3.37); the coefficient-level curl (3.44)–(3.47); the
   primal/dual transpose sequences (3.29)–(3.30); the antisymmetric Poisson
   matrix (4.27) with quadratic field energy (4.42); exact discrete
   continuity and Gauss conservation (4.46)–(4.52).
5. **J. Villasenor, O. Buneman**, *Rigorous charge conservation for local
   electromagnetic field solvers*, Comput. Phys. Commun., 1992, and
   **T. Zh. Esirkepov**, *Exact charge conservation scheme for
   particle-in-cell simulation with an arbitrary form-factor*, Comput.
   Phys. Commun., 2001. Charge-conserving deposition satisfying the
   discrete continuity equation on the grid, B-spline form factors of
   arbitrary order. (Titles and role verified.)

## 4. Convention reconciliation (computed, exact rationals)

`scripts/proofs/proof_coat_external_anchor.py`, 4/4 PASS:

- **A1** — `B2'(u) = B1(u+1/2) − B1(u−1/2)`: both sides piecewise linear
  with breakpoints in `{±1/2, ±3/2}`; verified at ≥2 interior rational
  points per piece (a per-piece proof for linear pieces). This is the
  knot-indexed recurrence `d/dx N_j^p = D_j^p − D_{j+1}^p` under the
  centered-cardinal shift `B_p(x) = N_0^p(x + (p+1)/2)`.
- **A2** — B2 integer-offset weights `(1/8, 3/4, 1/8)` plus partition of
  unity: FTD-0540's cardinality-loss values are the standard
  quadratic-spline values.
- **A3** — 1D commutation `d/dx Σᵢ aᵢB2(x−i) = Σᵢ (a_{i+1}−aᵢ)B1(x−i−1/2)`,
  symbolic in all seven periodic coefficients: the coefficient is the
  **forward** difference `a_{i+1} − aᵢ`, matching FTD-0550 eq. (7)'s
  `A_z[i,j+1,k] − A_z[i,j,k]` sign convention exactly.
- **A4** — assembled 3D wiring on a periodic `N=4` lattice in exact
  `Fraction` arithmetic: `(curl A)_x` of the face/1-form interpolant equals
  the edge/2-form interpolant of the FTD-0550 eq. (7) incidence
  coefficients at 30 sample points, using the analytic per-piece derivative
  of B2 (independent of A1, so the check is not circular).

## 5. Scope and pricing

The anchor covers the **representation layer**: the four spline families,
the staggered complex, the worldline deposit, the field gathers, the
commuting curl, the curl-transpose adjointness, discrete continuity, and
the midpoint Poynting identity. For that layer the external literature
additionally supplies, as available (unconsumed) imports, the `p`-general
complex at arbitrary degree, its exactness and commuting-projector
theorems, approximation and stability theory, and the Hamiltonian
splitting-integrator assembly. None of that is consumed by the arc; nothing
is licensed by this reconciliation.

The arc's dynamics gates remain FTD-specific and stand exactly as tagged:
the fixed-step energy scope (FTD-0543), the matter-work nonidentity and its
self-consistent successor (FTD-0545/0546), the accelerated-worldline branch
(FTD-0547/0548), schedule underdetermination (FTD-0549), the
discrete-gradient transaction (FTD-0551), and the self-force fork
(FTD-0552–0555, FTD-0565). The external literature does not adjudicate
them. One contextual connection is on record: energy-conserving PIC schemes
are known to exhibit spurious grid self-forces originating in the
deposition's residual non-smoothness (Langdon's classic analyses; *The
Energy Conserving Particle-in-Cell Method*, arXiv:1108.1959; *Finite
spatial-grid effects in energy-conserving particle-in-cell algorithms*,
Comput. Phys. Commun., 2020), so the coat's conservative Peierls-like
self-potential (FTD-0552) is the FTD instance of a known artifact class —
context for the fork's pricing, not a closure of it.

Pricing: the import ledger's §3.3 gains named-result row **IMP-C5**
(proven-external, reconciliation anchor). The effect is a repricing of
provenance, not of tags: FTD-0541 and FTD-0550's `[SELECTION]` components
now read as the selection of standard external machinery rather than as
FTD-native constructions, which lowers the arc's novelty surface and raises
its external verifiability simultaneously. No tag moves in either
direction; the `[THEOREM]` components keep their in-house proofs.

## 6. Falsifier

A coat identity exhibited with no counterpart in the cited complex, or a
structural mismatch beyond relabeling in the space/degree correspondence
(the `p=2` mixed-degree pattern, the half-integer staggering, the incidence
coefficients), retracts the anchor to partial or void; the coat rows' own
tags are unaffected in that event.

## 7. Reproducibility

- verification: `python scripts/proofs/proof_coat_external_anchor.py`,
  4/4 PASS, wall ≈ 2 s, sympy + exact `Fraction` arithmetic, deterministic
  seed;
- external sources: bibliographic records via web search of record
  2026-07-26; GEMPIC equations read directly from the arXiv:1609.03053 PDF;
- companion edits: `SPEC_IMPORT_LEDGER.md` + `import_ledger.json` rev v1.3
  (IMP-C5), LEDGER row FTD-0568, `META_INDEX.md` row 7.210.
