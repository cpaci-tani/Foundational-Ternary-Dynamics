# Theorem — Noncanonical genesis requires environment feedback or reset (FTD-0571)

**Status:** `[THEOREM — BLOCK-TRIANGULAR SYMPLECTIC NO-GO]` +
`[THEOREM — RAW GENESIS DEFECT RANK]` +
`[SOURCE AUDIT — EXISTING CONTINUOUS SPECTATORS UNCHANGED]` +
`[CLOSED NEGATIVE — SPECTATOR-ONLY NATIVE RESERVOIR]`

**Verdict:** `ENVIRONMENT_FEEDBACK_OR_RESET_REQUIRED`

**Dependencies:** FTD-0567, FTD-0569, FTD-0570.

## 1. General block theorem

Let `x` denote a `2n`-dimensional canonical system and `e` a
`2m`-dimensional canonical environment. At one differentiable branch, write
the derivative of an enlarged map as

\[
S=\begin{pmatrix}M&B\\C&D\end{pmatrix},
\qquad
\Omega=\begin{pmatrix}\Omega_x&0\\0&\Omega_e\end{pmatrix}.
\]

Suppose the projected system output is independent of the incoming
environment. Then

\[
B=\frac{\partial x'}{\partial e}=0.
\]

The lower-right block of `S^T Omega S=Omega` becomes

\[
D^{\mathsf T}\Omega_eD=\Omega_e.
\]

Thus `D` is symplectic and invertible. The upper-right block becomes

\[
C^{\mathsf T}\Omega_eD=0.
\]

Since both `Omega_e` and `D` are invertible, this forces `C=0`. The remaining
upper-left block is therefore

\[
\boxed{M^{\mathsf T}\Omega_xM=\Omega_x.}
\]

This proves:

> A system map that is independent of the incoming bath admits a symplectic
> dilation only if the system map is already symplectic.

Equivalently, any symplectic dilation of a non-symplectic projected map must
have `B!=0`: the system output must depend on the bath microstate in an open
neighborhood of the prepared state.

The theorem is local and finite-dimensional. It does not assume a continuum
time flow between ticks.

## 2. Rank of the genesis symplectic defect

Use the FTD-0570 canonical test pair `x=(J,W)`. In the radial/tangential basis,
the accepted single-genesis derivative is

\[
M=\operatorname{diag}(A,aI),
\qquad
A=\operatorname{diag}(1,t,t),
\]

where

\[
t=\frac{x}{x+k_g}\in(0,1),
\qquad
a=1-d\in(0,1].
\]

With the standard six-dimensional canonical form,

\[
\Delta
=\Omega_x-M^{\mathsf T}\Omega_xM
=\begin{pmatrix}0&K\\-K&0\end{pmatrix},
\]

where

\[
K=I-aA
=\operatorname{diag}(d,1-at,1-at).
\]

Hence

\[
\operatorname{rank}\Delta=
\begin{cases}
4,&d=0,\\
6,&0<d<1.
\end{cases}
\]

Turning off kinetic drain does not restore a canonical event: the two
tangential defects remain nonzero because the radial subtraction contracts
angular phase volume. Positive drain makes the defect full rank.

All 90 registered direction/excess/drain arms reproduce these ranks: 30 rank-4
and 60 rank-6 witnesses. The analytic and matrix defects agree exactly in the
C++ observer and below `5.56e-17` in the independent determinant check. The
maximum raw volume Jacobian remains `0.308641975308642`.

## 3. Existing `Voxel` fields cannot act as untouched compensators

The accepted single-genesis event writes only

\[
J\mapsto (1-k_g/|J|)J,
\qquad
W\mapsto(1-d)W
\]

among continuous variables. `manifest_at` assigns the discrete state,
particle-ID sentinel, spin, and color.

The source-locked audit finds 34 other continuous `Voxel` components unchanged
by this event:

| Spectator group | Components |
|---|---:|
| dual flux/wave fields | 12 |
| velocity and remainder | 6 |
| latency, proper time, clock phase, acceleration | 4 |
| strong flux/wave fields | 6 |
| weak flux/wave fields | 6 |
| **total** | **34** |

For any of these fields treated as an incoming environment, the projected
`(J,W)` output has `B=0`. The block theorem then requires the raw derivative
`M` to be symplectic, contradicting the rank witnesses.

Discrete state/spin/color labels can distinguish branch sheets but cannot
compensate a continuous symplectic defect. The counter-based RNG is a pure
function of seed, site, tick, and salt; it is not a dynamically updated bath
coordinate and supplies no `B` block.

Therefore the missing FTD-0570 environment cannot be obtained merely by
renaming an existing untouched `Voxel` field.

## 4. The prepared-bath loophole is open-system, not native closure

The theorem does not forbid a Hamiltonian enlarged map that agrees with
production only on a specially prepared bath state `e=e0`. Such a map can have

\[
\left.\frac{\partial x'}{\partial e}\right|_{e_0}\ne0
\]

even while its value at `e0` equals the production assignment.

But after the event the bath generally no longer occupies `e0`. Repeating the
same projected rule then requires one of three things:

1. reset or replace the bath;
2. export its accumulated record/energy and inject a fresh state;
3. retain an exact infinite-information history arranged to revisit the
   required preparation.

FTD-0499, FTD-0569, and FTD-0570 price these alternatives. None is present in
the production state or tick. A prepared-bath dilation is therefore a driven
open-system construction unless a new native transport/reset mechanism is
derived.

## 5. Classification

- **existing untouched spectators as reservoir:** closed negative;
- **environment-independent symplectic projection:** theoremically impossible
  for the frozen genesis map;
- **bath-dependent enlarged dynamics:** mathematically open, but it must alter
  the off-preparation system update;
- **repeated exact production behavior:** requires reset/export or
  infinite-information preparation;
- **native common action:** not recovered.

The environment fork is now narrower than “add a reservoir.” It must derive a
specific bath-to-`(J,W)` feedback law and a physical reset/transport channel,
or it is only a formal dilation. The alternative remains a different nonlinear
bound-state action and event transaction.

## 6. Non-implications

- The theorem does not prohibit fundamental irreversible FTD dynamics.
- It does not rule out a modified genesis rule with explicit recoil into
  neighboring fields.
- It does not prove a thermodynamic reset cost, bath temperature, or entropy
  production rate.
- It does not derive matter, charge, mobility, unitarity, or a scenario.
