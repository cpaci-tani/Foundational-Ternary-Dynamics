# EXPLR - Dyadic Lacunary Fourier Curve C3

**Document type:** Exploratory mathematical note
**Status:** [EXPLORATORY] for FTD; exact claims below are theorems of the explicitly defined curve only
**Provenance:** incorporated from a user-supplied technical note on the four-mode dyadic Fourier seed curve
**Related:** [EXPLR_DYADIC_LACUNARY_PHASE_RIBBONS.md](EXPLR_DYADIC_LACUNARY_PHASE_RIBBONS.md), [EXPLR_FOURIER_CURVE_LEVEL_4.md](EXPLR_FOURIER_CURVE_LEVEL_4.md), [REF_QCR_TRILOGY_BRIDGE.md](REF_QCR_TRILOGY_BRIDGE.md), [EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md](../algebra/EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md)

---

## 0. Summary

Define the seed curve

```text
C_3(t) = (x_3(t), y_3(t)),       0 <= t <= 2*pi,

x_3(t) = cos t + (1/2) cos 2t + (1/2) cos 4t + (3/8) cos 8t,
y_3(t) = 2 sin t - sin 2t + sin 4t - (3/4) sin 8t.
```

Equivalently, with

```text
a_0 = 1,    a_1 = 1/2,    a_2 = 1/2,    a_3 = 3/8,
```

the curve is

```text
C_3(t) =
sum_{k=0}^{3} a_k ( cos(2^k t), 2(-1)^k sin(2^k t) ).
```

The support is finite, dyadic, lacunary, and Fourier:

```text
finite dyadic recurrence
  -> regular analytic parametrization
  -> algebraic plane image
  -> branch overlap
  -> signed-area cancellation
  -> scale-localized curvature events.
```

The alternating factor `(-1)^k` matters: consecutive dyadic modes alternate
orientation. This is an alternating-chirality epicycle system, not merely a
sum of scalar waves.

**Important precision note:** the parametrization is regular and analytic, but
the plane image self-intersects. Thus the image should not be called a
nonsingular plane algebraic curve in the algebraic-geometry sense; it is a
degree-16 algebraic image with transverse double points.

---

## 1. Basic properties

| Property | Value |
|---|---|
| Domain | `t in [0, 2*pi]` |
| Frequencies | `1, 2, 4, 8` |
| Frequency law | `2^k` |
| Period | `2*pi` |
| Regularity | real analytic, regular immersion; no cusps |
| Closed | yes |
| Simple/Jordan | no |
| Self-intersecting | yes |
| Reflection symmetry | across the x-axis |
| Signed area | `3*pi/4` |
| Signed centroid | `(0, 0)` |
| Turning number | `-2` |
| Approximate arc length | `34.6833859266` |
| Approximate x-range | `[-1.2068214204, 2.375]` |
| Approximate y-range | `[-4.1217588145, 4.1217588145]` |

The symmetry is immediate:

```text
x_3(-t) = x_3(t),       y_3(-t) = -y_3(t),
```

so

```text
C_3(-t) = (x_3(t), -y_3(t)).
```

---

## 2. Algebraic reduction

Let

```text
u = cos t.
```

By Chebyshev angle-doubling,

```text
cos 2t = 2u^2 - 1,
cos 4t = 8u^4 - 8u^2 + 1,
cos 8t = 128u^8 - 256u^6 + 160u^4 - 32u^2 + 1.
```

Substitution gives

```text
x = X(u)
  = 48u^8 - 96u^6 + 64u^4 - 15u^2 + u + 3/8.
```

For the sine part, use

```text
sin(nt) = sin(t) U_{n-1}(cos t),
```

where `U_n` is a Chebyshev polynomial of the second kind. Then

```text
y = +/- sqrt(1 - u^2) P(u),
```

with

```text
P(u) = -96u^7 + 144u^5 - 52u^3 + 2.
```

Thus

```text
x = X(u),
y^2 = (1 - u^2) P(u)^2.
```

The visible plane curve is generated from one hidden scalar `u` with two
visible branches:

```text
u -> (x, +y),       u -> (x, -y).
```

Eliminating `u` gives an implicit equation

```text
F(x, y) = Res_u( x - X(u), y^2 - (1-u^2)P(u)^2 ) = 0.
```

For this seed curve, `F` has total degree 16. This makes `C_3` an exact
algebraic-plane-curve parametrization rather than a numerical drawing.

---

## 3. Regularity: no cusps

Differentiate:

```text
x_3'(t) = -sin t - sin 2t - 2 sin 4t - 3 sin 8t,
y_3'(t) =  2 cos t - 2 cos 2t + 4 cos 4t - 6 cos 8t.
```

In the variable `u = cos t`,

```text
x_3'(t) = -sin(t) R(u),
```

