# Cotangent rank-30 common irreducibility and parity-index obstruction v1

**Date:** 2026-08-24

**Status:** **[THEOREM — FIXED-C4-QUADRATURE SCALAR COMMUTANT / LINEAR
IRREDUCIBILITY]** +
**[THEOREM — PARITY INDEX AND DIRECTIONAL RANK DEFECT]** + **[SCOPED
CLOSED NEGATIVE — CONSTANT MAXWELL/TENSOR SPLIT / SELECTED EXACT ISOTROPIC
LINEAR CONE]** + **[CONDITIONAL DIRAC PRICE]** + **[OPEN — REPAIRED
LAYER-COVARIANT SYMBOL AND MOMENTUM-DEPENDENT CONSTRAINT/GAUGE COMPLEX]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_rank30_common_irreducibility_parity_index_obstruction.py](../../../../../scripts/proofs/proof_cotangent_rank30_common_irreducibility_parity_index_obstruction.py)
performs 1,175 exact checks on the selected layer-zero, fixed-C4-quadrature
rank-thirty witness,
including an exact $2700\times900$ commutant system and all 98 registered
nonzero primitive wavevectors.

---

## 1. Question after the common-closure theorem

The
[common collision-closure theorem](THEOREM_COTANGENT_COMMON_MAXWELL_TENSOR_COLLISION_CLOSURE_PRICE_v1.md)
proved that the first selected tensor-curl witness cannot retain the original
seven-dimensional Maxwell/Gauss space on one C4 quadrature slice. Its
collision-invariant slice carrier has

\[
 \mathcal V=\mathcal V_T\oplus\mathcal V_M,
 \qquad \dim\mathcal V_T=20,
 \qquad \dim\mathcal V_M=10.                 \tag{1}
\]

Equation (1) is a direct sum under the zero-momentum collision. It does **not**
prove that streaming preserves two physical sectors. This theorem asks the
next exact question:

> Do the three first spatial moments on the common rank-thirty carrier retain
> a nontrivial, momentum-independent Maxwell/tensor decomposition or enough
> native constraints to isolate the desired modes?

The answer is no for a constant sector split on this fixed quadrature, and not
yet for the constraints. The phase-complete successor below shows that this
slice irreducibility must not be promoted to the full four-phase carrier.

---

## 2. Selected common first-moment representation

Let $R$ be the low-height integer row basis consisting of:

1. the ten tensor rows and their ten collided copies; and
2. the seven Maxwell rows plus the three independent collided magnetic copies.

For the selected involutive collision $C$, define

\[
 G=RR^{\mathsf T},\qquad
 \eta=G^{-1},\qquad
 C_R=RCR^{\mathsf T}\eta.                    \tag{2}
\]

Then

\[
 RC=C_RR,\qquad C_R^2=I_{30}.                \tag{3}
\]

Using the same selected five-hop C18 route schedule as the rank-twenty tensor
witness, the co-rotating first moments are

\[
 A_a=C_R^{-1}R D_a C R^{\mathsf T}\eta,
 \qquad a\in\{x,y,z\}.                       \tag{4}
\]

Each is self-adjoint in the positive energy metric:

\[
 A_a^{\mathsf T}\eta=\eta A_a.              \tag{5}
\]

The positivity follows because $G=RR^{\mathsf T}$ is positive definite for
the exact full-row-rank carrier.

---

## 3. Scalar commutant and linear irreducibility

A constant endomorphism $X$ preserves every spatial generator exactly when

\[
 [X,A_x]=[X,A_y]=[X,A_z]=0.                  \tag{6}
\]

Vectorizing (6) gives a $2700\times900$ exact rational system. Its rank is

\[
 \operatorname{rank}\mathcal C=899,          \tag{7}
\]

so

\[
 \dim\operatorname{Comm}(A_x,A_y,A_z)=1.     \tag{8}
\]

The identity is in that kernel, hence

