# EXPLORATION — Gaussian-Prime Causal-Cone Reference Geometry

**Status:** `[THEOREM — ARITHMETIC GEOMETRY OF THE TWO LIFTS]` +
`[SELECTION — NORM MODULUS AS AN OPERATIONAL-TIME COORDINATE]` +
`[CONJECTURE — LOCAL PRIME PHASES AS CLOCKS]` +
`[OPEN — NATIVE FTD CARRIER, BORN PUSHFORWARD, AND LORENTZ BRIDGE]`

**Scope:** This note formalizes a reference geometry suggested by the visible
dispersal of Gaussian primes. It introduces no LEDGER row, changes no
constitutional commitment, and does not promote a Gaussian-prime picture to
native FTD dynamics. The exact results concern Gaussian integers and coordinate
lifts only. The time, measurement, and causal readings are separately tagged.

**Artifact:**
`scripts/visualization/viz_gaussian_prime_cone.py`; focused tests:
`scripts/tests/test_gaussian_prime_cone.py`.

---

## 1. The arithmetic point set

Let (z=a+ib\in\mathbb Z[i]) and

\[
N(z)=z\bar z=a^2+b^2.
\]

The Gaussian-prime classification is exact:

1. if (ab\ne0), then (z) is Gaussian prime exactly when (N(z)) is a
   rational prime;
2. if (ab=0), then (z) is Gaussian prime exactly when its nonzero
   coordinate is a rational prime (p\equiv3\pmod4);
3. (2) ramifies, with prime element (1+i) and its associates.

For a split prime (p=a^2+b^2), its Gaussian-prime elements have norm (p).
For an inert axis element (p+0i), the Gaussian norm is (p^2). Rational-prime
order and Gaussian-norm order must therefore not be conflated.

### 1.1 The diagonal fixed set is prime-empty after ramification

Reflection across the first-quadrant diagonal is

\[
R(z)=i\bar z,
\qquad R^2=1.
\]

Its fixed Gaussian integers are (a(1+i)). If |(a)| (>1), then

\[
a(1+i)=a\cdot(1+i)
\]

is composite. Hence the only Gaussian-prime elements on either (45^\circ)
diagonal are the associates of (1+i). Every odd split-prime orbit occurs in
a reflected pair strictly off the diagonal. This is an exact discrete
separatrix statement, not a dynamical avoided-crossing result.

### 1.2 What the exact periods actually are

There are three elementary symmetries which can look like a rotating clock
when the points are plotted in a chosen order:

1. multiplication by the Gaussian unit (i) is a quarter-turn,
   (Q(a,b)=(-b,a)), with exact order four: (Q^4=1);
2. diagonal reflection is (R(a,b)=(b,a)), with exact order two:
   (R^2=1);
3. for a generic split prime, these generate an eight-point (D_4) orbit.
   The ramified diagonal prime has only four distinct associates.

Thus **four** is the exact rotational period and **eight** is the generic
orbit size. If a renderer alternates ((a,b)) and ((b,a)), the visible
back-and-forth across (45^\circ) has period two by construction. It is a
reflection pairing, not yet a period in prime order or physical time.

The residue split is also modulo four: odd rational primes lie in
(1\pmod4) or (3\pmod4). But the increasing sequence of primes does not
alternate deterministically between those classes. Dirichlet balance is an
asymptotic density statement, not a two-tick clock.

---

## 2. Two lifts that must remain distinct

### 2.1 Integer-height norm lift: a paraboloid

The map

\[
P(a,b)=(a,b,h=N(z))
\]

satisfies

\[
h=a^2+b^2.
\]

This is a paraboloid. Its height is integer-valued and its radial scaling is
(r\sim\sqrt h). If height is interpreted as evolution order, it has
dynamical exponent two and is diffusion-like rather than ballistic.

### 2.2 Modulus lift: a Euclidean null cone

The map

\[
C(a,b)=(a,b,\tau=\sqrt{N(z)})
\]

satisfies