where

```text
R(u) = 384u^7 - 576u^5 + 256u^3 - 30u + 1,
```

and

```text
y_3'(t) = S(u),
```

where

```text
S(u) = -768u^8 + 1536u^6 - 928u^4 + 156u^2 + 2u.
```

At the endpoints where `sin t = 0`,

```text
y_3'(0) = -2,       y_3'(pi) = -6.
```

Away from those endpoints, a cusp would require `R(u) = S(u) = 0`. Exact
elimination gives

```text
Res_u(R, S) = 60514543933804184076288 != 0.
```

Therefore `R` and `S` have no common root, and

```text
C_3'(t) != 0
```

for all `t`. The sharp-looking features are high-curvature events, not
singularities of the parametrization.

---

## 4. Signed area and centroid

For a Fourier curve

```text
x(t) = sum_n a_n cos(nt),
y(t) = sum_n b_n sin(nt),
```

the signed area is

```text
A = (1/2) integral_0^{2*pi} (x y' - y x') dt
  = pi sum_n n a_n b_n.
```

For `C_3`:

| `n` | `a_n` | `b_n` | `n a_n b_n` |
|---:|---:|---:|---:|
| 1 | `1` | `2` | `2` |
| 2 | `1/2` | `-1` | `-1` |
| 4 | `1/2` | `1` | `2` |
| 8 | `3/8` | `-3/4` | `-9/4` |

Therefore

```text
A(C_3) = pi(2 - 1 + 2 - 9/4) = 3*pi/4.
```

The area is a residual after cancellation:

```text
2*pi - pi + 2*pi - 9*pi/4 = 3*pi/4.
```

The signed centroid is also exact:

```text
(xbar, ybar) = (0, 0).
```

---

## 5. Derivative-energy hierarchy

Define

```text
E_m(C_3) = integral_0^{2*pi} |C_3^{(m)}(t)|^2 dt.
```

For a Fourier curve,

```text
E_m = pi sum_n n^{2m}(a_n^2 + b_n^2).
```

Here `b_n = 2(-1)^k a_n` at `n = 2^k`, so

```text
a_n^2 + b_n^2 = 5a_n^2.
```

Hence

```text
E_m =
pi [
  5*1^{2m}
  + (5/4)*2^{2m}
  + (5/4)*4^{2m}
  + (45/64)*8^{2m}
].
```

Concrete values:

| Energy | Exact value |
|---|---:|
| `E_0` | `525*pi/64` |
| `E_1` | `75*pi` |
| `E_2` | `3225*pi` |
| `E_3` | `189525*pi` |

The modal ledger is:

| Mode | Position weight | Velocity weight | Acceleration weight |
|---:|---:|---:|---:|
| 1 | `5` | `5` | `5` |
| 2 | `5/4` | `5` | `20` |
| 4 | `5/4` | `20` | `320` |
| 8 | `45/64` | `45` | `2880` |

The `8`-mode contributes only about `8.57%` of position energy but about
`89.30%` of acceleration energy. In this precise sense:

```text
low modes define the body; high modes define the events.
```

---

## 6. Turning number and branch overlap

Let

```text
z(t) = x_3(t) + i y_3(t),       w = exp(it).
```

Multiplying the Laurent polynomial for `z'(t)` by `w^8` gives, up to a
non-zero scalar, the ordinary polynomial

```text
P_turn(w) =
-1/2 (
  3w^16 - 6w^12 + w^10 - 3w^9
  - w^7 + 3w^6 - 2w^4 + 9
).
```

By the argument principle, the tangent winding is

```text
N_inside - 8,
```

where `N_inside` is the number of zeros of `P_turn` inside the unit disk.
Numerical root isolation gives

```text
N_inside = 6,
turn(C_3) = 6 - 8 = -2.
```

The curve is not injective. The three non-axis branch overlaps occur at:

| `x` | `y` |
|---:|---:|
| `-0.8698817483` | `+/- 3.0133587694` |
| `-0.2385784805` | `+/- 0.8264600995` |
| `0.1742356444` | `+/- 0.6035699772` |

The three x-axis double points come from the roots of `P(u)` in `(-1, 1)`:

| `u` | `x = X(u)` |
|---:|---:|
| `0.3988454957` | `-0.3484712888` |
| `0.6711271358` | `0.4771569609` |
| `0.9785337968` | `1.7397634967` |

Together these give nine visible transverse self-intersection points. The
conceptual point is:

```text
same visible point != same internal phase.
```

The external readout `(x, y)` does not uniquely determine the hidden parameter.

---

## 7. Finite family

Define the finite dyadic family

```text
C_K(t) =
(
  sum_{k=0}^{K} a_k cos(2^k t),
  2 sum_{k=0}^{K} (-1)^k a_k sin(2^k t)
).
```

