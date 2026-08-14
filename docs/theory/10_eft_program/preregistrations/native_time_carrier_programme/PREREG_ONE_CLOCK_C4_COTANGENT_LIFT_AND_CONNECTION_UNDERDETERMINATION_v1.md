# Pre-registration — One-clock C4 cotangent lift and connection underdetermination v1

**Identifier:** `FTD-0976`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Does the clock-dependent cotangent lift of the retained `C4` carrier force
the proposed one-clock law

\[
 H={\left[\Pi+{\cal A}(\delta)(G-qI)\right]^2\over2M}
   +V(\delta)+H_{\rm int},                                 \tag{1}
\]

or does it force only the single-square form after an additional fiber
representation and connection have been selected?

The test must distinguish:

1. what follows uniquely from a specified clock-dependent fiber chart;
2. what the endpoint `C4` action fixes only modulo four;
3. what remains free in the local connection profile;
4. when the connection is a removable passive chart effect; and
5. what extra structure is required for physical mapping-torus holonomy,
   switching, or production use.

No production file, public type, engine phase, scale, coupling, or Born target
may change. No numerical search or fit is permitted.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md` | `56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1` |
| `THEOREM_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md` | `1729617446272A47C5A5812F88A89416E9ABC609CA672671017CFB8AEDD5D63E` |
| `THEOREM_MINIMUM_SUSPENSION_PRODUCTION_PAIR_OWNERSHIP_AND_MERGED_SQUARE_BOUNDARY_v1.md` | `E8FB92A279B3701EDEF6417098FED967B5B505B633690B54E197815FAA69645E` |

The frozen FTD-0975 result is controlling: pair capacity permits a shared
clock but does not authorize equation (1).

## 3. Registered regular fiber chart

Work on one regular local chart with clock coordinate `delta`, mechanical
clock momentum `K`, two internal angles `(beta,alpha)`, and their conjugate
generators `(G,I)`. The field action obeys `I>0` on the polar chart. The
origin `I=0`, where `alpha` is undefined, is outside this local chart.

Fix for one gate:

- integer representation weights `r_G,r_I`;
- a ternary representation character `q in {-1,0,+1}`;
- differentiable functions `F_G(delta),F_I(delta)`; and
- connections `A_G=F_G'`, `A_I=F_I'`.

The label `q` is only a fixed representation sector in this protocol. It is
not identified with the production state `s`.

Define the clock-dependent fiber chart

\[
 \beta_{\rm lab}=\beta-r_G F_G(\delta),\qquad
 \alpha_{\rm lab}=\alpha+q r_I F_I(\delta).                \tag{2}
\]

The canonical one-form gate must determine the new canonical momentum
`Pi` uniquely:

\[
 K\,d\delta+G\,d\beta_{\rm lab}+I\,d\alpha_{\rm lab}
 =\Pi\,d\delta+G\,d\beta+I\,d\alpha.                       \tag{3}
\]

## 4. Frozen cotangent-lift prediction

Equation (3) predicts

\[
 \Pi=K-r_G A_GG+q r_I A_II,
 \qquad
 K=\Pi+r_G A_GG-q r_I A_II.                               \tag{4}
\]

For one bare clock kinetic term, the transformed Hamiltonian must therefore
be

\[
 H={K^2\over2M}+V+H_{\rm int}
  ={\left(\Pi+r_G A_GG-q r_I A_II\right)^2\over2M}
   +V+H_{\rm int}.                                         \tag{5}
\]

The protocol must verify the full six-dimensional symplectic Jacobian, not
only compare equation shapes.

## 5. Frozen reaction and holonomy gates

Assume `H_int` is independent of `beta,alpha`, and that `q,r_G,r_I` are fixed
during the gate. Then `G` and `I` must be conserved. With equation (4), the
Hamilton equations must give

\[
 \dot\delta={K\over M},\qquad
 \dot K=-V'(\delta),                                      \tag{6}
\]

and the interaction-picture phase increments

\[
 \Delta\beta_{\rm int}=r_G\int A_G\,d\delta,
 \qquad
 \Delta\alpha_{\rm int}=-q r_I\int A_I\,d\delta.         \tag{7}
\]

Thus the connection forces reciprocal canonical reaction while leaving the
mechanical clock equation bare. This is conditional on the registered chart;
it is not yet a production theorem.

## 6. Frozen underdetermination controls

### 6.1 Local profile

On `delta in [0,1]`, compare

\[
 A_0(\delta)={\pi\over2},\qquad
 A_\epsilon(\delta)={\pi\over2}
                  +\epsilon(2\delta-1).                    \tag{8}
\]

Both have integral `pi/2`, but they differ locally for nonzero `epsilon`.
The same endpoint quarter-turn therefore does not fix the force profile.

### 6.2 Integer lift

The weights `r=1` and `r=5` give the same endpoint `C4` matrix because
`J^5=J`, but equation (4) assigns different local momentum reactions. The
finite carrier fixes representation weight only modulo four. Choosing the
minimum lift `r=+/-1` is a selection.

### 6.3 Common diagonal connection

The compact law (1) requires

\[
 r_G=r_I=1,\qquad A_G=A_I={\cal A}.                         \tag{9}
\]

Neither equal endpoint integrals nor the discrete `C4` carrier alone imply
equation (9). The certificate must exhibit unequal profiles with identical
endpoint holonomy.

### 6.4 Cross interaction

After equation (9) is selected, expanding the unique one-clock square must
produce

\[
 -{{\cal A}^2qGI\over M}.                                  \tag{10}
\]

The cross term is then forced by the selected merged cotangent lift and may
not be removed independently. What remains unforced is the diagonal lift
itself, not its algebraic expansion.

## 7. Time reversal and switching boundary

For the signed generator convention inherited from FTD-0963, test the
anti-symplectic sector map

\[
 (\Pi,G,I,q)\longmapsto(-\Pi,-G,I,-q).                     \tag{11}
\]

It must send `K -> -K`. A nonzero `q` held fixed instead of reversed fails
that covariance for the field load.

If `q` changes discontinuously while all canonical coordinates are held
fixed, equation (4) gives

\[
 \Delta K=-(q'-q)r_I A_I I.                               \tag{12}
\]

Therefore a changing ternary sector requires an explicit reversible
switching impulse/history/work transaction. A fixed-sector proof cannot be
promoted into a state-changing law.

## 8. Passive versus active topology

On every contractible regular gate, `A_G=dF_G/delta` and `A_I=dF_I/ddelta`
are exact one-dimensional connections. Their local curvature vanishes and
equation (2) removes them by a canonical chart change.

If each `F` is single-valued on a closed clock circle, its full-loop integral
is zero. A nonzero quarter holonomy requires a twisted endpoint identification
or mapping torus, for example

\[
 (\delta=1,z)\sim(\delta=0,Jz).                            \tag{13}
\]

The retained `C4` carrier makes equation (13) available as a selected bundle
gluing. The cotangent lift does not by itself prove that the gluing acts on a
physical production field rather than on coordinates.

## 9. Frozen checks

- **G1:** protocol and frozen-source hashes plus scope markers;
- **G2:** exact canonical one-form and full symplectic-Jacobian lift;
- **G3:** one-square Hamiltonian, action conservation, mechanical reaction
  cancellation, and phase holonomies;
- **G4:** exact local-profile and integer-lift counterexamples;
- **G5:** common-diagonal specialization and mandatory cross term;
- **G6:** ternary fixed-sector, time-reversal, and switching-impulse boundary;
- **G7:** zero local curvature, closed exact-holonomy control, and twisted
  endpoint necessity;
- **G8:** no production, active-coupling, `G*`, Born/Bell, or completeness
  promotion.

All algebra is exact. No floating comparison, numerical search, parameter
fit, or near-miss scan is permitted.

## 10. Frozen classifier

- **Outcome A — uniquely forced active law:** the retained `C4` carrier and
  one clock uniquely fix equation (1), its profile, weights, and physical
  action without an added bundle selection.
- **Outcome B — conditional cotangent theorem / representation debt:** a
  specified fiber action uniquely forces one covariant momentum and one
  kinetic square, but `C4` fixes only endpoint classes; profile, integer lift,
  common diagonal action, bundle gluing, switching transaction, and physical
  production identity remain selected or open.
- **Outcome C — no canonical merged lift:** equation (3) cannot yield a
  symplectic one-clock realization.
- **Outcome D — invalid:** any hash, identity, exact control, or scope gate
  fails.

The expected result is Outcome B. Success may sharpen the proposed law but
does not authorize production integration.