\[
\boxed{\tau^2-a^2-b^2=0}.
\]

This is the positive sheet of a (2+1) null cone. Split elements have
(\tau=\sqrt p); inert axis elements have (\tau=p). The cone is consistent
for both classes because it uses the Gaussian norm in both cases.

The two lifts are not interchangeable:

| height | exact surface | order property | possible reading |
|---|---|---|---|
| (h=N(z)) | (h=a^2+b^2) | integer | norm/scale order |
| (\tau=\sqrt{N(z)}) | (\tau^2=a^2+b^2) | generally noninteger | operational modulus |

The second row cannot be identified with FTD's integer tick (n) without an
additional map. In particular, (\sqrt p\) is irrational for every rational
prime (p). The cone reading is therefore `[SELECTION]`, not an ontic-time
derivation.

---

## 3. Three Gaussian sections of a 3+1 cone

Each planar point admits three coordinate-plane lifts:

\[
\begin{aligned}
C_{xy}(a,b)&=(a,b,0;\tau),\\
C_{yz}(a,b)&=(0,a,b;\tau),\\
C_{zx}(a,b)&=(b,0,a;\tau),
\end{aligned}
\qquad \tau=\sqrt{a^2+b^2}.
\]

Every lift satisfies

\[
\boxed{\tau^2-x^2-y^2-z^2=0}.
\]

The construction gives three great-circle sections of each normalized
spherical cross-section. It does **not** fill the sphere and therefore does
not constitute a three-dimensional prime-distribution theorem. Filling the
full cone would require either:

- a native dynamics that mixes the three planes while preserving the norm; or
- a genuinely three-dimensional norm-prime object with its own arithmetic
  classification.

Neither has been supplied here.

---

## 4. Local phase and global radial order

For a split prime (p=a^2+b^2), define

\[
u_p=\frac{a+ib}{\sqrt p}=e^{i\theta_p},
\qquad
\bar u_p=e^{-i\theta_p}.
\]

The modulus fixes the cone height and the argument fixes a ray on the cone.
This provides a precise local/global decomposition:

\[
\text{global radial coordinate }\tau_p=\sqrt p,
\qquad
\text{local conjugate phases }\pm\theta_p.
\]

