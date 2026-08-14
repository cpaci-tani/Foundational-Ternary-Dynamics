# Theorem — Reciprocal-carry reservoir and local impulse-ledger boundary v1

**Identifier:** `FTD-0897`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT CONDITIONAL RECIPROCAL-CARRY UPDATE]` +
`[THEOREM — UNIQUE RESERVOIR INCREMENT GIVEN BRANCH AND CONSERVATION]` +
`[THEOREM — CONDITIONAL FULL-STATE REVERSAL]` +
`[BOUNDARY — IMPULSE ORIGIN/ENERGY/SCALE/SUBSTRATE PARTITION OPEN]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — NATIVE CARRY HARDWARE AND TOTAL FIELD--MATTER MOMENTUM]`

## 1. Result

FTD-0896 proves that the character dual of local integer translations is a
torus: principal Bloch labels retain momentum only modulo reciprocal-lattice
vectors. FTD-0897 supplies the minimum exact bookkeeping update for the lost
reciprocal carry during a supplied equal-and-opposite pair increment.

On one axis let `k_1,k_2 in [-pi,pi)` and define

\[
c(x)=\left\lfloor\frac{x+\pi}{2\pi}\right\rfloor,
\qquad
\operatorname{wrap}(x)=x-2\pi c(x).                         \tag{1}
\]

For a supplied dimensionless increment `+q,-q`, set

\[
\begin{aligned}
k_1'&=\operatorname{wrap}(k_1+q),&c_1&=c(k_1+q),\\
k_2'&=\operatorname{wrap}(k_2-q),&c_2&=c(k_2-q),\\
W'&=W+c_1+c_2.                                               \tag{2}
\end{aligned}
\]

Then

\[
\boxed{k_1'+k_2'+2\pi W'=k_1+k_2+2\pi W}.                  \tag{3}
\]

The same construction applies componentwise in three dimensions. Thus one
integer triplet is sufficient to retain the reciprocal information discarded
by the principal branch. This triplet is transaction bookkeeping, not yet an
identified substrate field.

## 2. Why the reservoir update is unique

Hold fixed the principal branch, reciprocal unit `2 pi`, and wrapped endpoint
labels. If `W'=W+d`, exact conservation requires

\[
-2\pi c_1-2\pi c_2+2\pi d=0,
\]

and therefore

\[
\boxed{d=c_1+c_2}.                                           \tag{4}
\]

The integer update is not an adjustable coefficient. It is forced by the
branch convention and the demand that the lifted aggregate be exactly
additive.

## 3. Conditional reversal and recursion

Apply the inverse increment `-q,+q` to the endpoint labels. Because the
original labels lie in the selected principal half-open interval, the inverse
carries are `-c_1,-c_2`; equation (2) returns `W,k_1,k_2` exactly. A sequence
of admitted pair events telescopes:

\[
W_N-W_0=\sum_e(c_{1,e}+c_{2,e}),
\qquad K_{\rm tot}(N)=K_{\rm tot}(0).                       \tag{5}
\]

This is a stable recursive ledger only conditional on the inverse dynamics
regenerating `q`, or on an event history retaining it. FTD-0897 does not derive
an autonomous interaction law.

Individual particle windings provide an equivalent partition,

\[
w_1'=w_1+c_1,
\qquad w_2'=w_2+c_2,                                        \tag{6}
\]

with `W=w_1+w_2`. The theorem does not choose between particle histories, a
bond ledger, a substrate recoil cell, or a transported stress rail. These are
physically inequivalent realizations of the same exact aggregate accounting.

## 4. Cubic and locality statement

Equation (2) acts independently on the three coordinate components and the
conserved lifted aggregate transforms as a cubic vector. Away from the
half-open branch endpoint, signed permutations commute with the componentwise
reference map. At the branch endpoint the principal representative changes,
but the compensating integer carry leaves the lifted vector unchanged.

“Local” here means one admitted pair transaction consumes only its two labels,
the supplied increment, and its attached carry record. It does not mean the
current production substrate already contains that record or a routing law
for it.

## 5. Momentum scale and energy boundary

An imposed conversion gives the candidate

\[
P_{\rm tot}=p_*\left(k_1+k_2+2\pi W\right).                 \tag{7}
\]

