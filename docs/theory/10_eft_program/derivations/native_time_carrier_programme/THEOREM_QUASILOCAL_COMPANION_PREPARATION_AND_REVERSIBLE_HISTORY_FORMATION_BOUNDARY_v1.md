# FTD-0929 — Quasilocal companion preparation and reversible-history formation boundary v1

**Identifier:** `FTD-0929`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — UNIQUE GAPPED COMPANION PREPARATION MAP]` +
`[THEOREM — NO FINITE-SUPPORT / FINITE-CAUSAL-DEPTH EXACT FORMATION]` +
`[REFERENCE CONSTRUCTION — TARGET-BLIND RADIUS-ONE QUASILOCAL PREPARATION]` +
`[REFERENCE CONSTRUCTION — LOCAL COTANGENT HISTORY LIFT]` +
`[SCOPED NO-GO — POSITIVE QUADRATIC ENERGY FOR THE REGISTERED LOCAL HISTORY LIFT]` +
`[OPEN — POSITIVE RESERVOIR / PORT RECYCLING / STATIC HALO / DUAL-FIELD IDENTITY]`  
**Protocol:**
[`PREREG_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md),
SHA-256 `DA0C5514E893A88C612052AFD08A2C31ED6535E0E3BD50BBCCD65FF97ED0DEA2`  
**Certificate:**
[`proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py`](../../../../../scripts/proofs/proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py),
SHA-256 `AE6B5A068C9F1A0F0F81A73DB2EB037EF13F49F31845070B833602558B4AF0A7`,
`98/98` exact checks  
**Registered outcome:** `B — UNIQUE QUASILOCAL COMPANION / REVERSIBLE-HISTORY FORMATION BOUNDARY`

---

## 1. Result

FTD-0928's reciprocal companion is not arbitrary. On the self-dual section,
the exact matter-generated dynamic source `U_n` obeys

\[
 U_n=(K-2I)Q_n.                                         \tag{1}
\]

The native C18 stiffness band is `0<=K<=16/9`, so `2I-K` has gap `2/9`.
Consequently

\[
 \boxed{Q_n=-(2I-K)^{-1}U_n}                            \tag{2}
\]

is the unique square-summable companion profile. Equation (2) is a natural
mathematical PreparationMap from the present record/current source to the
field-shaped companion. It reads no target arm, desired final profile,
measurement context, or probability.

The map is exponentially quasilocal, not strictly finite range. For the
registered 53-site source, the unique profile has infinite support. Hence no
fixed finite number of causal radius-one updates can form it exactly on the
uncontained lattice.

There is nevertheless an exact local convergent construction:

\[
 Q^{(0)}=0,
 \qquad
 Q^{(N+1)}={K\over2}Q^{(N)}-{U\over2},                  \tag{3}
\]

with the rigorous error envelope

\[
 \boxed{
 \|Q-Q^{(N)}\|_2
 \le {9\over2}\left({8\over9}\right)^N\|U\|_2.}       \tag{4}
\]

Thus physical preparation must be understood as causal convergence, not an
instantaneous creation of a completed field.

The reduced recurrence (3) is lossy. It can be lifted to an exactly local
canonical map by exporting every overwritten field configuration into a
fresh complete history pair. Retaining the outgoing history preserves exact
reversal; discarding or dispersing it produces the effective loss of locally
irrelevant detail. This supplies a precise reference meaning for
**unactualization**: loss in the reduced actual record while information
remains in the larger environment.

Canonicality is not yet physical formation. The registered local history
lift has an expanding mode and no positive-definite conserved quadratic
metric. A positive source reservoir, fresh-port origin/recycling, autonomous
stopping, nonlinear energy transfer, and massless static-halo preparation
remain open.

---

## 2. Uniqueness and the dynamic spectral gap

Writing `u=cos k_x`, `v=cos k_y`, `w=cos k_z`, the C18 symbol is

\[
 \kappa(u,v,w)
 ={4\over3}-{2\over9}
 (u+v+w+uv+uw+vw).                                      \tag{5}
\]

It is multilinear on the cube `[-1,1]^3`, so its extrema occur at cube
vertices. Their exact values are

\[
 \left\{0,{4\over3},{16\over9}\right\}.                \tag{6}
\]

Therefore

\[
 {2\over9}I\le2I-K\le2I,
 \qquad
 \|(2I-K)^{-1}\|\le{9\over2}.                          \tag{7}
\]

Equation (7) proves existence and uniqueness of equation (2) in `ell^2`.
This gap belongs to the dynamic order-four resolvent. It is not a mass gap for
the underlying free field and does not apply to the static halo.

---

## 3. Why the exact companion is not finitely supported

The certificate reconstructs the FTD-0927 arm-zero dynamic source directly
from the 19-site live current and present-state midpoint law. It has 53-site
support and exact norm

