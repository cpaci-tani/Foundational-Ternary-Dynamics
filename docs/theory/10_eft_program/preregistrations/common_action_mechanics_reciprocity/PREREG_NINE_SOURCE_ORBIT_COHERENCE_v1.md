# PRE-REGISTRATION — Nine-source removal-time orbit coherence v1

**Date locked:** 2026-07-26  
**Identifier:** `FTD-0592`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-EVALUATION]`  
**Parent:** `FTD-0591`  

## 1. Question

Does the already-derived FTD-0590 cubic-orbit coherence inequality prove that
every reaction-free history of nine distinct stationary native sources stays
strictly below the production genesis threshold before a candidate first
descendant event?

This protocol evaluates one new integer count, `N=9`. It does not alter the
FTD-0590 theorem, optimize a source geometry, search polarities or schedules,
or introduce a stronger relaxation after seeing the result.

## 2. Frozen sector

Retain exactly the FTD-0590 sector:

- production 18-point wave stencil and native state-gradient source;
- zero initial `J`, wave velocity, and manifested velocity;
- one periodic odd quotient with `L in {9,17,33,65}`;
- nine distinct stationary ternary sources at arbitrary sites and with
  arbitrary signs;
- every source begins present and may be removed once at an arbitrary integer
  tick;
- no Gauss projection, damping, force, movement, transmutation, collision,
  clock, bath, toggle, scenario, or production modification.

The proof domain ends immediately before a hypothetical first descendant
genesis event.

## 3. Frozen bound

For each registered volume, use the FTD-0590 values computed from exhaustive
mode and nonzero-displacement cubic orbits:

\[
Q_L=\frac{G_C}{C_{\rm WAVE}^2L^3}\sqrt{A_LW_L},
\qquad
\mu_L=\max_{d\ne0}\frac1{W_L}
\sum_Ow_O|\chi_O(d)|,
\]

and the FTD-0588 common-step coefficient `C_L`. Evaluate, without changing
any coefficient,

\[
H_L^{\rm orb}(9,r)=
C_L\sqrt{9-r}
+Q_L\sqrt{r+\mu_Lr(r-1)},
\qquad r=0,1,\ldots,9.
\]

The reported volume bound is the exhaustive maximum over these ten removal
partitions. Ties are resolved by the smallest `r` only for deterministic
reporting; the value is unchanged.

No grouping by shared stencil eigenvalue `M`, exact schedule phase, source
geometry, polarity, observation time, or observation site is permitted in
FTD-0592. Any sharper bound requires a separately locked identifier.

## 4. Registered constants and tolerances

- `G_C = 0.0854245431028543695`;
- `C_WAVE^2 = 1/3`;
- `K_GENESIS = 1.5163860591519780`;
- orbit-invariance residual `<=5e-14`;
- direct-character residual `<=5e-13`;
- independent C++/Python scalar agreement `<=5e-12` absolute;
- exact orbit coverage and exact mode count `L^3-1` on every volume.

All spectral sums use compensated summation. Integer phases are reduced
modulo `L` before direct-character evaluation.

## 5. Required artifacts

- `engine/include/ftd/eft/nine_source_orbit_coherence.h`;
- `engine/src/eft/nine_source_orbit_coherence.cpp`;
- `engine/tests/test_nine_source_orbit_coherence.cpp`;
- `scripts/proofs/proof_nine_source_orbit_coherence.py`;
- `engine/results/ftd_0592/windows_msvc_cpu.json`;
- `engine/results/ftd_0592/windows_msvc_cpu.csv`;
- theorem, analysis, and adversarial audit records.

The C++ implementation must recompute the orbit quantities rather than copy
the FTD-0590 or FTD-0591 JSON. The Python verifier must independently
reconstruct the orbits and compare every reported scalar.

## 6. Outcome map

The protocol is valid only if every algebraic, coverage, character, finiteness,
and cross-language gate passes.

- If `max_r H_L^orb(9,r) < K_GENESIS` on all four volumes, verdict:
  `ARBITRARY_REMOVAL_N_LE_9_CLOSED_BY_ORBIT_COHERENCE`.
- If all validity gates pass but at least one registered volume is not strictly
  subcritical, verdict:
  `NINE_SOURCE_ORBIT_BOUND_INCONCLUSIVE`.
- If any validity or cross-language gate fails, verdict:
  `PROTOCOL_INVALID`.

An inconclusive bound is not a positive genesis mechanism. A closed result is
only a first-event impossibility theorem in the frozen sector. Neither result
licenses reciprocal motion, a particle claim, a toggle, a scenario, or a
change to production dynamics.

## 7. Failure consequence

No tolerance, volume, coefficient, or relaxation is changed after evaluation.
If the orbit bound is inconclusive, the only admissible next mathematical
branch is a new preregistration for the exact shared-`M` eigenshell structure
already identified in FTD-0590.