For every finite `K`, `C_K` is a closed real-analytic map, and its image is
algebraic: every `cos(2^k t)` and `sin(2^k t)` reduces through Chebyshev
polynomials in `u = cos t`.

Regularity is coefficient-dependent. The specific four-mode seed `C_3` is
regular by the resultant check in Section 3.

---

## 8. Infinite Weierstrass-like extension

Define

```text
C_infty(t) =
(
  sum_{k=0}^{infty} a_k cos(2^k t),
  2 sum_{k=0}^{infty} (-1)^k a_k sin(2^k t)
).
```

Use the seed amplitudes

```text
a_0 = 1,       a_1 = 1/2,       a_2 = 1/2,       a_3 = 3/8,
```

and impose the geometric tail

```text
a_k = (3/8) lambda^{k-3},       k >= 4,       0 < lambda < 1.
```

Uniform convergence follows from `sum |a_k| < infinity`, so

```text
0 < lambda < 1  =>  C_infty is continuous.
```

The derivative introduces a factor `2^k`; absolute derivative convergence is
controlled by `sum 2^k |a_k|`, hence

```text
lambda < 1/2  =>  C_infty has an absolutely convergent derivative.
```

For `lambda > 1/2`, the derivative terms no longer decay. This is the onset
of the classical lacunary/Weierstrass mechanism, but by itself it is not a
proof of nowhere differentiability.

For signed area,

```text
A_infty = 2*pi sum_{k=0}^{infty} (-1)^k 2^k a_k^2.
```

With the tail above,

```text
A_infty =
2*pi [
  3/8 + (9/4) lambda^2 / (1 + 2 lambda^2)
],
```

provided `lambda < 1/sqrt(2)`. Thus:

| Regime | Condition | Meaning |
|---|---|---|
| Smooth/tame | `0 < lambda < 1/2` | derivative-controlled |
| Rough but area-controlled | `1/2 < lambda < 1/sqrt(2)` | Weierstrass-like while signed area still converges |
| Rough and area-unstable | `1/sqrt(2) <= lambda < 1` | continuity may remain, but the area ledger fails |

Derivative energies obey

```text
E_m = 5*pi sum_{k=0}^{infty} 2^{2mk} a_k^2,
```

so, for `a_k ~ lambda^k`, convergence requires

```text
lambda < 2^{-m}.
```

| Energy | Converges if |
|---|---|
| Position energy `E_0` | `lambda < 1` |
| Velocity energy `E_1` | `lambda < 1/2` |
| Acceleration energy `E_2` | `lambda < 1/4` |
| Jerk energy `E_3` | `lambda < 1/8` |

Higher derivatives fail at increasingly smaller tail amplitudes.

---

## 9. Placement in FTD documentation

This note is filed as exploratory pure mathematics. It does not introduce a
new LEDGER claim and does not promote any FTD physics claim.

Its useful roles are narrower:

1. It gives a clean four-mode dyadic reference curve with exact signed
   invariants.
2. It separates finite algebraicity from infinite lacunary roughness.
3. It records branch overlap as a precise hidden-parameter phenomenon.
4. It provides a sibling reference to `EXPLR_FOURIER_CURVE_LEVEL_4.md`.

This `C_3` seed is not identical to the level-4 curve in
`EXPLR_FOURIER_CURVE_LEVEL_4.md`: that document uses the curve
`(x(t), -x'(t))`. Both share dyadic support and the signed area `3*pi/4`,
but the `y`-coefficients and geometric mechanisms differ.

---

## 10. What this document does NOT claim

- It is **not** a derivation of an FTD physical constant.
- It is **not** a derivation of `N_c = 3`.
- It is **not** an alpha-readout mechanism.
- It is **not** a modular-form or L-function identification.
- It is **not** a claim that every finite dyadic coefficient choice is regular.
- It is **not** evidence for an infinite completed lattice ontology; the
  infinite extension is a classical analytic comparison object only.

---

## 11. Verification notes

The following checks were performed while incorporating the note:

| Claim | Verification |
|---|---|
| Algebraic reduction | exact Chebyshev substitution |
| Implicit degree | exact resultant; total degree 16 |
| Regularity | exact resultant `Res(R,S) != 0` plus endpoint checks |
| Area | Fourier orthogonality; exact `3*pi/4` |
| Centroid | exact signed centroid numerators vanish |
| Turning number | argument-principle polynomial has 6 roots inside the unit disk |
| Trigonal node relay | exact symmetric-polynomial reduction; x-axis branch collapses generate off-axis nodes by a `+/- 2*pi/3` phase move |
| Weierstrass thresholds | exact ratios `2lambda`, `2lambda^2`, and `4^m lambda^2`; Holder threshold `H=1/2` coincides with signed-area failure |
| Arc length/ranges/self-intersections | numerical verification of stated approximations |

