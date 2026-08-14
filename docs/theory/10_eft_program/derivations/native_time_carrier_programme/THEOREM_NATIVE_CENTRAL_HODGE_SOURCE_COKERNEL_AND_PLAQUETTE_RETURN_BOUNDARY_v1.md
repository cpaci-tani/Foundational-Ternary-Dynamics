# Theorem — Native central-Hodge source cokernel and plaquette-return boundary v1

**Identifier:** `FTD-0920`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — UNIQUE LINEAR RETURN SOURCE]` +
`[THEOREM — EXACT EIGHT-FIBER CENTRAL-HODGE COKERNEL]` +
`[CLOSED NEGATIVE — ELEMENTARY PLAQUETTE CLOSURE BY THE LIVE CENTRAL SOURCE]` +
`[CONDITIONAL — MOORE-COATED RELAXED PERIODIC SOURCE RANGE]` +
`[OPEN — LOCAL TERNARY RECIPROCAL CLOCK BODY]`

## 1. Result

The source needed to turn any prescribed finite body into an exact invariant
oscillator is unique. For an isotropic body stiffness `kappa`, it is

\[
 \boxed{U_{\rm ret}=(K-\kappa I)J.}
\]

The unchanged production coupling source

\[
 \mathcal H(s,sv)=-G_C\nabla_c s+G_C\operatorname{curl}_c(sv)
\]

cannot supply that return for the elementary FTD-0918 plaquette. This remains
impossible after enlarging `s` and `sv` to arbitrary independent continuous
density and current fields.

The reason is exact. Central gradient and central curl are simultaneously
blind at the eight zero/Nyquist corners

\[
 k_\epsilon=\pi\epsilon,
 \qquad \epsilon\in\{0,1\}^3.
\]

The elementary plaquette has nonzero content in four of those fibers. The
free stiffness equals `4/3` in two and `16/9` in the other two. No one body
stiffness can cancel both sets, so its unique return source lies outside the
production source range.

The already-selected FTD-0577 Moore coat removes all eight blind components.
On a fixed even periodic quotient, the coated return therefore lies in the
**relaxed global linear** source range. This is not yet clock hardware: the
preimage need not be local, ternary, support-gated, continuity-compatible,
reciprocal, or powered by a positive autonomous reservoir.

The missing dynamics are consequently narrower than “confinement” in the
abstract. A viable local clock body must be parity balanced first, and then
must realize its exact return through local ternary source mechanics with a
closed work/reaction ledger.

## 2. The unique return source

Let `B` embed body coordinates `q` into the field configuration,

\[
 J=Bq,
\]

and let `K_b` be the desired stiffness inside that body. The production-order
kick is

\[
 P^+=P-KBq+U.
\]

Exact restricted motion requires

\[
 P^+=P-BK_bq.
\]

Subtracting the two equations fixes `U` without freedom:

\[
 \boxed{U_{\rm ret}(Bq)=(KB-BK_b)q.}
\]

For the `C4` doublet, symmetry forces `K_b=kappa I`, as proved in FTD-0918.
Therefore

\[
 \boxed{U_{\rm ret}=(K-\kappa I)J.}
\]

The term `KJ` returns precisely the exterior leakage/boundary force that the
free field would otherwise transmit away; `-kappa J` leaves the desired
internal oscillator kick. This identity establishes what the hardware must
do. It does not establish an actuator, feedback channel, or source of work.

## 3. Exact range of the central-Hodge source

Ignore the nonzero common factor `G_C`. At wavevector `k`, define the real
central-derivative vector

\[
 d(k)=(\sin k_x,\sin k_y,\sin k_z).
\]

Up to the common Fourier factor `i`, the map from scalar density and vector
current to vector impulse is

\[
 H(d)=\begin{bmatrix}-d&C(d)\end{bmatrix},
 \qquad C(d)j=d\times j.
\]

The cross-product matrix is skew and obeys

\[
 C(d)C(d)^T=|d|^2I-dd^T.
\]

Consequently,

\[
 \boxed{H(d)H(d)^T=|d|^2I.}
\]

This gives the complete modewise rank classification:

\[
 \operatorname{rank}H(d)=
 \begin{cases}
 3,&d\ne0,\\
 0,&d=0.
 \end{cases}
\]

On an even periodic quotient, `d=0` exactly when every coordinate is `0` or
`pi`. There are eight such modes. Therefore

\[
 \boxed{
 \operatorname{Ran}\mathcal H
 =\{U:\widehat U(\pi\epsilon)=0
 \text{ for every }\epsilon\in\{0,1\}^3\}.}
\]

For the exact `L=4` witness, the vector target has dimension `3*4^3=192`.
The 56 ordinary modes contribute rank three and the eight blind modes rank
zero:

\[
 \operatorname{rank}\mathcal H=56\cdot3=168,
 \qquad
 \dim\operatorname{coker}\mathcal H=24.
\]

At every ordinary mode, an explicit relaxed right inverse is

\[
 \begin{pmatrix}\widehat\rho\\\widehat j\end{pmatrix}
 ={H(d)^T\widehat U\over |d|^2}.
\]

This inverse is modewise and global. Its existence does not imply a
uniformly finite-range real-space inverse.

## 4. Real-space parity ledger

The eight blind Fourier conditions have an exact local-lattice statement.
For each parity character, define

\[
 M_\epsilon(U)=
 \sum_x(-1)^{\epsilon\cdot x}U(x).
\]

A shift by either `+e_i` or `-e_i` multiplies the character by the same sign
`(-1)^{epsilon_i}`. Their difference therefore has zero character sum. Hence

\[
 M_\epsilon(\nabla_c\rho)=0,
 \qquad
 M_\epsilon(\operatorname{curl}_c j)=0
\]

for all eight `epsilon` and for arbitrary fields.

The `8 x 8` character matrix on the site-parity classes is the Walsh matrix
`W`, with

\[
 WW^T=8I,
 \qquad W^{-1}={1\over8}W^T.
\]

Thus all eight moments vanish if and only if the total vector impulse in
each of the eight site-parity classes vanishes. A central-Hodge actuator may
redistribute impulse inside a parity class, but it cannot supply a net impulse
to that class.

This is why the condition is ledgerable: the obstruction is an exact set of
eight conserved source budgets, not a qualitative statement about visual
symmetry.

## 5. Elementary plaquette obstruction

Use one component of the FTD-0918 first-harmonic plaquette,

\[
 f=\delta_{000}-\delta_{110}.
\]

At a blind corner,

\[
 \widehat f(\pi\epsilon)
 =1-(-1)^{\epsilon_x+\epsilon_y}.
\]

It is zero when `epsilon_x=epsilon_y` and equals two when exactly one of
`epsilon_x,epsilon_y` is one. The four nonzero blind fibers are

\[
 (100),(010),(101),(011).
\]

For the production `C18` stiffness `K=-Delta_18/3`, direct evaluation gives

\[
 \kappa_{100}=\kappa_{010}={4\over3},
 \qquad
 \kappa_{101}=\kappa_{011}={16\over9}.
\]

The unique return source has blind amplitude

\[
 \widehat U_{\rm ret}(\pi\epsilon)
 =(\kappa_\epsilon-\kappa)\widehat f(\pi\epsilon).
\]

For it to lie in the central-Hodge range, all four amplitudes must vanish.
The first pair requires `kappa=4/3`; the second requires `kappa=16/9`.
Because these values differ,

\[
 \boxed{
 \text{no scalar body stiffness closes the elementary plaquette through }
 -\nabla_cs+\operatorname{curl}_c(sv).}
\]

Two exact controls expose the mismatch:

| desired stiffness | `z`-even blind return | `z`-odd blind return |
|---:|---:|---:|
| bare internal `25/18` | `-1/9` | `7/9` |
| one-tick quarter-turn `2` | `-4/3` | `-4/9` |

This is stronger than FTD-0918's leakage witness. It proves not merely that
the free body opens, but that the existing production source architecture
cannot close it, even with its live ternary restrictions removed.

## 6. Why the Moore coat is the correct next control

FTD-0577 selected the symmetric separable radius-one coat

\[
 B_M(k)=\prod_i\cos^2{k_i\over2}.
\]

It vanishes whenever any coordinate equals `pi`. The plaquette itself is
neutral and hence vanishes at the zero mode. Therefore

\[
 \widehat{B_Mf}(\pi\epsilon)=0
\]

at all eight blind corners. Since `K` is diagonal in the same Fourier basis,

\[
 \widehat{(K-\kappa I)B_Mf}(\pi\epsilon)=0
\]

for arbitrary `kappa`. The coated return is thus in the relaxed global source
range on every fixed even periodic quotient.

This does not promote the coat. Its inherited FTD-0577 status remains
`[SELECTED — NONCARDINAL COUPLING COAT]`. Nor does it turn the relaxed inverse
into production matter. The live source still requires

\[
 \rho=s\in\{-1,0,+1\},
 \qquad j=sv,
\]

so current is unavailable where the manifested state is zero. The modewise
inverse may be box-spanning, and no continuity-compatible moving source,
source reaction, or positive battery has been supplied.

## 7. Dynamics now missing

FTD-0919 left a generic localization/confinement debt. This theorem resolves
the first branch of that debt and produces a concrete construction target.
A viable successor body needs all of the following:

1. **Parity-balanced carrier geometry.** Its configuration and required
   return source must have zero total in every site-parity class. The
   FTD-0577 Moore coat is the current minimum symmetric reference.
2. **Local ternary source compilation.** The return must be expressed by a
   finite-range `s in {-1,0,+1}` and `j=sv`, rather than an independent
   continuous Fourier preimage.
3. **Central continuity.** The density/current history must satisfy the
   condition required by the exact FTD-0576 work ledger.
4. **Reciprocal reaction.** Matter/source variables must receive the opposite
   impulse and work rather than acting as an external controller.
5. **Positive autonomous storage.** A retained source/environment pair must
   pay and recover the boundary-return work over a cycle.
6. **Formation and stability.** The body must arise and survive under local
   dynamics, not be reinitialized each tick.

Only after those close is it meaningful to test whether the maintained
oscillator's period can be synchronized by the quartic `G*` clock law. `G*`
does not repair this source-range obstruction and was not used in the proof.

## 8. Epistemic boundary

This theorem does not derive a local clock, a new force, a ternary source
preimage, a local Hodge inverse, an autonomous battery, a physical particle,
gamma, Born frequencies, Bell correlations, or the `G*` gearbox. It also does
not alter production.

The positive coated statement is deliberately conditional and global:

\[
 \text{blind moments vanish}
 \Longrightarrow
 \text{a relaxed source exists on a fixed periodic quotient}.
\]

It is not substrate evidence until locality, ternarity, continuity,
reciprocity, and energy closure are independently certified.

## 9. Verification

The locked preregistration is
`PREREG_NATIVE_CENTRAL_HODGE_SOURCE_COKERNEL_AND_PLAQUETTE_RETURN_BOUNDARY_v1.md`
with SHA-256
`8972B48856FFB374AB1764E539F1405A8EDD794690E660EC12547C839E8DB448`.

The independent exact certificate is
`scripts/proofs/proof_native_central_hodge_source_cokernel_plaquette_return_boundary.py`
with SHA-256
`10F659D17FD8A4137D9E7C3E2221871A3B207F27509609DA9532E4BD09C675EB`.

It passes `68/68` exact gates and reports

```text
OUTCOME=A_NATIVE_CENTRAL_SOURCE_COKERNEL_PLAQUETTE_RETURN_OBSTRUCTION
UNIQUE_RETURN_SOURCE=K_MINUS_KAPPA
CENTRAL_HODGE_BLIND_FIBERS=8
L4_VECTOR_COKERNEL_DIMENSION=24
ELEMENTARY_PLAQUETTE_DIRECT_SOURCE_CLOSURE=FALSE
MOORE_COAT_RELAXED_PERIODIC_RANGE=TRUE
TERNARY_LOCAL_RECIPROCAL_REALIZATION=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```
