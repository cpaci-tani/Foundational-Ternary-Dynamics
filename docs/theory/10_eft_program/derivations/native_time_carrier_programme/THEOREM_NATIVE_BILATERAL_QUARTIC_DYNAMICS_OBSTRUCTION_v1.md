# Theorem — Native Bilateral/Quartic Dynamics Obstruction v1

**Identifier:** `FTD-0838`  
**Status:** `[THEOREM — FROZEN PRODUCTION L/R CORE HAS NO ORIENTED EXCHANGE]` +
`[THEOREM — SMOOTH FIXED-STATE PHASE_READ BRANCH HAS NO QUARTIC RESTORER]` +
`[THEOREM — HOMOGENEOUS DAMPING HAS NO POSITIVE STABLE SHELL]` +
`[CONDITIONAL THEOREM — DEGREE-MINIMUM RADIAL/BATH EXTENSION]` +
`[SELECTION/OPEN — COARSE PAIR CLOSURE AND PHYSICAL G* GEARBOX]`  
**Date:** 2026-08-10  
**Production impact:** none  
**Certificate:**
`scripts/proofs/proof_native_bilateral_quartic_dynamics_obstruction_v2.py`,
`22/22`

## 0. Result

The present production substrate does **not** contain the dynamics required
to turn the FTD-0836 self-dual quartic clock into native hardware:

1. `J_L` and `J_R` evolve under identical independent operators, so their
   sum and difference block-diagonalize and never rotate into one another;
2. every smooth fixed-state `phase_read` field-acceleration branch is affine
   in the field and therefore has at most a quadratic modal potential; nulling
   its quadratic stiffness makes it flat, not quartic; and
3. production damping is homogeneous, so it contracts toward zero rather than
   selecting a nonzero energy shell.

Within the smallest registered repair class, the minimum additional dynamics
is

\[
 \boxed{X_{n+1}=[1+\eta(1-E_n)]JX_n},
 \qquad
 E_n=X_n^TX_n,
 \qquad 0<\eta<1,                              \tag{1}
\]

together with one external energy account

\[
 \boxed{B_{n+1}=B_n+E_n-E_{n+1}}.              \tag{2}
\]

Equation (1) is degree-minimum in the registered context-blind radial-gain
class, but it is not native FTD dynamics. Equation (2) is the smallest exact
energy ledger for the repair. The still-missing gearbox is the physical map
from a constituent ensemble to the signed pair coordinate `u=q|q|`, and from
that coordinate's nonuniform quartic flow to global substrate ticks.

## 1. Exact scope

