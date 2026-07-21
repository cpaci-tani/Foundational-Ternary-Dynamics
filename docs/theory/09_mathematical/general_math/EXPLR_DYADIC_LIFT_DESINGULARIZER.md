# EXPLR - Dyadic Lift Desingularizer

**Document type:** Exploratory mathematical note
**Status:** [THEOREM] for the finite dyadic lift and unknot statement;
[EXPLORATORY] for the state-space interpretation
**Companion seed note:** [EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md](EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md)
**Companion family note:** [EXPLR_DYADIC_TRIGONAL_RELAY_FAMILY.md](EXPLR_DYADIC_TRIGONAL_RELAY_FAMILY.md)
**Verifier:** `scripts/proofs/proof_dyadic_lift_desingularizer.py`

---

## 0. Result

The dense-looking plane curve is not intrinsically a self-crossing space
object. Its real plane crossings are a loss of phase information in the
two-coordinate readout.

For any finite dyadic cosine carrier

```text
x(t) = a_0 cos(t) + sum_{k=1}^m a_k cos(2^k t),   a_0 != 0,
```

define the one-coordinate phase lift

```text
z(t) = sin(t).
```

Then both

```text
B(t)     = (x(t), z(t)),
Gamma(t) = (x(t), y(t), z(t))
```

are regular embedded circles for every smooth second readout `y(t)`. Moreover
`Gamma` is isotopic through embeddings to the planar curve `B`, and therefore
has the unknot type.

For the seed `C_3`, this resolves every real plane self-intersection at once.
It is a readout result, not a statement about the complex projective
singularities of the implicit plane curve.

No FTD physics claim is promoted here.

---

## 1. Why this is the right lift

The plane curve stores the fundamental cosine `cos(t)` in `x`, but the paired
quadrature `sin(t)` is mixed into the alternating multi-frequency `y` readout.
The added height restores precisely that missing fundamental phase component:

```text
cos(t) plus sin(t) -> hidden phase t modulo 2*pi.
```

The higher dyadic terms do not obstruct this because every frequency after the
fundamental is even. Under the reflected phase `t -> pi-t`, all higher cosine
terms are fixed while the fundamental changes sign.

---

## 2. Exact embedding theorem

### 2.1 Injectivity

Suppose `B(s) = B(t)`. Equality of the second coordinate gives

```text
sin(s) = sin(t),
```

so on the phase circle either

```text
s = t
```

or

```text
s = pi - t.
```

The second case has the exact dyadic reflection rule

```text
x(pi-t) - x(t) = -2 a_0 cos(t).
```

Thus `x(pi-t)=x(t)` forces `cos(t)=0`. At either such phase,
`pi-t = t` modulo `2*pi`, so this apparent second case is not a distinct phase.
Therefore `B` is injective. Since `Gamma` retains the pair `(x,z)`, it is also
injective.

### 2.2 Regularity

If `cos(t) != 0`, then

```text
z'(t) = cos(t) != 0.
```

At the only remaining phases, `t=pi/2` and `3*pi/2`, every higher dyadic sine
vanishes and

```text
x'(pi/2)     = -a_0,
x'(3*pi/2)   =  a_0.
```

These are nonzero because `a_0 != 0`. Hence `B` and `Gamma` are regular.
A regular injective map from the compact circle into Euclidean space is an
embedding.

### 2.3 Unknot statement

The explicit deformation

```text
H_q(t) = (x(t), q y(t), sin(t)),   0 <= q <= 1,
```

keeps `(x(t), sin(t))` fixed. Every `H_q` is therefore an embedding. At
`q=0` the curve lies in the `x-z` plane, so the space curve is isotopic through
embeddings to a planar circle. It has the unknot type.

This theorem is intentionally modest: a plane readout can show a complicated
node network even when one phase-quadrature lift produces an unknotted curve.

---

## 3. Seed specialization

For `C_3`, the coordinates are

```text
x(t) = cos(t) + (1/2)cos(2t) + (1/2)cos(4t) + (3/8)cos(8t),
y(t) = 2sin(t) - sin(2t) + sin(4t) - (3/4)sin(8t).
```

Writing `u=cos(t)`, the carrier is `x(t)=X(u)`, with the exact odd-part rule

