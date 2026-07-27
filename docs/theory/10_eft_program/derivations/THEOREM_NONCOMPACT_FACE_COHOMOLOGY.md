# FTD-0583 — Noncompact Matched-Face Cohomology

**Status:** `[THEOREM — PERIODIC REAL COCHAIN COHOMOLOGY]` +
`[THEOREM — LOCAL ZERO-HARMONIC CONTRACTIBILITY]` +
`[CLOSED NEGATIVE — LOCALIZED PROTECTED CARRIER IN CURRENT NONCOMPACT FACE VARIABLES]` +
`[OPEN — NONLINEAR DEFORMING CORE OR NEW COMPACT/SINGULAR VARIABLE]`  
**Date:** 2026-07-26  
**Verdict:**
`MATCHED_NONCOMPACT_COHOMOLOGY_GLOBAL_ONLY_LOCAL_PROTECTED_DEFECT_CLOSED`

## 1. Scope

The matched face/edge sidecar uses ordinary real cochains on a periodic cubic
lattice. FTD-0583 classifies that complex before any compact connection,
branch integer, singularity, or nonlinear carrier is introduced. The result
is a theorem about those frozen variables, not a theorem that localized
topological matter is impossible in every discrete ontology.

## 2. Matched periodic complex

Let `d_0` be the backward site-to-edge difference, `C` the existing
`matched_curl`, and `D` the existing backward face divergence. They form

\[
0\longrightarrow C^0(T_L^3;\mathbb R)
 \xrightarrow{d_0}C^1(T_L^3;\mathbb R)
 \xrightarrow{C}C^2(T_L^3;\mathbb R)
 \xrightarrow{D}C^3(T_L^3;\mathbb R)
 \longrightarrow0 .
\]

The component formulas give `C d_0=0` and `D C=0` by telescoping backward
differences. At Fourier momentum `k`, define

\[
q_i=1-e^{-ik_i}.
\]

The three symbols are the Koszul maps

\[
d_0(k)\phi=q\phi,\qquad C(k)A=q\times A,
\qquad D(k)E=q\cdot E.
\]

For `k != 0`, at least one component of `q` is nonzero. Consequently

\[
\operatorname{rank}d_0=1,\qquad
\operatorname{rank}C=2,\qquad
\operatorname{rank}D=1,
\]

and the sequence is exact. At `k=0`, all three symbols vanish. Thus only the
zero Fourier mode contributes cohomology and

\[
\boxed{(b_0,b_1,b_2,b_3)=(1,3,3,1)},
\qquad
\boxed{H^2(T_L^3;\mathbb R)\cong\mathbb R^3}.
\]

The native observer independently row-reduces all 728 registered symbols; it
does not insert these ranks or Betti dimensions as constants.

## 3. Meaning of the three face classes

The three `H^2` coordinates are constant face fluxes through the three
noncontractible coordinate two-tori. For a divergence-free field, the flux
through parallel planes is independent of the plane. A curl has zero flux
through every such plane, so local curl updates cannot change these three
numbers.

These classes are global and real-valued. Nothing in `MatchedFaceFlux`
identifies values modulo a period, supplies transition functions, or restricts
the plane flux to an integer lattice. Therefore this `R^3` is not the integer
first-Chern data of compact `U(1)`.

## 4. Local contraction theorem

Let `E` be divergence-free with all three harmonic plane fluxes zero. Fourier
exactness gives `E=C A`. Then

\[
E_t=tE=C(tA),\qquad 0\le t\le1
\]

stays in the same zero-Gauss, zero-harmonic sector and joins `E` continuously
to the vacuum. With the quadratic matched-field energy,

\[
U(E_t)=t^2U(E).
\]

For a localized curl representative the support need not grow during this
contraction. Hence the present linear field space has neither a nonzero local
topological floor nor a disconnected localized sector.

## 5. Gauss charge is not field topology

Periodicity gives

\[
\sum_x D E(x)=0.
\]

A seeded source/sink dipole satisfies Gauss exactly, but replacing `E` by
`tE` replaces its source values by `tq` and `-tq`. The field configuration and
its Gauss charges therefore scale continuously to zero. A ternary source
label `s=+/-1` may still be an exact primitive label; it does not convert the
responding real face field into a quantized topological class.

## 6. Consequence

The current noncompact face/edge variables cannot, by themselves, provide the
localized protected defect or bundle needed to evade the FTD-0580 Peierls
pinning and FTD-0582 one-way-coupling closures. Global plane-flux sectors do
exist, but they are delocalized boundary data and continuously valued.

The surviving branches require genuinely different mathematics: a nonlinear
deforming `(s,J,W)` core, a compact link variable with explicit periodicity,
a singular/branched connection, a boundary-supported sector, or another new
primitive. None is licensed or implemented here. In particular, this result
does not license FTD-0481, charge quantization, compact `U(1)`, a particle
claim, a scenario, or Lorentz recovery.

The locked preregistration SHA-256 is
`755D703FB3E9DA9CA7F2EB46B1FE399D704F739AD08050D39242D1EB0B2BB922`.