\[
 \|U_0\|^2={463\over100}.                                \tag{8}
\]

Represent finite fields by Laurent polynomials in `(x,y,z)`. If `F` is the
face-neighbor sum and `E` the edge-neighbor sum, then

\[
 18(2-\kappa)=12+2F+E.                                  \tag{9}
\]

On `y=z=1`,

\[
 18(2-\kappa(x,1,1))=6(x+x^{-1}+4),                     \tag{10}
\]

while the registered source has

\[
 U_{0x}(x,1,1)={(x-1)^2(x+1)^2\over4x^2}.               \tag{11}
\]

At

\[
 x_*=-2+\sqrt3,
\]

equation (10) vanishes and equation (11) does not. Therefore the denominator
does not divide the source in the Laurent ring. A finitely supported `Q_0`
would make `U_0=(K-2I)Q_0` divisible by that denominator, a contradiction.

Hence:

\[
 \boxed{Q_0\text{ has infinite support}.}               \tag{12}
\]

The certificate also obtains nonzero multivariate polynomial-division
remainders. The one-dimensional witness is already decisive.

Because a radius-one causal update starting from compact data has finite
support after every finite number of ticks, equation (12) also proves the
finite-causal-depth no-go. This does not prohibit the convergent causal limit.

---

## 4. Target-blind quasilocal preparation

Expanding the gapped inverse gives

\[
 Q=-{1\over2}\sum_{m=0}^{\infty}
 \left({K\over2}\right)^mU.                             \tag{13}
\]

The first `N` terms are exactly recurrence (3). Its residual is

\[
 (K-2I)Q^{(N)}-U
 =-\left({K\over2}\right)^NU.                           \tag{14}
\]

Since `||K/2||<=8/9`, summing the geometric tail proves equation (4).

Every application of `K` expands the dependency cone by at most one C18
step. Consequently `Q^{(N)}(x)` reads only source data within distance
`N-1`. For nested finite regions, two preparations agree wherever the whole
dependency cone lies in the smaller region. This is the correct local-net
restriction consistency for the finite-depth approximants.

The scalar convolution `K` commutes with signed-cubic rotations. Therefore

\[
 U_{n+1}=S U_n
 \quad\Longrightarrow\quad
 Q^{(N)}_{n+1}=S Q^{(N)}_n                              \tag{15}
\]

at every depth. The registered `C4` covariance is exact, not asymptotic.

`[REFERENCE CONSTRUCTION]` Equation (3) is a mathematical causal preparation
algorithm. Its convergence is theorem-grade; its realization as autonomous
positive-energy substrate dynamics is not.

---

## 5. Reversible history and unactualization

Write one local layer abstractly as `q'=Aq+Bz`, where `A=K/2` and
`Bz=-U/2`. The reduced map cannot be inverted on the full field space because
`K` has a zero mode.

Introduce one fresh field-shaped coordinate `e` and one outgoing history
coordinate `h`:

\[
 z'=z,
 \qquad q'=Aq+e+Bz,
 \qquad h'=q.                                            \tag{16}
\]

Its coordinate Jacobian and inverse are

\[
 J=\begin{pmatrix}I&0&0\\B&A&I\\0&I&0\end{pmatrix},
 \qquad
 J^{-1}=\begin{pmatrix}I&0&0\\0&0&I\\-B&I&-A\end{pmatrix}. \tag{17}
\]

Both use only the same local operators `A` and `B`. The cotangent lift

\[
 \mathcal T=\operatorname{diag}(J,J^{-\mathsf T})        \tag{18}
\]

is exactly symplectic. On the fresh-coordinate section `e=0`, it implements
one step of equation (3), while `h'=q` stores the complete overwritten input.

This yields the exact distinction:

- **fundamental enlarged evolution:** retain `h` and equation (18) is local
  and reversible;
- **reduced actual evolution:** omit inaccessible outgoing history and the
  observed map is contractive and lossy.

That reduced loss is a mathematically explicit candidate for
unactualization. It is not erasure from the full ontology. Information has
moved into correlations/history that the local actual record no longer
retains.

Within this registered lift, a used port is not fresh: its outgoing
coordinate equals the old `q`. Repeating `N` contraction layers therefore
requires `N` fresh complete field-shaped ports, or an independently derived
reset/compression/recycling mechanism. No universal one-pair-per-tick theorem
outside this lift is claimed.

---

## 6. Why canonical history is not yet positive formation

For a scalar contraction mode `0<a<=8/9`, the coordinate part of equation
(16) contains

\[
 C_a=\begin{pmatrix}a&1\\1&0\end{pmatrix}.              \tag{19}
\]

Its larger eigenvalue is

\[
 \lambda_+={a+\sqrt{a^2+4}\over2}>1,                   \tag{20}
\]

where the inequality follows exactly from

\[
 a^2+4-(2-a)^2=4a>0.                                    \tag{21}
\]