```text
X(u) - X(-u) = 2u.
```

The higher modes contribute only an even polynomial in `u`. The fundamental
mode supplies the entire odd part and is exactly what makes `z=sin(t)` a
universal crossing resolver for this family.

The 3D presentation is therefore

```text
Gamma_3(t) =
( cos(t) + (1/2)cos(2t) + (1/2)cos(4t) + (3/8)cos(8t),
  2sin(t) - sin(2t) + sin(4t) - (3/4)sin(8t),
  sin(t) ).
```

When this object seems to braid, fold, or momentarily self-intersect while
rotated on screen, that is a viewing/projection event. The curve itself has no
real self-crossing in this lift.

---

## 4. The exact high-dimensional picture

Define the normalized oscillator state

```text
Phi(t) = (exp(i t), exp(2 i t), exp(4 i t), exp(8 i t)) in T^4.
```

This is not a free four-dimensional torus trajectory. Its coordinates obey

the deterministic dyadic chain

```text
Phi_1 = Phi_0^2,
Phi_2 = Phi_1^2,
Phi_3 = Phi_2^2.
```

So it is a closed one-dimensional resonant orbit inside `T^4`, with the
fundamental phase still completely recoverable from the first coordinate. The
plane curve is the linear readout

```text
x(t) = sum_k a_k Re(Phi_k(t)),
y(t) = 2 sum_k (-1)^k a_k Im(Phi_k(t)).
```

It compresses eight real oscillator coordinates to two visible coordinates.
The vector space kernel of this readout has dimension six. A plane crossing
occurs exactly when a chord of the resonant orbit lies in that kernel.

This is the rigorous kernel behind the "tesseract shadow" intuition:

```text
one hidden phase
  -> four dyadically locked oscillator channels
  -> a severe two-coordinate projection
  -> visible folds, nodes, and apparent depth.
```

The precise object is a resonant `S^1` orbit in a four-oscillator state space,
not a literal tesseract and not evidence for a physical extra dimension.

---

## 5. Tubular surface corollary

**[THEOREM]** When the hypotheses of the lift theorem hold, `Gamma(S^1)` is a
smooth compact embedded circle. The tubular-neighborhood theorem therefore
supplies some radius `rho > 0` for which the normal-disk bundle is embedded.
For every `0 < r < rho`, its boundary

```text
S(t, delta) = Gamma(t) + r[N(t) cos(delta) + B(t) sin(delta)]
```

is an embedded surface diffeomorphic to `S^1 x S^1`. Here `(N,B)` is any
smooth transported orthonormal frame of the normal bundle. Since the
centerline is an unknot, this sufficiently thin tube is an unknotted solid
torus boundary.

This is an existence theorem, not an exact numerical value for `rho`. The
interactive atlas reports a **sampled** thickness diagnostic based on

```text
rho_sample = min(1/kappa_max, dcrit/2),
```

where `kappa_max` is sampled along the lifted centerline and `dcrit` is the
smallest sampled chord approximately orthogonal to both endpoint tangents.
The corresponding local surface-area Jacobian bound is

```text
J_min_sample = min_t r |Gamma'(t)| max(0, 1 - r kappa(t)).
```

A sampled pass is numerical evidence that the displayed tube lies within a
normal neighborhood. It is not a proof of the global reach, and a thick
torus-like rendering may intentionally lie outside this conservative regime.

---

## 6. What the lift does and does not settle

The lift establishes:

- all real plane self-intersections are projection artifacts once `z=sin(t)`
  is retained;
- the finite dyadic space curve has a simple topological type;
- complicated visual behavior can coexist with a one-parameter hidden state.

It does not establish:

- that the plane algebraic curve is nonsingular over the complex projective
  closure;
- a family formula for the `75+15+15=105` projective singularity budget;
- a physical hidden dimension, thermodynamic law, or FTD mechanism.

The next exact question is more refined: characterize all single-mode or
linear lifts that separate the plane fibers, and determine which lifts retain
the alternating-chiral relay as a visible symmetry.

---

## 7. Verification

Run:

```text
python scripts/proofs/proof_dyadic_lift_desingularizer.py
```

The verifier uses symbolic trigonometric and Chebyshev identities only. It
does not conduct a numerical near-miss search.

*End of document.*