\[
 \boxed{\operatorname{Comm}(A_x,A_y,A_z)=\mathbb R I_{30}.} \tag{9}
\]

Because the $A_a$ are jointly self-adjoint in a positive metric, any proper
common invariant subspace would have an invariant orthogonal complement and
therefore a nontrivial commuting orthogonal projector. Equation (9) excludes
one. Thus the selected first-moment representation is linearly irreducible.

In particular, there is no nontrivial constant idempotent

\[
 \Pi^2=\Pi,\qquad [\Pi,A_a]=0                 \tag{10}
\]

that separates the collision labels “Maxwell-10” and “tensor-20” after
streaming. Those labels describe collision closures, not invariant dynamical
sectors.

---

## 4. Inversion grading and its index

Spatial inversion does survive as an energy-orthogonal involution $P$:

\[
 P^2=I,\qquad P^{\mathsf T}\eta P=\eta,
 \qquad PA_a=-A_aP.                           \tag{11}
\]

Its projectors

\[
 P_\pm={I\pm P\over2}                         \tag{12}
\]

have unequal ranks

\[
 \boxed{\operatorname{rank}P_+=17,qquad
        \operatorname{rank}P_-=13.}           \tag{13}
\]

Thus every Bloch first moment

\[
 A(k)=k_xA_x+k_yA_y+k_zA_z                   \tag{14}
\]

is off diagonal between a 17-dimensional and a 13-dimensional parity space.
The dimension mismatch forces at least four zero modes. The registered exact
census is sharper:

| Primitive directions | Count | $\operatorname{rank}A(k)$ | Kernel parity split |
|---|---:|---:|---:|
| Non-FCC-defect directions | 86 | 26 | $4+0$ |
| FCC face diagonals $(\pm1,\pm1,0)$ and permutations | 12 | 24 | $5+1$ |

On the 86 generic registered directions both rectangular parity blocks have
rank thirteen. On the twelve FCC directions both fall to rank twelve. The
extra FCC zero pair is therefore a directional defect of this selected
schedule, not a polarization theorem.

Moreover,

\[
 \bigcap_{a=x,y,z}\ker A_a=\{0\},
 \qquad
 \bigcap_{a=x,y,z}\ker A_a^{\mathsf T}=\{0\}. \tag{15}
\]

The zero modes depend on momentum. There is no four-dimensional constant
subspace that can simply be removed from every voxel.

---

## 5. Physical-seed leakage

Embed the usual four-dimensional TT seed in the original tensor rows and a
four-dimensional transverse $(E,B)$ seed in the original Maxwell rows. Their
Krylov closures under $A(k)$ are:

| Direction | TT seed | Maxwell seed | Combined rank-eight seed |
|---|---:|---:|---:|
| Cubic axis $(1,0,0)$ | 8 | 10 | 18 |
| Body diagonal $(1,1,1)$ | 22 | 18 | 22 |
| Generic $(1,2,3)$ | 28 | 28 | 30 |

The generic combined seed is cyclic for the entire carrier. Consequently,
neither the desired Maxwell nor TT coordinates define a closed physical
subspace of this unconstrained first-moment action.

There is a stronger spectral obstruction. Put

\[
 x={\lambda^2\over |k|^2}.
\]

After removing the forced zero factors, the normalized nonzero characteristic
polynomials on a cubic axis and a body diagonal are, up to nonzero constants,

\[
 p_{100}(x)=
 (x-1)(2x-5)^2(2x-1)^4(3x^2-10x+4)^3,       \tag{16}
\]

\[
 \begin{aligned}
 p_{111}(x)={}&(x-2)(3x-4)^2(3x-1)^2\\
 &\times(81x^4-513x^3+846x^2-351x+38)^2.
 \end{aligned}                                \tag{17}
\]

Their exact polynomial gcd is

\[
 \boxed{\gcd_{\mathbb Q[x]}(p_{100},p_{111})=1.} \tag{18}
\]

