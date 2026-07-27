# AUDIT — Two-domain Lorentz construction: BCC time, SC+FCC space

**Registry:** FTD-0411  
**Status:** `[CONDITIONAL THEOREM — selected two-domain kernel]` + `[SCOPED NO-GO — finite-state positive-norm linear localization over the rational M18 algebra]` + `[SELECTED IMPLEMENTATION PROTOTYPE]` + `[OPEN — nonlinear/constrained realization and common cone]`  
**Verdict:** `TWO-DOMAIN-CONE-EXISTS; FINITE-LINEAR-EXACT-CLOCK-EXCLUDED`  
**Verification:** `scripts/proofs/proof_lorentz_two_domain_bcc_time.py`  
**Date:** 2026-07-22

## 0. Result

The proposal that the BCC layer is temporal while the SC+FCC layers are
physical can be made into an exact pole equation. It is not yet forced by
P1–P5, so the layer-role assignment is a `[SELECTION]`.

Use the production SC+FCC spatial symbol `M18(q)` and the normalized
three-factor BCC temporal symbol

\[
T_B(\theta)=\frac23\left(1-\cos^3\theta\right).
\]

The normalization is unique once the leading temporal kinetic term is fixed
to `theta²`. The pole equation

\[
T_B(\theta)=c^2M_{18}(\mathbf q)
\]

has no quartic preferred-frame term if and only if

\[
\boxed{c^2=\frac17},\qquad \boxed{c=\frac1{\sqrt7}}.
\]

Thus `1/sqrt(7)` is not inserted into this selected architecture; it follows
from the BCC cubic time kernel plus the existing SC+FCC spatial kernel. The
integer `7` comes from the `-7 theta^4/12` coefficient of normalized
`1-cos^3(theta)`. Its numerical equality with the framework integer `b3=7`
does not establish a common origin.

The literal scalar BCC clock is not viable as a complete local update. Its
cubic equation in `cos(theta)` has one real branch and two complex branches,
which become four non-unit-circle transfer roots. More strongly, a finite
number of positive-norm linear auxiliaries depending rationally on `M18` cannot
remove them: the cubic is irreducible over `Q(M18)`, while every
eigenvalue of the Hermitian part of a unitary transfer must be real. Exact
realization therefore requires a nonlocal cube-root operator, a genuinely
nonlinear/constrained clock, or an interacting emergence mechanism.

A default-off CPU prototype implements a stable two-tick IR localization. It
preserves the derived cone and exact quartic cancellation but differs at sixth
order. It is not an exact BCC temporal dynamics and does not close Lorentz
recovery.

---

## 1. The selected two-domain map

The Moore shell separates exactly into:

| role in this hypothesis | Moore layer | offsets | Fourier character |
|---|---|---:|---|
| physical space | SC + FCC | 6 + 12 | additive face/edge symbol `M18` |
| temporal return | BCC | 8 | multiplicative character `cos qx cos qy cos qz` |

The physical reason for testing this split is structural, not numerical. SC
and FCC offsets translate through one or two physical axes. A BCC corner
changes all three axes simultaneously, and its return character is the only
Moore character that factorizes as a product of three one-dimensional
characters. Assigning those three synchronized factors to a local clock gives

\[
\cos q_x\cos q_y\cos q_z
\longrightarrow \cos\theta\cos\theta\cos\theta=\cos^3\theta.
\]

That arrow is the new assumption. The Moore decomposition proves the product
character; it does not prove that the three factors are temporal phases or
that they must be synchronized.

The production physical-space symbol remains

\[
M_{18}(\mathbf q)
=4-\frac23\sum_i\cos q_i
-\frac23\sum_{i<j}\cos q_i\cos q_j.
\]

Writing `S2=sum(q_i²)`, `Q4=sum(q_i^4)`, and `Q6=sum(q_i^6)`, its infrared expansion is

\[
M_{18}=S_2-\frac{S_2^2}{12}
+\frac{S_2Q_4}{72}-\frac{Q_6}{90}+O(q^8).
\]

This pure-power basis corrects a notation collision found by FTD-0414: the
earlier FTD-0407 document used the label `Q6` for a different cubic
discriminator, while this document described it as `sum(q_i^6)`.

---

## 2. Canonical normalization of BCC time

The raw BCC return defect is `1-cos³(theta)`. Near the origin,

\[
1-\cos^3\theta=\frac32\theta^2+O(\theta^4).
\]