The cotangent lift retains this expanding eigenvalue. If a positive-definite
quadratic metric `G` were invariant, an eigenvector `v` would satisfy

\[
 v^{\mathsf T}Gv
 =\lambda_+^2v^{\mathsf T}Gv,
\]

which is impossible for `lambda_+>1` and `v!=0`. Thus the registered local
history lift has no positive conserved quadratic energy.

This is a scoped no-go for equation (16), not for all possible nonlinear
reservoir dynamics. FTD-0928's equal-metric species quarter turn is positive
and energy preserving, but it transfers a reservoir pair that already
contains the completed target phase. Using it to “derive” equation (2) would
be circular.

The next mechanism must combine the strengths of both constructions:

1. local source-driven computation rather than a preloaded target;
2. complete reversible history;
3. positive source/reservoir energy;
4. causal port supply, transport, and recycling; and
5. autonomous stopping or compliance without a global residual read.

---

## 7. The static halo is a different problem

The dynamic inverse in equation (2) is gapped. The static halo solves a
massless equation involving `K` itself. Along the exact Fourier line
`(exp(i theta),1,1)`,

\[
 \kappa(\theta)={2\over3}(1-\cos\theta)\longrightarrow0. \tag{22}
\]

Therefore every fixed local Richardson factor `1-eta K` approaches one on
long-wavelength modes. There is no volume-independent contraction constant
strictly below one analogous to `8/9`.

The dynamic companion can be prepared with a uniform geometric envelope;
the static halo cannot. A multiscale transport mechanism, boundary condition,
or genuinely massless relaxation analysis is required. The static and
dynamic formation debts must remain separate.

---

## 8. Existing left/right field capacity

Production already stores two complete field-shaped pairs:

\[
 (J_L,W_L),\qquad(J_R,W_R).
\]

This is enough **representational capacity** to host two reciprocal field
coordinates without minting a new storage type. It does not determine the
physical identification. The current engine defines observable flux as
`J_L+J_R` and chirality as `J_L-J_R`, propagates the two Laplacians
separately, and applies the same prescribed Hodge source to both.

Production does not contain:

- the Neumann preparation (3);
- the reversible history lift (16);
- the FTD-0928 reciprocal mismatch operator; or
- a derived normalization identifying `(X,Q)` with `(L,R)` or their
  common/relative modes.

The left/right architecture is therefore a natural candidate, not a derived
gearbox. Choosing an identification remains a separately priced selection
until action, source, energy, and observable normalizations all agree.

---

## 9. Epistemic ledger

| Claim | Status | Meaning |
|---|---|---|
| The self-dual companion is uniquely `-(2I-K)^-1 U` | `[THEOREM]` | Follows from the exact `2/9` gap |
| The registered companion has finite support | `[CLOSED NEGATIVE]` | Exact Laurent witness excludes it |
| A fixed finite causal depth forms the exact uncontained companion from compact data | `[CLOSED NEGATIVE]` | Finite causal support cannot equal the unique infinite-support profile |
| Radius-one iteration (3) converges with error (4) | `[THEOREM]` | Exact Neumann series and band bound |
| The preparation is restriction-consistent and `C4` covariant | `[THEOREM]` | Dependency-cone locality and commutation with rotations |
| The local history lift is canonical and reversible | `[REFERENCE CONSTRUCTION]` | Exact cotangent lift with outgoing history retained |
| Discarded outgoing history realizes reduced loss/unactualization | `[CONJECTURE]` | Precise mathematical candidate; physical environment identification open |
| The registered local history lift preserves positive quadratic energy | `[CLOSED NEGATIVE]` | Expanding eigenvalue excludes it |
| Existing left/right field pairs can store two field-shaped canonical sectors | `[ENGINE FACT]` | Representation capacity only |
| Existing left/right fields are physically `(X,Q)` | `[OPEN]` | Identity and normalization are not derived |
| The static halo has the same uniform geometric preparation as the dynamic companion | `[CLOSED NEGATIVE]` | Massless spectrum accumulates at zero |
| Positive reservoir work, fresh-port recycling, stopping, full formation, and recovery are closed | `[OPEN]` | No autonomous microdynamics yet |

---

## 10. Consequence

The most economical stable recursive architecture is no longer “matter plus
a clock.” It has three roles:

1. a manifested matter/current core generating the compact source;
2. a reciprocal self-dual field pair carrying the persistent dynamic profile;
3. an outward history/reservoir channel carrying the information and work
   displaced during formation.

The left/right substrate can represent role 2, but role 3 is not yet derived.
The next admissible step is a preregistered positive local reservoir/port
microdynamics that realizes equation (16) without its hyperbolic energy
defect, or proves that the existing field pairs cannot do so. Static-halo
formation must be tested as a separate massless transport front.

Until then, FTD has a unique causal PreparationMap and a reversible
information accounting, but not autonomous positive-energy formation.

