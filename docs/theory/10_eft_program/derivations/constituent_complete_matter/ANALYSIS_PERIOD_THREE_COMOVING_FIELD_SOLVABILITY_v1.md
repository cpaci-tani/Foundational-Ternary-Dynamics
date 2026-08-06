# FTD-0716 — Period-three co-moving-field solvability v1

**Status:** `[NUMERICAL FACT — REGULAR FINITE-VOLUME FIELD SOLUTION]`  
**Verdict:** `PERIOD_THREE_COMOVING_FIELD_SOLUTION_REGULAR`  
**Production status:** unchanged

## Result

The exact FTD-0715 three-phase trajectory deposits 48 quadratic-coat face
currents with continuity residual `3.47e-16` and zero causal excess. Unlike the
rigid two-tick source, this source lies in the range of the complete translated
three-tick field operator

\[
A_3(k)=e^{ik_x}U(k)^3-I
\]

at every `L=33` momentum.

The full `215622`-degree minimum-norm solution has

```text
spectral residual max              1.0049244505810427e-14
real-space residual max            4.534890179545463e-16
left-null source projection L2     4.2384114701053835e-17
incompatible mode count            0
minimum source-active singular     6.014989198681775e-5
solution amplification             19.584116289345392
maximum solution component         0.6141525303952011
reality residual                   3.4376096299926273e-16
Parseval residual                  4.4304411732074777e-16
```

The operator retains `2182` structural zero singular values, but the source is
orthogonal to all of them. The result is therefore a compatibility statement,
not a claim that the field operator is nonsingular.

## Contrast with rigid motion

FTD-0711 found eight body-diagonal modes with nonzero null projection for the
rigid two-tick current. FTD-0716 changes neither the matched field operator nor
the face-current definition. It changes the source history by allowing the
composite's internal phase to cycle. That change removes the complete
nullspace obstruction.

The correct inference is:

> A co-moving dressing is compatible with recurrent internal motion even
> though it is incompatible with rigid transport of the same localized rest
> object.

This is the first complete finite-volume matter-trajectory plus field-return
candidate in this branch. It does not yet show that the field causes the
trajectory.

**Successor note (FTD-0717):** absolute Gauss matching against the quadratic
constituent density passes at all four phases with maximum residual
`1.89e-15`. The translated solution is therefore Gauss-realizable. The same
successor rejects its independently selected minimum-norm representative as a
common-action solution because per-tick recoil and kinetic-plus-field energy
do not match.

## Remaining common-action burden

The solution was selected by minimum field norm and source compatibility. The
matter momenta were selected independently by the discrete-gradient velocity
lift. A native mobile object requires one atomic relation to produce both.
Specifically, the field must now:

1. supply the opposite of the FTD-0715 tick matter impulses;
2. exchange exactly the telescoping matter kinetic energy;
3. generate, rather than merely accommodate, all three constituent segments;
4. preserve Gauss, locality, cubic covariance, and the state-only inverse;
5. return the full matter-plus-field state translated by one site.

No counterterm, impulse rescaling, homogeneous-mode fitting, or post-hoc force
is licensed by this result.

## Provenance

- protocol: `5F74489C...41BE19`
- source summary: `74C74D60...AB4923`
- source: `A56F3776...169825`
- solution summary: `C71302BE...A4619C`
- modes: `1742E805...44CFE`
- correction: `020AD31A...7C38E8`
- C++ source observer: `C6724C1E...C34FE4`
- Fourier proof: `1174A6CC...74B0`