Canonical temporal normalization therefore fixes the prefactor to `2/3`:

\[
T_B(\theta)=\frac23(1-\cos^3\theta)
=\theta^2-\frac7{12}\theta^4
+\frac{61}{360}\theta^6+O(\theta^8).
\]

Equivalently,

\[
T_B(\theta)=\frac12(1-\cos\theta)
+\frac16(1-\cos3\theta).
\]

This identity shows exactly what the clock contains: a fundamental temporal
harmonic plus a third harmonic. No AGM value has been inserted.

---

## 3. Derivation of the cone

Let `u=theta²` and `x=c²M18`. Reversion of the BCC time series gives

\[
u=x+\frac7{12}x^2+\frac{23}{45}x^3+O(x^4).
\]

Substitution of the spatial symbol yields

\[
\theta^2
=c^2S_2
+\frac{c^2(7c^2-1)}{12}S_2^2+O(q^6).
\]

The complete rotationally invariant dimension-six pole correction vanishes
precisely when

\[
c^2(7c^2-1)=0.
\]

Discarding the zero-speed branch leaves

\[
\boxed{c^2=1/7}.
\]

At this value the pole through sixth order is

\[
\boxed{
\theta^2=\frac{S_2}{7}
-\frac{61}{123480}S_2^3
+\frac{S_2Q_4}{504}-\frac{Q_6}{630}+O(q^8)}.
\]

The quartic term is absent. A cubic rotational-breaking term survives at
sixth order, so this is an improved infrared cone, not exact Lorentz symmetry.

The speed is a ratio of physical-lattice spacing to temporal-lattice spacing.
It is not a claim that an SI photon traverses `1/sqrt(7)` metres per second.
An observed speed follows only after both lattice units are calibrated and
all observable sectors are shown to share this same pole.

---

## 4. The physical branch is bounded on the complete spatial band

The exact production band is `0<=M18<=16/3`. At `c²=1/7`,

\[
\cos^3\theta=1-\frac{3M_{18}}{14}
\in\left[-\frac17,1\right].
\]

The real cube root therefore gives one real principal phase over the entire
band. There is no high-momentum loss of the principal oscillatory branch.

This fact is insufficient for a dynamical implementation because a scalar
polynomial clock must carry every root, not only the chosen one.

---

## 5. Literal scalar realization has four unwanted modes

Set

\[
y=\cos\theta,\qquad R(M)=1-\frac{3M}{14}.
\]

The temporal pole is `y³-R=0`. For generic `R!=0`, its discriminant is

\[
\operatorname{Disc}(y^3-R)=-27R^2<0.
\]

Hence the cubic has one real root and two non-real conjugate roots. A scalar
finite-difference realization converts each `y` into transfer roots through

\[
z+z^{-1}=2y.
\]

If `|z|=1`, then `(z+z^{-1})/2` is real. The two complex `y` branches therefore
produce four transfer roots off the unit circle. They are exponentially
growing/decaying ghost modes. Selecting only the real cube root is a nonlocal
spectral projection, not a local scalar update.

### 5.1 Finite-state rational-M18 unitary auxiliaries do not cure it

The scalar failure is not cured merely by adding a finite number of ordinary
positive-norm linear clock components.

Let `K=Q(M18)` be the rational functional calculus of the production
SC+FCC symbol. Any finite-state linear localization that preserves the premise
that physical space enters only through `M18`, using polynomial or rational
couplings, has matrix entries in `K`. The desired phase would make

\[
y=\cos\theta
\]

an algebraic eigenvalue satisfying

\[
p(y)=y^3-R(M)=0,
\qquad R(M)=1-\frac{3M}{14}.
\]

`R(M)` has a simple zero at `M=14/3`, so it is not a cube in `K`. The cubic
`p` is therefore irreducible over `K`.

Assume a finite-dimensional local unitary transfer `U(M)` has the desired
eigenphase. Then

\[
H(M)=\frac12\left(U+U^\dagger\right)
\]

is a finite Hermitian matrix over the same operator field, and its
corresponding eigenvalue is `y=cos(theta)`. The minimal polynomial `p(y)` must
divide the characteristic polynomial of `H`. It brings all three algebraic
conjugates with it. Two are non-real for generic `M`, contradicting the real
spectrum of a Hermitian matrix.

Therefore:

> **Rational-M18 linear no-go.** No finite-dimensional positive-norm unitary
> linear transfer with entries rational in `M18` realizes the exact BCC
> temporal branch.

