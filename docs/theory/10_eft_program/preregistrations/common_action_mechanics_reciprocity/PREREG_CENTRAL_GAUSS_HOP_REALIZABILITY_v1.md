# PRE-REGISTRATION — Central-Gauss hop realizability v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0471`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0427`, `FTD-0443`, `FTD-0470`  
**Engine artifact:** `engine/tests/campaign_central_gauss_hop_realizability.cpp`  
**Campaign SHA256:** `D6B917E3110AA9C5295BEC30261D465D364CD6258375E443345D38D5A40D88CC`  
**Helper SHA256:** `971C601EE7355670F519D0499FDEB37D1D980FE76F7D2C0F920E3E2C7A071DE2`

## 1. Question

FTD-0470 established that exact face-hop work belongs to an oriented link.
Before testing link recoil, this campaign asks whether the production
cell-centered field with central divergence can represent the Gauss-source
change of one adjacent site hop locally at all.

The comparison is with the already selected matched face/backward-difference
sidecar of FTD-0427. No production operator or tick phase is changed.

## 2. Pre-derived graph theorem

For a cell-centered component `J_i(m)`, central divergence contributes

```text
(D_i J_i)(x) = [J_i(x+e_i)-J_i(x-e_i)]/2.
```

Thus `J_i(m)` is an oriented edge between source sites `m+e_i` and `m-e_i`:
the source graph moves by two lattice sites per edge.

- **G1 (even-L obstruction):** for even periodic `L`, each checkerboard
  character `chi_i(x)=(-1)^{x_i}` is in the left nullspace of `D_i`, hence
  of the full central divergence under the corresponding pairing. An adjacent
  hop changes `chi_i` pairing by magnitude `2`, so its source delta is outside
  the divergence image. No global solution exists.
- **G2 (odd-L lower bound):** for odd periodic `L`, step two generates the
  axial cycle. Reaching `a+e_i` from `a` requires `(L-1)/2` step-two edges,
  since `-2[(L-1)/2]=1 mod L`. Therefore any axial central-divergence
  realization has support at least `(L-1)/2`. The registered construction
  saturates this bound with component value `2q` (signed by direction).
- **G3 (matched face locality):** backward face divergence represents the
  same hop by subtracting the one oriented transport-current face. Continuity
  and the Gauss-source delta close on support one for even and odd `L`.

These are exact finite-graph statements. Coupling by `G_C` rescales both sides
and does not affect realizability or support.

## 3. Frozen fixtures

1. **Even central field:** `L={16,32}`, three axes, both directions and
   polarities. Record the desired parity pairing and the pairing of central
   divergence for a deterministic nonzero field. Total 24 hop rows plus six
   null-field pairings.
2. **Odd central field:** `L={17,33,65}`, three axes, both directions and
   polarities. Construct the shortest step-two path and measure exact Gauss
   residual and support. Total 36 rows.
3. **Matched face field:** `L={16,17,32,33,65}`, three axes, both directions
   and polarities. Route one face current, apply `E<-E-current`, and measure
   continuity, Gauss delta, current support, and field-update support. Total
   60 rows.

## 4. Gates

- valid finite fixtures in all rows;
- even-L desired parity-pairing magnitude exactly `2`, arbitrary-field central
  divergence pairing `<=1e-12`, and `realizable=false`;
- odd-L endpoint reached, Gauss residual `<=1e-12`, graph steps and nonzero
  support both exactly `(L-1)/2`;
- matched-face continuity and Gauss residuals `<=1e-12`, with exactly one
  current component and one changed face component.

## 5. Outcome map

- all gates pass:
  `CENTRAL_GAUSS_HOP_EVEN_IMPOSSIBLE_ODD_NONLOCAL_FACE_LOCAL`;
- any theorem/construction gate fails with valid fixtures:
  `CENTRAL_GAUSS_HOP_REALIZABILITY_CLAIM_FAILS`;
- invalid fixture: `PROTOCOL_INVALID`.

A matched-face pass does not prove physical charge, gauge invariance, recoil,
or a production replacement. It establishes the minimal representation needed
for local exact source transport under the declared finite-volume complex.

## 6. Run of record

Pinned MSVC `14.44.35207`, Release, CPU observer, focused target
`campaign_central_gauss_hop_realizability`, output
`engine/results/ftd_0471/windows_msvc_cpu.csv`.

**Recorded outcome:**
`CENTRAL_GAUSS_HOP_EVEN_IMPOSSIBLE_ODD_NONLOCAL_FACE_LOCAL`. Even-L null
pairings closed below `3.65e-16`; odd-L exact support was `8/16/32` at
`L=17/33/65`; all matched-face continuity and Gauss residuals were zero on
support one. See `AUDIT_CENTRAL_GAUSS_HOP_REALIZABILITY.md`.