The repeatable verifier is:

```text
python scripts/proofs/proof_dyadic_lacunary_fourier_curve.py
```

---

## 12. Deep FTD probe

This section probes what the curve can honestly contribute to FTD-style
thinking. The answer is narrow but useful: `C_3` is a finite readout benchmark
for hidden phase, dyadic recurrence, and cancellation ledgers. It is not a
substrate derivation.

### 12.1 Probe contract

Allowed:

- exact algebraic consequences of the stated coefficients;
- structural obstructions and boundary statements;
- explicit finite/infinite comparisons;
- analogies tagged as analogies.

Forbidden:

- numerical near-miss searches;
- interpreting shared integers as physical identifications;
- treating a parametric curve as a lattice update rule;
- promoting `N_c`, alpha, generations, or Moore structure from this curve.

### 12.2 Hidden Fibonacci velocity spine

Although the curve is written with amplitudes

```text
a_0, a_1, a_2, a_3 = 1, 1/2, 1/2, 3/8,
```

the derivative-weighted amplitudes are

```text
2^k a_k = 1, 1, 2, 3.
```

Thus, for the four retained modes,

```text
a_k = F_{k+1} / 2^k,       k = 0,1,2,3,
```

where `F_1,F_2,F_3,F_4 = 1,1,2,3`.

This is not a discovery of a new physical law. With four data points it is a
low-complexity exact description, best tagged:

```text
[STRUCTURAL OBSERVATION] finite seed only.
```

But it does give a sharp finite/infinite contrast. If one adopts the Fibonacci
continuation

```text
a_k = F_{k+1}/2^k,
```

then asymptotically

```text
a_k ~ (phi/2)^k / sqrt(5),
```

so the effective lacunary tail parameter is

```text
lambda_eff = phi/2.
```

This lies above both critical thresholds:

```text
phi/2 > 1/2,
phi/2 > 1/sqrt(2).
```

Therefore the Fibonacci continuation would be continuous (`phi/2 < 1`) but
derivative-rough and signed-area-unstable. Equivalently, the modal area terms

```text
(-1)^k F_{k+1}^2 / 2^k
```

do not even tend to zero, since their magnitude grows at asymptotic ratio

```text
phi^2 / 2 > 1.
```

FTD-facing interpretation:

```text
finite truncation is load-bearing.
```

The seed's exact cancellations are meaningful only because the recurrence is
cut off at four modes. The infinite Fibonacci reading is not a better FTD
object; it is precisely the boundary where the area ledger loses control.

### 12.3 Uniform 3:1 chirality ledger

Write each mode in complex form:

```text
a_n cos(nt) + i b_n sin(nt)
  = c_+(n) e^{int} + c_-(n) e^{-int},

c_+ = (a_n + b_n)/2,
c_- = (a_n - b_n)/2.
```

For this seed,

```text
b_{2^k} = 2(-1)^k a_k.
```

Hence

| `k` | Dominant orientation | `c_+` | `c_-` | Magnitude ratio |
|---:|---|---:|---:|---:|
| 0 | forward | `3a_k/2` | `-a_k/2` | `3:1` |
| 1 | backward | `-a_k/2` | `3a_k/2` | `3:1` |
| 2 | forward | `3a_k/2` | `-a_k/2` | `3:1` |
| 3 | backward | `-a_k/2` | `3a_k/2` | `3:1` |

So the curve has a uniform chirality bias: every dyadic mode has the same
dominant/subdominant magnitude ratio, and the dominant orientation alternates.

The signed-area contribution of each mode is exactly the chirality imbalance:

```text
pi n ( |c_+|^2 - |c_-|^2 ) = pi n a_n b_n.
```

This turns the area identity

```text
A(C_3) = 3*pi/4
```

into a chirality ledger:

```text
forward surplus - backward surplus + forward surplus - backward surplus.
```

FTD-facing interpretation:

```text
the curve is a clean finite model of alternating orientation plus residual
global invariant.
```

That is the honest analogy. It does not imply that FTD's physical chiral
sectors are derived from this curve.

### 12.4 Trigonal node relay

The nine visible self-intersection points are not an unstructured numerical
list. They split into:

1. three x-axis branch-collapse points, where `P(r)=0` and the `+y/-y`
   branches coincide at `y=0`;
2. three off-axis overlap pairs, with both signs of `y`.

These two groups are exactly linked.

Let `u` and `v` be two distinct hidden cosine-values. Off-axis overlap requires

```text
X(u) = X(v),
Q(u) = Q(v),       Q(u) = (1-u^2)P(u)^2.
```

Dividing both equations by `u-v` and writing

```text
s = u + v,
p = uv,
```