The carry algebra does not determine `p_*`. Under `p_* -> s p_*`, the same
dimensionless invariant gives `P -> sP`; in the FTD-0893 dressed tensor this
freedom enters quadratically. FTD-0897 therefore does not close absolute
inertial mass.

Nor does momentum closure supply self-dual energy. For a periodic band,

\[
\epsilon(k+2\pi w)=\epsilon(k),                             \tag{8}
\]

so the energy cannot see `W`. The same carry update is compatible with
inequivalent reservoir laws such as `E_R=0` and `E_R=aW^2`. Moreover a
supplied opposite quasimomentum increment need not conserve the two-label
band energy. The missing energy/work transaction cannot be inferred from
equation (3).

This cleanly separates two pieces of the proposed recursive hardware:

1. `W` is the exact discrete topological/history memory required by branch
   crossings;
2. an energy-bearing realization still needs either a conjugate phase/action
   cell or an explicit work/environment ledger derived from a common local
   action.

The existing selected phase-rail type is a reference candidate for item 2,
not evidence that native substrate carry hardware has formed.

## 6. Epistemic accounting

The following are theorem-grade within the registered branch and supplied
increment assumptions:

- exact lifted conservation;
- uniqueness of the aggregate integer update;
- conditional exact reversal;
- multi-event telescoping; and
- equivalence of aggregate and individual-winding partitions.

The following remain open:

- dynamical origin of `q` from one local matter--field action;
- physical ownership and finite-state realization of the carry triplet;
- reservoir energy/work and backreaction;
- the conversion scale `p_*`;
- exact total field--matter momentum and the FTD-0893 physical `B` map;
- absolute inertial mass and stable matter;
- production migration, Born recovery, Bell laboratory recovery,
  operational Lorentz hiding, and native `G*` synchronization.

No selected type, import, calibration, or adoption currency is added.

## 7. Certificate and implementation

The frozen preregistration SHA-256 is
`A6775AD78DA96BB606871EB6C924148CB45498DE1097EB955BD99057587B3E97`.
The frozen certificate
`scripts/proofs/proof_reciprocal_carry_reservoir_local_impulse_ledger.py`
has SHA-256
`A12998C3E6599BD76AA6F36615A31B1BED37EFE206CAE39ADC0E51F658A89C19`
and passed `89/89` on its first immutable execution without repair.

The isolated fail-closed reference implementation is:

- `engine/include/ftd/eft/reciprocal_carry_reservoir.h`, SHA-256
  `69D4D225DD0D94EBD3A13C424FB78CA51238495A3DB51625129514253293B6BE`;
- `engine/src/eft/reciprocal_carry_reservoir.cpp`, SHA-256
  `6EA6FBE2DD6D7A51D068F9D319398E3B8366FF182DE6F4EC07515F57B7F0AAE9`;
  and
- `engine/tests/test_reciprocal_carry_reservoir.cpp`, SHA-256
  `15718167F361233B9A131B0A3DF68CD335BAA7EA9E2C91483E0FF17666E5EDE2`.

The focused CTest passes `1/1`, and the isolated actualization/EFT chain passes
`25/25`. The implementation changes no production
`Voxel`, field, renderer, boundary, default toggle, or tick phase.

## 8. Next acceptance gate

Derive the supplied increment and its energy/work update from one local
matter--field action. The same transaction must say where the carry lives,
how finite local hardware represents or transports it, and how its physical
impulse unit is fixed. A phase/action realization must close an exact local
energy ledger and regenerate the inverse increment without reading a target
outcome. Only then may the complete momentum map be inserted into FTD-0893.

```text
RECIPROCAL_CARRY_UPDATE=EXACT_CONDITIONAL_ON_SUPPLIED_OPPOSITE_INCREMENT
RECIPROCAL_RESERVOIR_INCREMENT=UNIQUE_GIVEN_BRANCH_AND_CONSERVATION
FULL_STATE_REVERSAL=EXACT_IF_INCREMENT_REVERSIBLY_AVAILABLE
INTERACTION_INCREMENT_ORIGIN=OPEN
RESERVOIR_PARTITION=NOT_SELECTED
RESERVOIR_ENERGY_LAW=OPEN
PHYSICAL_MOMENTUM_SCALE=OPEN
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```
