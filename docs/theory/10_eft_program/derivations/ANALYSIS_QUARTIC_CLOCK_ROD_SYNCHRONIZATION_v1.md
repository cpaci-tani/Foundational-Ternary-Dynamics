# FTD-0771 — Quartic Clock--Rod Synchronization Boundary v1

**Status:** `[THEOREM — CONDITIONAL COORDINATE-EDGE/CLOCK RATIO WITHIN THE IMPOSED QUARTIC HAMILTONIAN]` +
`[CLOSED NEGATIVE — UNIQUE SYNCHRONIZATION FROM P1--P5 CONSERVATIVE EXTENSIONS]` +
`[CLOSED NEGATIVE — DIRECT G* SIGNATURE IN THE FTD-0770 SELECTED LINEAR COMMON-CONE CONTROL]` +
`[OPEN — NATIVE RATE, SHELL, AND CLOCK SELECTION]`  
**Date:** 2026-08-02  
**Protocol:**
[`PREREG_QUARTIC_CLOCK_ROD_SYNCHRONIZATION_v1.md`](../preregistrations/PREREG_QUARTIC_CLOCK_ROD_SYNCHRONIZATION_v1.md),
SHA-256
`360BAC51AC50F525DD4AF6DCD588F61831F13778C38FF5644854E2D35817FE16`  
**Production status:** unchanged

## 1. Result

Coupling the quartic period to one causal lattice interval does define a clean
dimensionless observable. Here “rod” means an abstract coordinate edge and
topological interval, not an operational material measuring rod. For clock
rate matching `rho`, base-shell energy `E`, axial lattice interval `ell`, and
registered substrate speed `C_sub`, the support-cone result is

```text
d_4 := (ell/C_sub)/T_(4,rho)(E)
     = rho (2E)^(1/4)/(sqrt(pi)G*)                 (1)
```

when `C_sub=ell/tau` and the Hamiltonian flow is parameterized relative to the
primitive tick `tau`. The corresponding phase advance is

```text
chi_4 = 2pi d_4
      = 2sqrt(pi) rho (2E)^(1/4)/G*.              (2)
```

More generally, set `u=C_sub tau/ell`, the registered speed in axial edges
per primitive tick. Then both formulas acquire the factor `1/u`:

```text
d_4 = (rho/u) (2E)^(1/4)/(sqrt(pi)G*),
chi_4 = (rho/u) 2sqrt(pi)(2E)^(1/4)/G*.           (2a)
```

Thus the proposed coordinate-edge/clock comparison succeeds as an exact
**conditional** `G*`-bearing ratio. At `rho=1` and on the amplitude-one shell
`2E=1`,

```text
d_4 = 1/(sqrt(pi)G*),
chi_4 = 2sqrt(pi)/G*.                              (3)
```

However, neither `rho=1` nor `E=1/2` follows from FTD Postulates 1--5. A
one-parameter family of equally deterministic and equally local on-site
quartic extensions changes `rho` and therefore changes (1). Initial data can
also occupy any positive shell. Consequently, (3) is not a derived minimum
dimensionless time step of the current FTD substrate.

The exact verdicts are

```text
CLOCK_ROD_RATIO_CONDITIONAL_GSTAR_PRESENT
P1_P5_SYNCHRONIZATION_UNDERDETERMINED
COMMON_CONE_GSTAR_CANCELLATION
```

## 2. Premise ledger

The construction combines structures with different epistemic status:

| ingredient | status at this result |
|---|---|
| abstract cubic adjacency interval `ell` | `[AXIOM]` as a coordinate edge, not a length in metres |
| primitive integer tick `tau` | `[AXIOM]` |
| Moore support bound `C_MOORE=ell/tau` | consequence of P4 in the registered `L-infinity` coordinate convention |
| production `C_SPEED=ell/(sqrt(3)tau)` | `[SELECTED]`; not the topological support bound and not a uniqueness theorem |
| physical SI value of one voxel | gauge/calibration dependent; excluded from Axiom-Zero derivation by FTD-0059 |
| quartic oscillator `(p^2+q^4)/2` | `[IMPOSED / SELECTED CANDIDATE]` per FTD-0770 |
| relative clock rate `rho` | `[IMPOSED]` until a substrate matching law fixes it |
| occupied shell `E` | initial-condition input; amplitude-one `E=1/2` is a normalization, not native selection |
| identification of Hamiltonian time with substrate ticks | `[OPEN]` dynamically; represented here by `rho` |

The raw edge is enough to define a dimensionless coordinate-edge reference,
but it does not derive a physical minimum length or construct a material rod.
FTD-0137 explicitly treats `a_phys` as gauge. This result is independent of
that SI calibration because `ell` cancels.

