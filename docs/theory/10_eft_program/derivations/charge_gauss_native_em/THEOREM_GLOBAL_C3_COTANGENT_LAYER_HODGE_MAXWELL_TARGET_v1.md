# Global-C3 cotangent-layer Hodge-Maxwell target v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT CLOCK-LAYERED EDGE--FACE FIRST MOMENT]** +
**[SELECTION — GLOBAL C3 COTANGENT OWNERSHIP]** + **[OPEN — FINITE
LAYER-COVARIANT COLLISION, GAUSS/SOURCE/WORK CLOSURE]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_global_c3_cotangent_layer_hodge_maxwell_target.py](../../../../../scripts/proofs/proof_global_c3_cotangent_layer_hodge_maxwell_target.py)
performs 29,972 exact checks. It uses no fitted coefficient, measured target,
or numerical eigensolver.

---

## 1. Clock layer instead of a new record property

For $f=(d,n,h)$ let $c=d\times n$ and define the polar and axial triads

\[
 P=(d,hn,c),
 \qquad
 A=(n,hc,hd).                                      \tag{1}
\]

At global cotangent layer $q\in\mathbb Z_3$, read

\[
 E_q=P_q,
 \qquad
 B_q=A_q.                                          \tag{2}
\]

Each pair in equation (2) consists of perpendicular unit vectors and has the
correct polar/axial transformation law under all of $O_h$.

The shared-edge flag tick cycles the legs forward. Advancing the global layer
backward,

\[
 f\mapsto T f,
 \qquad q\mapsto q-1,                              \tag{3}
\]

gives

\[
 (E_{q-1},B_{q-1})(Tf)=(E_q,B_q)(f).               \tag{4}
\]

Thus $q$ can be supplied by the already-postulated global tick modulo three;
it need not be added as an independently variable per-record payload. This
ownership is a **[SELECTION]** until the common action proves how creation at
arbitrary ticks inherits and retains the layer convention.

## 2. Three exact transport layers

Each layer has 192 channels and a rank-seven readout

\[
 (n,E_x,E_y,E_z,B_x,B_y,B_z).                     \tag{5}
\]

Let $A_a^{(q)}$ be its exact first streaming moment along axis $a$.

### Layer 0: edge/longitudinal transport

The only nonzero entries are

\[
 A_a^{(0)}(n,E_a)=1,
 \qquad
 A_a^{(0)}(E_a,n)=\frac13.                         \tag{6}
\]

### Layer 1: edge--face curl

The only nonzero entries are

\[
 A_a^{(1)}(E_i,B_j)
 =A_a^{(1)}(B_j,E_i)
 =-\frac12\epsilon_{iaj}.                          \tag{7}
\]

This is the required antisymmetric spatial incidence, represented as a
symmetric-hyperbolic flux block.

### Layer 2: storage

\[
 A_a^{(2)}=0.                                      \tag{8}
\]

The zero first moment is an exact storage layer, not an assumed pause in the
underlying flag/phase permutation.

## 3. Three-tick Floquet target

The per-tick averaged generator is

\[
 \bar A_a=\frac13\sum_{q=0}^2 A_a^{(q)}.           \tag{9}
\]

For arbitrary $k$, its exact continuous generator has characteristic
polynomial

\[
 \boxed{
 \chi_{-i\bar A(k)}(\lambda)
 =\lambda
 \left(\lambda^2+\frac{|k|^2}{27}\right)
 \left(\lambda^2+\frac{|k|^2}{36}\right)^2.}      \tag{10}
\]

Equation (10) contains:

- two transverse electric--magnetic polarization pairs with speed $1/6$;
- one scalar--longitudinal pair with speed $1/(3\sqrt3)$; and
- one longitudinal magnetic zero mode.

On the vacuum transverse subspace, equation (7) is exactly the centered
Hodge-Maxwell target up to the derived clock-layer normalization $1/6$.

## 4. What remains open

This is a kinematic first-moment theorem, not yet a native electromagnetic
action. The next construction must:

1. select a reversible $O_h$-equivariant base collision preserving the
   layer-zero readout and commuting with the three-tick return $U^3$;
2. generate the other two collisions by clock conjugation and prove the
   complete layer-covariant inverse;
3. derive the exact three-tick product-reference Bloch kernel;
4. turn the scalar--longitudinal pair into Gauss plus charge continuity rather
   than an extra vacuum wave;
5. attach actualization source and capacity work without changing the curl
   coefficient; and
6. define a native dimensionless source-to-field response before comparing it
   with the fine-structure root.