Writing the physical branch directly as

\[
y=\left(1-\frac{3M}{14}\right)^{1/3}
\]

does not evade the theorem. Its binomial series contains every power `M^n`;
since `M^n` spans up to `n` Moore shells, the exact cube-root operator has
unbounded spatial support. It is nonlocal.

The theorem does not exclude a nonlinear constrained update, an interacting
collective pole after coarse-graining, an approximation to finite infrared
order, or a larger direction-dependent local operator algebra. The last escape
would abandon the strict “physical space enters only through `M18`” premise and
would require a new cubic-symmetry/anisotropy audit. The theorem excludes the
most direct exact finite-state paraunitary escape within the two-domain map.

---

## 6. Stable P4-local IR localization

For two alternating nearest-Moore kicks `k0,k1`, the determinant-one Floquet
pole is

\[
\sin^2\theta
=\frac{k_0+k_1}{2}M
-\frac{k_0k_1}{4}M^2.
\]

Matching the BCC-time speed and quartic cancellation fixes

\[
k_0+k_1=\frac27,
\qquad k_0k_1=-\frac1{49},
\]

and therefore

\[
\boxed{k_0=\frac{1+\sqrt2}{7}},
\qquad
\boxed{k_1=\frac{1-\sqrt2}{7}}.
\]

The exact local surrogate pole is

\[
\boxed{\sin^2\theta=\frac{M_{18}}7+\frac{M_{18}^2}{196}}.
\]

It is monotone on the physical band and

\[
X(16/3)=\frac{400}{441}<1,
\]

so all free Floquet multipliers lie on the unit circle. Each microscopic tick
reads only the existing nearest Moore shell; the effective `M18²` term appears
after two legal ticks.

Its infrared pole is

\[
\theta^2=\frac{S_2}{7}
-\frac{121}{123480}S_2^3
+\frac{S_2Q_4}{504}-\frac{Q_6}{630}+O(q^8).
\]

The BCC clock and surrogate agree at leading and quartic orders, and on both
directional sixth-order coefficients, but their isotropic coefficients differ by

\[
-\frac{121}{123480}+\frac{61}{123480}=-\frac1{2058}.
\]

The engine toggle `lorentz_bcc_time_floquet` is therefore labeled
`[SELECTED IR PROTOTYPE]`, is CPU-only and unit-step, and defaults off.

---

## 7. Why a three-tick scalar match does not repair the defect

Matching the BCC temporal germ through `M³` with a period-three scalar cell
gives the exact real kicks

\[
\left\{\frac17,\frac5{14},-\frac1{14}\right\}
\]

and half-trace

\[
C_3(M)=1-\frac9{14}M+\frac3{196}M^2+\frac5{2744}M^3.
\]

At the production endpoint,

\[
C_3(16/3)=-\frac{15899}{9261}<-1.
\]

Thus the natural three-tick scalar localization reproduces the BCC germ more
deeply but is unstable on the complete SC+FCC band. The stable two-tick map is
the stronger current implementation even though it leaves an order-six
mismatch.

---

## 8. What is and is not established

Established conditionally on the selected two-domain kernel:

1. Canonical BCC temporal normalization is unique.
2. Quartic pole cancellation fixes `c²=1/7` without a numerical search.
3. The principal BCC phase is real on the complete production band.
4. No finite-state positive-norm linear/unitary auxiliary system rational in
   `M18` can isolate the physical cube-root branch.
5. A nearest-Moore period-two surrogate is exactly full-band stable and
   reproduces the BCC cone through quartic order.

Not established:

1. P1–P5 do not force BCC to be temporal or SC+FCC to be exclusively spatial.
2. No nonlinear, constrained, interacting, or enlarged-operator ghost-free BCC
   clock has been constructed; the rational-`M18` finite-state linear class is
   excluded.
3. Matter, gauge, gravity, and composite excitations have not been shown to
   share `c²=1/7`.
4. Radiative suppression of dimension-three/four preferred-frame operators
   remains uncomputed.
5. The surviving sixth-order cubic term is not Lorentz invariant.
6. No conversion from lattice units to the measured SI value of light speed
   follows from this construction alone.

The next hard target is no longer a generic finite paraunitary matrix; that
class is closed. It is an explicitly constrained nonlinear clock or an
interacting coarse-grained pole whose physical correlator reproduces the BCC
branch without inserting the nonlocal cube root. Failure there would demote
BCC time from a dynamical candidate to an infrared mnemonic.