## 3. Exact period derivation

For even `m>=2`, define

```text
h_m(q,p)=(p^2+q^m)/2=E.
```

The positive turning point is `A=(2E)^(1/m)`. Since `dot(q)=p` for the base
Hamiltonian, four times the quarter orbit gives

```text
T_m^(0)(E)
 = 4 integral_0^A dq/sqrt(2E-q^m)
 = [4/m] B(1/m,1/2) (2E)^(1/m-1/2)
 = K_m (2E)^(-(m-2)/(2m)),                        (4)

K_m = [4/m] B(1/m,1/2).
```

Parameterize the clock relative to the primitive tick by

```text
H_(m,rho)=(rho/tau)h_m.
```

Multiplying a Hamiltonian by `rho/tau` multiplies its vector field by the same
factor, so

```text
T_(m,rho)(E)/tau
 = K_m/[rho (2E)^((m-2)/(2m))].                   (5)
```

For `m=4`,

```text
K_4 = B(1/4,1/2)
    = Gamma(1/4)Gamma(1/2)/Gamma(3/4)
    = sqrt(pi)G*,                                 (6)
```

which yields (1)--(3).

The coefficient normalization is also load-bearing. For the more general

```text
H=p^2/(2M)+Lambda q^4/2,
```

the same quadrature gives

```text
T(E)(2E)^(1/4)
 = sqrt(pi)G* M^(1/2)Lambda^(-1/4).                (6a)
```

The familiar bare `sqrt(pi)G*` therefore already assumes `M=Lambda=1` in the
canonical chart. P1--P5 select neither coefficient. The relative matching
parameter `rho` makes this missing clock normalization explicit rather than
hiding it in the Hamiltonian units.

## 4. What the minimum distance contributes

With the P4 topological support bound,

```text
C_MOORE=ell/tau,
tau_rod=ell/C_MOORE=tau.                          (7)
```

Therefore the edge contributes a coordinate-edge/clock normalization but no
additional numerical constant: `ell/C_MOORE=tau` reconstructs the
already-postulated primitive tick algebraically. It does not establish that a
realized signal traverses one axial edge in one tick. The meaningful
observable is not a solver step. It is

```text
minimum nonzero substrate interval / local clock period.
```

Calling it `d_4` avoids conflating three different objects:

1. the primitive one-tick interval;
2. the phase advance of a selected continuous auxiliary clock during that
   interval; and
3. the arbitrarily refinable timestep used by a numerical integrator.

The coordinate edge is primitive adjacency under P1, the minimum nonzero time
index is one tick under P2, and their topological support relation is P4. The
second item above is the observable in (1); the third has no ontological
significance.

## 5. Invariance is not selection

Under a common change of time coordinate `t'=s t`,

```text
T'=sT,       C_sub'=C_sub/s,       Omega'=Omega/s.
```

Hence

```text
(ell/C_sub')/T'=(ell/C_sub)/T,
Omega' ell/C_sub'=Omega ell/C_sub.                (8)
```

The ratio is therefore coordinate invariant once both structures use one
time coordinate. This is a genuine virtue of the proposal.

It does **not** remove the dynamical rate `rho`. A passive rescaling changes
the descriptions of both clocks together and leaves (1) fixed. Changing
`rho` changes the clock relative to the substrate tick and produces a
different compatible dynamics. Common-unit covariance and relative-rate
selection are different questions.

Likewise, assigning a different physical length to `ell` does not change (1)
when `C_sub` is transformed with the same lattice calibration. No physical
value of `a_phys` has been derived.

## 6. Speed ambiguity

FTD carries two distinct raw speeds relevant here:

```text
C_MOORE = ell/tau                    topological support bound,
C_SPEED = ell/(sqrt(3)tau)           selected production transport value.
```

The nominal axial-edge time `ell/C_SPEED` associated with the selected
transport value is `sqrt(3)tau`, so that choice gives

```text
d_4^(transport)=sqrt(3)d_4^(support).              (9)
```

Equation (9) is not an inconsistency. It shows that “use the minimum distance
and the speed” is incomplete until the metric and speed role are named. The
axiomatic Moore support cone and the selected wave/particle transport cone
answer different questions. The Moore statement uses `L-infinity` reach;
face- and body-diagonal Euclidean reaches are different and must not be
silently substituted for the registered axial interval.

## 7. P1--P5 conservative-extension underdetermination theorem

### 7.1 Free-rate countermodel

Take any `rho>0` and apply the exact time-`tau` flow of `H_(4,rho)` on-site
once per substrate tick. This update is deterministic. Because it is on-site,
it uses a strict subset of the allowed Moore dependency cone. Changing `rho`
does not alter the lattice, tick sequence, ternary substrate, locality bound,
or determinism.