Hecke equidistribution makes the normalized angular distribution uniform in
the large-norm sense; modern context and the boundary between proved
equidistribution and conjectural fine-scale variance are given by
[Rudnick and Waxman](https://arxiv.org/abs/1705.07498). Fourier-mode
prime-counting functions are governed by angular Hecke (L)-functions. In the
associated explicit-formula reading, a zero (1/2+i\gamma) contributes
schematically to a Gaussian-norm cutoff (X) through a mode of the form

\[
\cos(\gamma\log X+\phi).
\]

Since (X=\tau^2), the same mode on cone height is

\[
\cos(2\gamma\log\tau+\phi),
\]

with logarithmic-height period (\pi/\gamma) and multiplicative radial factor
(e^{\pi/\gamma}). This individual-zero statement is a conditional spectral
component, not evidence that the unsmoothed prime plot has a clean isolated
oscillation. It is an arithmetic scale beat, not a period in global FTD
ticks.

Accordingly, the period ledger is:

| structure | exact or conditional period | interpretation |
|---|---|---|
| unit rotation (z\mapsto iz) | 4 quarter-turns | exact Gaussian symmetry |
| diagonal reflection (z\mapsto i\bar z) | 2 reflections | exact pairing |
| generic (D_4) orbit | 8 distinct points | orbit size, not elapsed time |
| residue character modulo 4 | conductor 4 | arithmetic class, not prime-order alternation |
| Hecke spectral mode (\gamma) | (\Delta\log\tau=\pi/\gamma) | conditional log-scale beat |
| critical quartic oscillator | (TA=\sqrt\pi G^*\sqrt{m/(2\lambda)}) | selected maintained-clock law |

Only the last row is presently a candidate physical clock. The preceding
rows organize the arithmetic point set; they do not advance the global tick.

### 4.1 Clock interpretation and its price

Treating (u_p) as a local clock phase is `[CONJECTURE]`. It requires a
physical carrier whose state realizes the prime-indexed phase and whose phase
can be read without importing the desired measurement statistics. The global
lemniscatic period coefficient

\[
\sqrt\pi G^*=2\varpi
\]

and the finite-place prime phases belong to the same CM arithmetic setting,
but this does not make one a derived dynamical synchronization of the other.
The archimedean period/local Euler-data relation is structural; the physical
time identification remains open.

---

## 5. Quadratic channel weights and interference

The null condition normalizes the spatial direction:

\[
\left(\frac a\tau\right)^2+
\left(\frac b\tau\right)^2=1.
\]

Writing (a/\tau=\cos\theta) and (b/\tau=\sin\theta) gives

\[
w_x=\cos^2\theta,
\qquad
w_y=\sin^2\theta,
\qquad
w_x+w_y=1.
\]

After an orthogonal contextual rotation by (\phi), the corresponding
component weights are

\[
w_1=\cos^2(\theta-\phi),
\qquad
w_2=\sin^2(\theta-\phi).
\]

The Gaussian norm also has the exact interference identity

\[
N(z_1+z_2)=N(z_1)+N(z_2)+2\operatorname{Re}(z_1\bar z_2).
\]

These are theorem-grade quadratic identities. They do **not** establish that
physical terminal-record frequencies equal the component weights. The latter
requires a substrate equilibrium measure and a context-complete basin
pushforward that does not read or encode the target weights. That debt remains
under `PREREG_CONTEXTUAL_BORN_RECOVERY_v1.md`.

---

## 6. Native Moore causality is a separate cone

For the full 26-neighbour Moore graph, the exact minimum number of native
steps from the origin to ((x,y,z)) is

\[
d_{26}(x,y,z)=\max(|x|,|y|,|z|).
\]

Its causal frontier is cubic, not Euclidean. For example,

\[
d_{26}(2,3,0)=3,
\qquad
\sqrt{2^2+3^2}=\sqrt{13}.
\]

Therefore the Gaussian Euclidean cone cannot be presented as the raw native
Moore causal cone. It may serve only as:

1. an abstract potentiality/reference geometry; or
2. an operational cone after a demonstrated isotropic recovery map.

This is a live branch conflict, not a contradiction to be hidden. The
operational recovery must meet the cone/isotropy requirements already stated
in the temporal-interior programme.

---

## 7. Visualization correction

The legacy Blender paraboloid placed split elements at height (N=p) but
inert axis elements at height (p), even though their Gaussian norm is
(p^2). That mixed the paraboloid and cone lifts in one image. The artifact is
corrected so all elements in `blender_paraboloid_primes.py` use height (N).

The new reference visualizer renders separately:

1. the planar Gaussian-prime set and diagonal fixed lines;
2. the consistent (2+1) cone with (\tau=\sqrt N);
3. the normalized three-plane sections of the (3+1) cone.

The third panel is labeled explicitly as three great circles rather than a
full spherical dispersal.

---

## 8. Acceptance and stop conditions

The arithmetic geometry passes if exact classification, norm placement,
diagonal exclusion, (2+1) null residuals, (3+1) null residuals, and
quadratic normalization all pass symbolically or to floating-point roundoff.

No physical claim may be promoted unless all of the following are supplied:

1. a native or explicitly adopted map from integer tick (n) to the selected
   operational modulus (\tau);
2. a restriction-consistent preparation measure;
3. a context-blind carrier/gate mechanism;
4. a non-target-coded terminal-basin pushforward;
5. operational hiding of the preferred tick and recovery of the declared
   Euclidean cone within preregistered tolerances;
6. an account of how three plane sections become full three-dimensional
   propagation, or a declaration that they do not.

Failure of any item preserves the cone as exact arithmetic reference geometry
but closes the corresponding physical interpretation.