gives two symmetric polynomial equations `A(s,p)=0` and `B(s,p)=0`. The
first is

```text
A(s,p) =
-192p^3s + 480p^2s^3 - 288p^2s
-288ps^5 + 384ps^3 - 128ps
+48s^7 - 96s^5 + 64s^3 - 15s + 1.
```

Now impose the phase-relay relation

```text
p = s^2 - 3/4.
```

Then exact reduction gives

```text
A(s, s^2 - 3/4) = 48s^7 - 72s^5 + 26s^3 + 1
                = P(-s)/2,
```

and

```text
B(s, s^2 - 3/4)
```

is divisible by the same factor. Therefore every x-axis branch-collapse root
`r` of

```text
P(r) = -96r^7 + 144r^5 - 52r^3 + 2 = 0
```

generates an off-axis overlap pair by setting `s=-r` and

```text
uv = r^2 - 3/4.
```

Equivalently, if `r = cos(theta)`, then the off-axis hidden values are

```text
u = cos(theta + 2*pi/3),
v = cos(theta - 2*pi/3).
```

because

```text
u + v = -r,
uv = r^2 - 3/4.
```

This is the cleanest finite branch grammar found in the probe:

```text
x-axis collapse seed
  -> +/- 2*pi/3 hidden-phase relay
  -> off-axis visible node pair.
```

FTD-facing interpretation:

```text
threefold structure appears as a phase-relay grammar internal to the curve.
```

That is a real mathematical result. It is still not a derivation of color,
generations, or Moore decomposition. The "3" here is a finite trigonometric
branch mechanism.

### 12.5 Finite two-clock grammar

The deeper probe exposes two independent finite clocks:

| Clock | Formula | Meaning inside the curve |
|---|---|---|
| Orientation clock | `(-1)^k` | dyadic levels alternate dominant rotation |
| Relay clock | `t -> t +/- 2*pi/3` | x-axis collapse seeds generate off-axis node pairs |

The first is a `2`-pattern in mode index. The second is a `3`-pattern in
hidden phase. Their coexistence gives a mathematical `2 x 3` branch grammar:

```text
two orientations across three phase relays.
```

This is close in flavor to the QCR-trilogy bridge's `6 = 2 x 3` sector
language, but the status is deliberately lower:

```text
[STRUCTURAL OBSERVATION] for this curve only.
```

The curve does not prove that FTD has six physical sectors. It supplies a
finite toy grammar in which orientation alternation and threefold hidden-phase
relay can coexist without contradiction.

### 12.6 Rational normalization and readout degeneracy

Set `w = e^{it}`. Then `cos(2^k t)` and `sin(2^k t)` are Laurent polynomials
in `w`, so

```text
w -> (x(w), y(w))
```

is a rational map from the phase parameter to the plane. The plane image has
degree 16, but its normalization is genus zero because it is parametrized by
one rational phase variable.

This gives the cleanest FTD-facing lesson:

```text
complicated visible geometry can be the image of a simple hidden state.
```

The self-intersections are not defects in the hidden phase. They are failures
of the visible readout `(x,y)` to distinguish internal phase. This is directly
compatible with FTD's type-priority caution: visible content does not, by
itself, reconstruct the context/type that generated it.

### 12.7 Event localization

The energy hierarchy already proves the scale-localization principle:

```text
mode 8 contributes 8.57% of position energy
but 89.30% of acceleration energy.
```

Numerically, the largest curvature events occur at speed minima. The top
curvature peaks, paired by x-axis symmetry, begin at approximately:

| Curvature | `t` | `(x,y)` | Speed |
|---:|---:|---:|---:|
| `3751.7290` | `0.55548` | `(0.66901, 1.67710)` | `0.12355` |
| `2266.3881` | `2.65303` | `(-1.06039, 0.31979)` | `0.14591` |
| `621.9363` | `1.53992` | `(0.39163, 1.99752)` | `0.22481` |

FTD-facing interpretation:

```text
high modes barely move the body but dominate events.
```

This is a structural analogy to finite-resolution readout, not evidence for
any particular FTD interaction term.

### 12.8 Negative findings

The probe does not find a disciplined route from this curve to:

- `x_+ = 1/alpha`;
- `N_c = 3`;
- three generations;
- Moore-neighborhood decomposition;
- a native FTD update law;
- a Bell/QM mechanism;
- a dimensional calibration.

The strongest honest status is:

```text
[EXPLORATORY STRUCTURAL BENCHMARK]
```

The curve is valuable because it is exact, finite, dyadic, branch-rich, and
cancellation-rich. Those are FTD-flavored mathematical properties. They do not
make it an FTD derivation.

---

## 13. Fractal / Weierstrass probe