On one fixed shell,

```text
d_4(2rho,E)=2d_4(rho,E).                           (10)
```

Thus two conservative extensions with the same P1--P5 reduct give different
coordinate-edge/clock ratios. A unique value cannot be a logical consequence
of those postulates plus mere compatibility with an auxiliary quartic clock.
This is not a universal no-go for a future native rule that adds and fixes the
missing clock type.

The argument is even stronger before extension: P1--P5 contain no
action--angle variable or quartic Hamiltonian at all. FTD-0658 also warns that
the global tick label is not a state-functional intrinsic matter phase, while
FTD-0659 finds coherent phase but not conserved constituent action in the
strongest registered native excited-clock candidate.

This proof does not rely on FTD-0208's stronger `L1` update-budget argument.
A deterministic local tick can change several state components at once. The
free relative-rate countermodel alone is sufficient for the present no-go.

### 7.2 Free-shell obstruction

Even if `rho=1` were separately fixed,

```text
d_4(E) proportional to E^(1/4).                    (11)
```

Every `E>0` is a valid quartic orbit. The amplitude-one choice `E=1/2` is the
shell on which the period equals `sqrt(pi)G*`; it is not selected merely by
calling the amplitude “one.” A native formation or invariant-measure law must
select the shell before (3) can become a physical prediction.

### 7.3 Scoped conclusion

The minimum edge solves the dimensional bookkeeping problem. It does not
solve the dynamical matching problem. The strongest theorem available from
the current premises is the family (1), not its `rho=1`, `E=1/2` member.

## 8. Common-cone control

FTD-0770 proved for the selected coupled clock, with graph continuum factor
`d_R` and `eta=kappa/E`,

```text
(c_clock/(Omega_m ell))^2
 = d_R eta (m-2)/(2m).                             (12)
```

If one tries to derive the synchronization by imposing
`c_clock=C_sub`, then for `m>2`

```text
Omega_m ell/C_sub
 = sqrt[2m/(d_R eta (m-2))].                       (13)
```

For the quartic clock,

```text
Omega_4 ell/C_sub=2/sqrt(d_R eta),
d_4=1/[pi sqrt(d_R eta)].                          (14)
```

The exact period normalization `K_m`, hence the direct quartic `G*` factor,
cancels. Therefore common-cone matching does not rescue a non-rescalable
`G*` signature from the FTD-0770 linear field. A future independent derivation
of `eta` could carry its own algebraic structure, but inserting `G*` into
`eta` would not establish that the quartic period produced it.

This control exposes the fork precisely:

```text
compare a selected quartic clock directly with the primitive tick
    -> exact G*-bearing ratio, but free rho and E;

derive the clock rate by matching the selected linear clock-wave cone
    -> period normalization cancels, leaving eta and graph topology.
```

## 9. Exact certificate

[`proof_quartic_clock_rod_synchronization.py`](../../../../scripts/proofs/proof_quartic_clock_rod_synchronization.py)
contains no fitted or empirical inputs. It is an exact symbolic consistency
certificate accompanying the analytic derivation and executes `20/20` checks
covering the transformed period quadrature, quartic gamma reduction, rate
scaling, coordinate-edge/clock ratios, common coordinate scaling, edge
cancellation,
`C_SPEED` alternative, free-rate and free-shell discriminators, and
common-cone cancellation.

```text
FTD-0771 quartic clock--rod exact certificate: 20/20 PASS
CLOCK_ROD_RATIO_CONDITIONAL_GSTAR_PRESENT
P1_P5_SYNCHRONIZATION_UNDERDETERMINED
COMMON_CONE_GSTAR_CANCELLATION
```

## 10. Scientific conclusion and successor gate

The proposed construction is appropriate and mathematically productive. It
finds the independently referenced comparison that FTD-0770's internal
wave-to-cycle ratio lacked. It thereby establishes where `G*` can appear:
in a freely normalized comparison of an imposed quartic clock with the
primitive substrate tick.

It does not establish an operational material rod, realized signal
trajectory, or native clock, and it does not yet derive their synchronization.
A valid successor must obtain,
without setting them by convention,

1. a state-functional native phase with an autonomous rate relative to the
   production tick;
2. a conserved conjugate action or equivalent invariant;
3. a formation or stability law selecting one nonzero shell; and
4. a declared choice of support cone versus transport cone.

If those dynamics uniquely return `rho=1`, `E=1/2`, and the quartic exponent,
then (3) becomes a derived coordinate-edge/clock ratio. Until then it is an
exact conditional theorem plus a scoped underdetermination result.