This result is locked to the source hashes listed in
[`PREREG_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md).
It is a theorem about:

- the frozen CPU production `phase_read`/`phase_write` L/R field core;
- its explicit weak L/R swap;
- the smooth `phase_read` acceleration law on a branch where the ternary state
  and event decisions are fixed; and
- the registered radial-gain extension class.

It is not a theorem that no switching, correlated constituent ensemble,
coarse-graining, or future local rule could generate an effective quartic
law. Genesis and evaporation are threshold/event rules. An effective law
obtained by averaging their histories would require a separately defined
coarse-graining map and a fresh preregistered test.

## 2. Obstruction I: no native clockwise/counterclockwise exchange

### 2.1 Production algebra

On every L/R-symmetric production phase, write the field update abstractly as

\[
 L'=AL+b,\qquad R'=AR+b,                        \tag{3}
\]

where `A` contains the shared wave operator, clock scalar if enabled,
integrator, and damping, while `b` is the equally split state source. Define

\[
 F=L+R,\qquad D=L-R.                            \tag{4}
\]

Then exactly

\[
 \boxed{F'=AF+2b,\qquad D'=AD}.                 \tag{5}
\]

The register matrix is block diagonal. There is no term of the form
`D -> F` or `F -> -D`. This reproduces the earlier exact sum/difference
theorem in
[`EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md`](../../../08_structural/EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md)
directly against the current production hashes.

The physical content is sharp: production has two copies of one wave sector,
not one oriented two-phase oscillator.

### 2.2 Why the weak swap is insufficient

The required oriented exchange is

\[
 J=\begin{pmatrix}0&1\\-1&0\end{pmatrix},
 \qquad J^2=-I,
 \qquad \det J=+1.                              \tag{6}
\]

Production weak transmutation instead applies

\[
 S=\begin{pmatrix}0&1\\1&0\end{pmatrix},
 \qquad S^2=I,
 \qquad \det S=-1.                              \tag{7}
\]

In `(F,D)` coordinates,

\[
 S:(F,D)\longmapsto(F,-D).                      \tag{8}
\]

Thus the weak event reverses an existing chirality label. It does not advance
an oriented phase. Repeated swaps have period two, whereas the smallest real
orientation carrier has period four.

## 3. Obstruction II: no smooth native quartic restorer

Fix the ternary state and source during one smooth `phase_read` acceleration
branch. The field acceleration is affine. Restricted to any modal coordinate
`q`, its conservative potential has the form

\[
 V(q)=\frac{\kappa}{2}q^2-hq+V_0.              \tag{9}
\]

Therefore

\[
 \frac{d^4V}{dq^4}=0.                           \tag{10}
\]

If `q=0` is an equilibrium, then `h=0`. If null-flatness is additionally
required, `kappa=0`, and

\[
 V(q)=V_0,                                      \tag{11}
\]

not `lambda q^4`. This is the dynamical form of the FTD-0794 verdict: the
native linear/plaquette structure selects harmonic behavior; deleting the
quadratic term does not manufacture a quartic term.

Genesis, evaporation, and weak transmutation do not evade (10) as smooth
restorers. They are thresholded, seed-keyed state changes. Single-substrate
genesis applies a selected partial drain that is not an exact latent-heat
identity, while dual-substrate genesis has no matching drain. Those events
could participate in a future piecewise or statistical effective model, but
such a model is absent from the current local action and is outside this
theorem.

## 4. Obstruction III: damping cannot stabilize a nonzero shell

Production damping multiplies field amplitudes by `1-g`. Hence a quadratic
energy obeys

\[
 E_{n+1}=(1-g)^2E_n.                            \tag{12}
\]

The fixed-point equation is

\[
 E_{n+1}-E_n=E_ng(g-2)=0.                       \tag{13}
\]

For the production regime `0<g<1`, the only fixed point is

\[
 \boxed{E_*=0}.                                 \tag{14}
\]

When `g=0` the map is neutral at every energy; it still selects no positive
shell. Selective damping changes which sites contract but supplies no target
energy and no dynamical reservoir containing the removed energy. The
diagnostic cumulative-dissipation field is an account after the fact, not a
state that can return energy to an under-amplitude clock.

## 5. Why primitive ternarity does not supply the quartic coordinate

FTD-0836 uses the signed energy coordinate

\[
 u=q|q|,\qquad u^2=q^4.                         \tag{15}
\]

At one primitive ternary site, however,

\[
 s|s|=s\qquad(s\in\{-1,0,+1\}).                \tag{16}
\]

The map is the identity and creates no new degree. Nor does it commute with
coarse-graining. For the exact ensemble

\[
 P(+1)=\frac34,qquad P(-1)=\frac14,
 \qquad P(0)=0,                                 \tag{17}
\]

one has

\[
 \mathbb E[s|s|]=\mathbb E[s]=\frac12,
 \qquad
 \mathbb E[s]|\mathbb E[s]|=\frac14.           \tag{18}
\]

So the desired `u` is not the expectation of a primitive one-site observable.
An independent identically prepared pair would supply

\[
 \mathbb E[s_1s_2]=\mathbb E[s_1]\mathbb E[s_2]=q^2,  \tag{19}
\]

but (19) already assumes a constituent pairing and a factorization/closure
law; restoring `sign(q)` also requires a polarity context. Correlated pairs
replace `q^2` by their correlation. The framework therefore needs an explicit
`PairClosureMap`, not the assertion that discreteness automatically produces
the quartic coordinate.

This matches the contextual ensemble intuition: the constituents may be the
microstates of a particle, galaxy, or macroscopic object, but the ensemble's
membership and closure law are part of the context type. They cannot be
inferred from the word *ensemble* alone.

## 6. Minimum conditional repair

### 6.1 The oriented kernel is forced once the type is adopted

One real channel cannot carry an operator squaring to `-I`. In two real
channels, every orientation-preserving orthogonal map has the form

\[
 Q=\begin{pmatrix}c&s\\-s&c\end{pmatrix},
 \qquad c^2+s^2=1.                              \tag{20}
\]

Imposing `Q^2=-I` yields `c=0`, `s=+/-1`. Therefore

\[
 \boxed{Q=+J\ \text{or}\ -J}.                  \tag{21}
\]

The sign is the clockwise/counterclockwise choice. Thus two channels are
dimension-minimum, and the quarter-turn is unique up to orientation once an
oriented two-channel type has been adopted.

### 6.2 A constant gain cannot stabilize the shell

Let the context-blind correction depend only on the self-dual energy:

\[
 X_{n+1}=\rho(E_n)JX_n,
 \qquad E_n=X_n^TX_n.                           \tag{22}
\]

Then

\[
 E_{n+1}=E_n\rho(E_n)^2.                        \tag{23}
\]

To fix the unit shell with positive gain requires `rho(1)=1`. If `rho` is
constant, this forces `rho=1`, for which the radial multiplier is one: neutral,
not stable.

For a differentiable gain, the unit-shell multiplier is

\[
 \left.\frac{dE_{n+1}}{dE_n}\right|_{E=1}
 =1+2\rho'(1).                                  \tag{24}
\]

Local stability therefore requires

\[
 -1<1+2\rho'(1)<1
 \quad\Longleftrightarrow\quad
 -1<\rho'(1)<0.                                 \tag{25}
\]

### 6.3 Lowest-degree radial gain

The lowest-degree nonconstant polynomial satisfying `rho(1)=1` is linear.
Write

\[
 \rho(E)=1+\eta(1-E).                           \tag{26}
\]

Then

\[
 E_{n+1}=E_n[1+\eta(1-E_n)]^2                  \tag{27}
\]

and

\[
 \left.\frac{dE_{n+1}}{dE_n}\right|_{E=1}
 =1-2\eta.                                      \tag{28}
\]

Hence

\[
 \boxed{0<\eta<1}                              \tag{29}
\]

is exactly the local stable interval. Degree minimality is a theorem within
the registered class. Choosing this class, choosing `eta`, and installing it
in the substrate are selections.

### 6.4 Why a third register is necessary for closure

Whenever `E_{n+1} != E_n`, the two-channel core alone is not energy closed.
One scalar environmental account suffices:

\[
 B_{n+1}=B_n+E_n-E_{n+1}.                       \tag{30}
\]

Then exactly

\[
 \boxed{E_{n+1}+B_{n+1}=E_n+B_n}.              \tag{31}
\]

For an over-amplitude core, the bath receives energy; for an under-amplitude
core, the bath supplies it. A physical realization must additionally ensure
that the bath has available positive energy and specify the work/dissipation
channel. Equation (30) is an exact ledger, not yet a material mechanism.

This is the simplest answer to “something more than matter”: a stable
recursive core cannot consist only of the two phase channels whose radius it
repairs. It needs an exterior energetic degree of freedom. Because FTD does
not assume microscopic reversibility, the update may also discard irrelevant
detail, but energetic loss still has to be booked separately from information
loss.

## 7. What remains of the `G*` gearbox

If a physical coarse variable `q` and the selected pair map (15) are supplied,
then

\[
 u^2+y^2=q^4+y^2.                               \tag{32}
\]

The selected quartic Hamiltonian gives the weighted oriented flow

\[
 \frac{d}{ds}\begin{pmatrix}u\\y\end{pmatrix}
 =2\sqrt{|u|}\,J\begin{pmatrix}u\\y\end{pmatrix}.  \tag{33}
\]

Its traversal is

\[
 \int_{\rm cycle}ds
 =B\!\left(\frac14,\frac12\right)
 =\sqrt\pi\frac{\Gamma(1/4)}{\Gamma(3/4)}
 =\boxed{\sqrt\pi G^*}.                        \tag{34}
\]

The algebraic calendar is therefore coherent once the quartic coordinate and
cadence are present. The production substrate still lacks the gearbox
identifying them. The minimum outstanding interface has four parts:

| Missing type | Required role | Current status |
|---|---|---|
| `OrientedPhasePair` | realize `+/-J` in native local degrees of freedom | absent from frozen L/R core |
| `PairClosureMap` | map a declared constituent context to `u=q|q|` | selected/open |
| `EnergyBath` | exchange the radial repair energy with a positive reservoir | exact account only; mechanism open |
| `CadenceMap` | realize the weight `1/(2 sqrt(|u|))` against global tick `n` | open |

Only after these maps arise from source-blind local rules would `G*` be a
substrate clock constant rather than the exact period of a selected quartic
model.

## 8. Interpretation of bilateral systems

A left/right brain analogy points at the correct abstract shape—two
complementary registers whose relative phase retains an orientation that
quadratic summaries lose. The derivation does not identify cerebral
hemispheres with `J_L/J_R`, prove a neural quartic oscillator, or connect
consciousness to `G*`. Any such application would have to identify measurable
biological variables realizing all four rows of the interface table and beat
a harmonic control under preregistration.

## 9. Falsifiers and next experiment

The source-scoped obstruction is falsified by any production path, within the
locked scope, that exhibits one of:

1. a continuous antisymmetric L/R cross-block;
2. a nonzero smooth fourth-order restoring coefficient on a fixed-state local
   branch; or
3. an energy-closed positive shell selected by existing damping/event
   dynamics.

The constructive next experiment is not another period comparison. It is a
fresh local-rule candidate that implements the four-row interface without
reading a quartic target or `G*`. It must preregister:

- the constituent context and pair observable before trajectories are seen;
- the reservoir state and exact work ledger;
- clockwise/counterclockwise discrimination;
- harmonic and register-swap controls; and
- held-out perturbations testing whether the unit shell and quartic cadence
  emerge rather than being encoded.

## 10. Certificate provenance

FTD-0837 was the first lock and returned `20/22`; two algebraically equal
SymPy expressions were compared structurally. It books no theorem.

FTD-0838 hash-locked the full parent and changed only those two comparisons to
exact simplified differences. The inherited certificate returned `22/22` and
printed:

```text
FROZEN_PRODUCTION_CORE_ORIENTED_EXCHANGE_ABSENT
FROZEN_PRODUCTION_CORE_SMOOTH_QUARTIC_RESTORER_ABSENT
FROZEN_PRODUCTION_CORE_NONZERO_STABLE_SHELL_ABSENT
MINIMAL_BILATERAL_RADIAL_BATH_EXTENSION_CONDITIONAL_THEOREM
COARSE_GRAINING_PAIR_CLOSURE_STATUS=SELECTED_AND_OPEN
GSTAR_SUBSTRATE_GEARBOX_STATUS=NOT_DERIVED
```

No numerical search, fitted tolerance, production modification, biological
data, Born target, or post-hoc law substitution entered the certificate.