The infinite extension is where the seed becomes a genuine lacunary-analysis
object. This section records the regularity, variation, area, and dimension
facts that follow from the geometric tail.

### 13.1 Tail notation

For the tail, write

```text
a_k = A lambda^{k-3},       A = 3/8,       k >= 3.
```

The tail contribution has frequencies `2^k`, amplitudes proportional to
`lambda^k`, and mode vectors

```text
g_k(t) = a_k (cos(2^k t), 2(-1)^k sin(2^k t)).
```

The derivative amplitude scale is

```text
2^k a_k ~ (2 lambda)^k,
```

and the signed-area term scale is

```text
2^k a_k^2 ~ (2 lambda^2)^k.
```

These two ratios are the whole story:

```text
derivative roughness ratio = 2 lambda,
area convergence ratio     = 2 lambda^2.
```

### 13.2 Holder regularity

For `1/2 < lambda < 1`, define

```text
H(lambda) = -log(lambda)/log(2).
```

Equivalently,

```text
lambda = 2^{-H}.
```

The standard dyadic split proof gives the tail estimate

```text
|C_infty(t+h) - C_infty(t)| <= const(lambda) |h|^H.
```

Sketch: choose `N` with `2^{-(N+1)} < |h| <= 2^{-N}`. Low frequencies
`k <= N` are bounded by the derivative estimate

```text
|h| sum_{k<=N} 2^k a_k,
```

and high frequencies `k>N` by the amplitude estimate

```text
sum_{k>N} a_k.
```

For `lambda > 1/2`, both are of order `lambda^N`, hence of order `|h|^H`.

Thus:

| Range | Regularity |
|---|---|
| `0 < lambda < 1/2` | `C^1`; in fact `C^m` whenever `lambda < 2^{-m}` |
| `lambda = 1/2` | critical Zygmund-type bound `|h| log(1/|h|)` |
| `1/2 < lambda < 1` | `H(lambda)`-Holder |

The same Fourier coefficient scale also blocks better generic Holder
exponents for scalar components: the coefficient at frequency `n=2^k` is
`~n^{-H}`. A `beta`-Holder scalar function with `beta>H` would force faster
coefficient decay. So `H` is the natural exponent, not merely an artifact of
the proof.

### 13.3 Variation and length

For `lambda < 1/2`, the derivative series converges absolutely, so the curve
has finite length.

For `lambda > 1/2`, the scalar `x`-component cannot have bounded variation.
Reason: a bounded-variation periodic function has Fourier coefficients
`O(1/n)`, but this tail has

```text
n a_n = 2^k a_k ~ (2 lambda)^k,
```

which is unbounded when `2 lambda > 1`. Therefore the plane curve has infinite
length for `lambda > 1/2`.

At `lambda = 1/2`, this coefficient test is inconclusive: `n a_n` stays
bounded. Classical Weierstrass theory treats this as a critical rough boundary,
but this document does not claim an internal proof of infinite length at the
critical point.

### 13.4 Signed area stability

The signed-area tail is controlled by

```text
sum (-1)^k 2^k a_k^2.
```

With the geometric tail, the magnitude ratio is

```text
2 lambda^2.
```

Therefore signed area converges absolutely exactly when

```text
lambda < 1/sqrt(2).
```

This is the important split:

```text
roughness begins at lambda = 1/2,
area failure begins at lambda = 1/sqrt(2).
```

So there is a nonempty rough-but-area-controlled regime:

```text
1/2 < lambda < 1/sqrt(2).
```

### 13.5 Dimension bounds

For `1/2 < lambda < 1`, the parameterized image is `H`-Holder with

```text
H = -log(lambda)/log(2).
```

A Holder map from a one-dimensional parameter set to the plane has Hausdorff
dimension bounded by

```text
dim(image) <= min(2, 1/H).
```

The cap `1/H = 2` occurs exactly at

```text
H = 1/2
```

which is equivalent to

```text
lambda = 1/sqrt(2).
```

That is the same threshold at which the signed-area ledger fails. This is a
real structural alignment:

```text
area-stability boundary
  = Holder image-dimension plane-cap boundary.
```

But it is only a boundary alignment. This note does **not** claim that the
image dimension equals `1/H`, nor that the curve becomes plane-filling. It
only proves the upper-bound transition.

For scalar graphs such as `t -> x(t)`, classical Weierstrass theory often
gives graph dimension

```text
dim graph = 2 - H
```

under nondegeneracy hypotheses. That is an external theorem template, not an
FTD-side result established here.

### 13.6 Phase diagram