An exact isotropic linear branch \(\lambda^2=c^2|k|^2\) would contribute the
minimal polynomial of \(c^2\) to both normalized spectra. Equation (18)
therefore excludes every such branch in this selected unconstrained
first-moment carrier. An invariant constraint or quotient cannot create an
eigenvalue absent from the parent operator. The generator itself must first
be repaired—through a layer-covariant composition, different schedule, or
larger finite action—before a constraint complex can isolate Maxwell and
spin-2 poles.

---

## 6. Conditional common constraint price

Two transverse Maxwell polarizations with their phase partners require four
phase-space dimensions. Two helicity-two tensor polarizations with their
partners require four more. The desired combined physical phase dimension is
therefore eight.

For a thirty-dimensional phase carrier, conditional Dirac counting gives

\[
 N_{\rm phys}=30-2F-S=8,
\]

or

\[
 \boxed{2F+S=22.}                             \tag{19}
\]

First-class-only reduction would require $F=11$. Even if the four generic
parity-index zero modes were eventually proven first class,

\[
 F=4\quad\Longrightarrow\quad S=14.           \tag{20}
\]

No such status is assigned here. A zero eigenvector of a first-moment matrix
is not by itself a constraint, a gauge generator, or a closed bracket.

---

## 7. Exact scope

### Closed negative

For the selected layer-zero, rank-thirty, one-record common witness:

1. the Maxwell-10 and tensor-20 collision closures are not separately
   invariant under the first-moment streaming generators;
2. no nontrivial momentum-independent linear projector separates them;
3. the four generic zero modes have no common fixed carrier subspace;
4. an unconstrained transverse-Maxwell plus TT seed leaks to all thirty modes
   at a generic registered wavevector; and
5. the normalized axis and body-diagonal spectra are coprime, excluding an
   exact isotropic linear cone in this selected first-moment generator.

### Not claimed

This theorem does not prove:

1. a layer-covariant three-tick rank-thirty action;
2. that the zero modes are first-class or second-class constraints;
3. a local gauge algebra or bracket;
4. isolated Maxwell or spin-2 poles;
5. a static gravitational response, universal sourcing, or lensing; or
6. a native electromagnetic coupling normalization.

It is scoped to the selected layer-zero, fixed-C4-quadrature linear witness. A
layer-covariant three-tick symbol may differ. The
[phase-complete common-closure theorem](THEOREM_COTANGENT_PHASE_COMPLETE_COMMON_CLOSURE_AND_C4_SELECTION_v1.md)
also shows that restoring both C4 quadratures raises the carrier to rank fifty
and restores an exact tensor-40/Maxwell-10 linear split by C4 type. The scalar
commutant here is therefore a slice result, not a theorem that the complete
native action linearly mixes those sectors.

Nonlinear occupation-dependent constraints, a multi-record chain complex,
staggered primal/dual fields, a different route schedule, and longer
reversible compilation remain open.

---

## 8. Next locked gate

The next candidate may not insert a constant Maxwell projector, TT projector,
phase slice, or four unexplained constraints. It must first choose honestly
between the complete rank-fifty C4 carrier and a derived phase-reality
quotient. It must then replace the selected one-layer generator by a finite,
layer-covariant symbol with the required direction-stable spectral factors
and provide local polynomial symbols $K(k)$ and $G(k)$ generated by that same
action such that:

1. $\ker K(k)$ is invariant under the full common evolution;
2. $G(k)$ generates a declared redundancy and its bracket closes;
3. the physical quotient has dimension eight on generic nonzero momenta;
4. any exceptional FCC rank defect is removed, constrained, or predicted;
5. the parent symbol and quotient contain distinct, direction-stable massless
   Maxwell and helicity-two poles;
6. the construction extends covariantly through all three clock layers; and
7. the same actualization/capacity vertex sources its static sector for the
   blind coupling and lensing observatories.

Until that gate passes, rank thirty is an exact fixed-quadrature capacity
price and obstruction—not the phase-complete unified physical action.