| Regime | Condition | Regularity | Length / variation | Signed area | Dimension bound |
|---|---|---|---|---|---|
| Smooth finite curve | `0 < lambda < 1/2` | `C^1` | finite length | converges | image dimension `1` |
| Critical derivative boundary | `lambda = 1/2` | `h log(1/h)` type | not settled here | converges | near-H=`1` boundary |
| Rough but area-controlled | `1/2 < lambda < 1/sqrt(2)` | `H`-Holder, `H>1/2` | infinite length | converges absolutely | `<2` upper bound |
| Area-critical roughness | `lambda = 1/sqrt(2)` | `H=1/2` | infinite length | fails | plane-cap upper bound |
| Area-unstable roughness | `1/sqrt(2) < lambda < 1` | `H<1/2` | infinite length | fails | capped by `2` |

### 13.7 FTD-facing interpretation

The infinite Weierstrass probe says:

```text
finite truncation protects the ledger.
```

The four-mode seed can carry exact signed area, exact branch grammar, and exact
high-curvature events because the high-frequency hierarchy is stopped. If the
same dyadic logic is continued with slow enough decay, the curve leaves the
finite-algebraic world and enters a rough analytic world.

This is FTD-relevant as a boundary lesson, not as an ontology change:

- finite dyadic recurrences can be algebraic and ledger-controlled;
- infinite dyadic continuations can be continuous while losing length control;
- roughness can begin before signed area fails;
- the signed-area failure threshold coincides with the Holder
  image-dimension cap threshold;
- none of this licenses completed-infinity lattice ontology inside FTD.

---

## 14. Geometric intuition pass

This section records a deliberately non-promotional geometry reading. It is
not a new theorem layer; it is a disciplined intuition scaffold for future
exact probes.

### 14.1 Projection / aliasing machine

The hidden object is simple:

```text
t in S^1.
```

The visible object is complicated because the readout folds that phase circle
through four dyadic elliptical modes:

```text
one smooth hidden phase
  -> two visible Chebyshev branches in u = cos(t)
  -> finite transverse node network
  -> residual signed area after alternating chirality cancellation.
```

So `C_3` is best viewed as a projection/aliasing machine. The plane point
does not remember enough to reconstruct the hidden phase. This is the same
lesson already visible in the branch-overlap data, now phrased geometrically.

### 14.2 Finite algebraic world vs rough analytic world

Finite `C_3` lives in the algebraic world:

- regular analytic immersion;
- degree-16 algebraic image;
- finitely many visible real nodes;
- exact signed area;
- exact turning number `-2`;
- exact branch-relay grammar.

The infinite tail changes category. The source remains a circle and the image
remains continuous for `lambda < 1`, but the readout is no longer a finite
algebraic curve. Above `lambda = 1/2`, wrinkles shrink too slowly to preserve
tangent control; the image becomes coastline-like: continuous, infinite length,
and without a stable tangent field.

The `lambda = 1/sqrt(2)` threshold is especially geometric. Since it is the
point where

```text
H(lambda) = 1/2,
```

it is also the Young/rough-path area boundary: below it, the path is rough but
its signed area is still canonically controlled by Fourier summation; at and
above it, area requires extra second-order data rather than being a clean
ledger of the path alone.

### 14.3 Exact structures that matter most

The strongest exact structures found so far are:

1. **Trigonal node relay.** Axis branch-collapse roots generate off-axis node
   pairs by the hidden-phase move `theta -> theta +/- 2*pi/3`.
2. **Chirality ledger.** Every mode has a `3:1` rotating-amplitude imbalance,
   and the dominant orientation alternates by dyadic level.
3. **Rational normalization.** A degree-16 plane curve with genus-zero
   normalization should hide a large singularity budget over its complex
   projective completion. The nine real nodes are likely only the real-visible
   part of the algebraic singularity story.

### 14.4 Exact follow-up probes

The next worthwhile exact probes are:

1. Compute the full projective singular locus of the degree-16 implicit curve:
   real, complex, and points at infinity. Reconcile this with the genus
   formula.
2. Build the exact self-intersection chord diagram on `S^1`, then compute
   immersed-plane-curve invariants such as rotation number and Arnold-style
   invariants.
3. Decompose the full branch-overlap correspondence in `(u,v)`, isolating the
   trigonal relay factor from the residual complex factors.
4. Prove the infinite-tail regularity sharply: exact Holder exponent,
   differentiability obstructions, and projection-wise nondegeneracy.
5. Reformulate the `lambda = 1/sqrt(2)` boundary using Young integration or
   rough-path language, so the area threshold is geometric rather than only
   Fourier-bookkeeping.

### 14.5 Guardrail

This intuition pass adds no LEDGER row and promotes no FTD physics claim. The
valid takeaway is:

```text
C_3 is a finite, exact, dyadic readout benchmark whose geometry teaches how a
simple hidden phase can project to branch-rich visible structure.
```

---

## 15. Projective singularity budget

The geometric-intuition pass predicted that the nine real nodes should be only
the visible part of a much larger projective singularity budget. This section
turns that into an exact accounting.

The verifier is:

```text
python scripts/proofs/proof_dyadic_curve_singularity_budget.py
```

### 15.1 Homogeneous phase parametrization

Use the complex-linear coordinate

```text
v = i y.
```

This does not change singularity invariants; it only removes explicit `i`
from the Laurent parametrization.

With `w = exp(it)`, both affine coordinates have common denominator
`16w^8`:

```text
x(w) = N_x(w)/(16w^8),
v(w) = N_v(w)/(16w^8),
```

where

```text
N_x(w) =
3(w^16+1) + 4(w^12+w^4) + 4(w^10+w^6) + 8(w^9+w^7),

N_v(w) =
-6(w^16-1) + 8(w^12-w^4) - 8(w^10-w^6) + 16(w^9-w^7).
```

The corresponding homogeneous map `P^1 -> P^2` is:

```text
[S:T] ->
[
  3(S^16+T^16)
  +4(S^12T^4+S^4T^12)
  +4(S^10T^6+S^6T^10)
  +8(S^9T^7+S^7T^9),

  -6(S^16-T^16)
  +8(S^12T^4-S^4T^12)
  -8(S^10T^6-S^6T^10)
  +16(S^9T^7-S^7T^9),

  16S^8T^8
].
```

It has degree 16 and no base point. Since the implicit image also has total
degree 16, the parametrization is generically one-to-one onto its image. The
normalization is therefore `P^1`, so the geometric genus is zero.

For a degree-16 plane curve, the arithmetic genus is

```text
g_a = (16-1)(16-2)/2 = 105.
```

Thus all singularities together must account for defect `105`.

### 15.2 Finite double-pair budget

In the affine chart, two non-diagonal parameters `w != z` give the same point
when

```text
N_x(w) z^8 - N_x(z) w^8 = 0,
N_v(w) z^8 - N_v(z) w^8 = 0.
```

After removing the diagonal factor `w-z`, the resultant in `z` factors as:

```text
w^150 * F_14(w) * F_28(w) * F_108(w)
```

where the nonzero finite factors have degrees

```text
14, 28, 108.
```

The `w^150` factor is the projective-infinity contribution introduced by
clearing denominators. The finite nonzero degree sum is

```text
14 + 28 + 108 = 150.
```

These are ordered preimages. Dividing by 2 gives

```text
75
```

finite unordered double-pair units.

The factor meanings align with the earlier branch grammar:

| Factor degree | Geometric role |
|---:|---|
| `14` | axis branch-collapse pairs from `P(u)=0` |
| `28` | trigonal relay off-axis pairs |
| `108` | residual complex finite double-pair units |

The nine visible real nodes are contained in the first two factors. Most
finite double-pair units are complex.

### 15.3 The two infinity cusps

There are two points at infinity:

```text
w = 0        -> [3: 6:0],
w = infinity -> [3:-6:0].
```

Near `w=0`, use the affine chart `X != 0`:

```text
eta = V/X - 2,
xi  = Z/X.
```

The local expansions begin:

```text
eta ~ w^4,
xi  ~ w^8.
```

After removing the quadratic tangent term,

```text
rho = xi - (3/16) eta^2,
```

one gets

```text
rho ~ w^11.
```

The same orders occur at `w=infinity`, using `q=1/w` and `eta = V/X + 2`.

Thus each infinity branch has first Puiseux data bounded below by

```text
(4, 11),
```

which gives local defect at least

```text
delta_infinity >= (4-1)(11-1)/2 = 15.
```

There are two such points, so infinity contributes at least

```text
2*15 = 30.
```

### 15.4 Budget closure

The verified lower-bound budget is:

```text
finite double-pair units + infinity cusp lower bounds
= 75 + 15 + 15
= 105.
```

This equals the full arithmetic-genus defect. Therefore there is no room for
additional hidden defect:

- the finite double-pair units account for `75`;
- each infinity cusp contributes exactly `15`;
- the total singularity defect is exactly `105`.

This gives the projective skeleton:

```text
degree-16 rational image
  -> genus-zero normalization
  -> 75 finite double-pair units
  -> two infinity cusps of defect 15 each
  -> total defect 105.
```

### 15.5 FTD-facing interpretation

The projective budget reinforces the readout-machine picture. The visible
plane curve looks like a finite drawing with nine real crossings, but the
projective algebraic object carries a much larger hidden singularity ledger:

```text
real-visible nodes: 9
finite projective double-pair units: 75
total projective defect: 105
```

So the external readout is strongly lossy. It exposes only a small real slice
of the singularity structure forced by the hidden phase map.

This is useful FTD intuition, but not a physics claim:

```text
simple hidden phase -> lossy visible projection -> large hidden algebraic budget.
```

*End of document.*
