# THEOREM — Quadratic-coat fixed-step matter work is not an identity

**Identifier:** `FTD-0545`  
**Status:** `[THEOREM — UNIFORM-WITNESS NONIDENTITY] + [CLOSED NEGATIVE —
UNIVERSAL FIXED-STEP MATTER-WORK IDENTITY]`  
**Inputs:** the selected FTD-0542 coat interaction, its analytic endpoint
variation, the production dispersion, and the FTD-0544 field-work ledger.

## 1. Endpoint Legendre map

Let one slab have duration `h`, displacement `d=x1-x0`, and

```text
S_m=-(M h/c)sqrt(1-|d|^2/h^2),
H(p)=sqrt(M^2+c^2|p|^2).                           (1)
```

The free endpoint momentum is

```text
p=M d/[c h sqrt(1-|d|^2/h^2)].                    (2)
```

For the coat interaction

```text
S_int=qg integral_0^1 [A(t,x(t)).d-h Phi(x(t))]dt, (3)
```

direct differentiation of the straight segment gives

```text
D_1 S_int=qg integral[-A+(1-t)(grad A)^T d
                      -h(1-t)grad Phi]dt,
D_2 S_int=qg integral[ A+t(grad A)^T d
                      -h t grad Phi]dt.            (4)
```

The canonical and gauge-covariant kinetic endpoint momenta are therefore

```text
P0=p-D_1 S_int,       pi0=P0-qg A0(x0),
P1=p+D_2 S_int,       pi1=P1-qg A1(x1).            (5)
```

These formulas use the same `B1/B2` compatible interpolation as the deposited
FTD-0542 current. Four-point Gauss-Legendre integration, split at every
half-integer knot, is exact on each polynomial chart.

## 2. Exact counterexample

Choose a spatially uniform harmonic connection with

```text
Phi=0,   A1-A0=-h E,   g=beta/h.                  (6)
```

Partition of unity makes the interpolated connection constant and annihilates
all spatial gradients. Equations (4)--(5) reduce exactly to

```text
pi0=p-a,   pi1=p+a,   a=beta q E/2.               (7)
```

The FTD-0544 matter-side requirement is

```text
H(pi1)-H(pi0)=beta<E,K>.
```

First-moment reproduction and (2) give

```text
beta<E,K>=2 a.c^2 p/H(p).                         (8)
```

For collinear `p` and `a`, the exact defect is consequently

```text
D=H(p+a)-H(p-a)-2a c^2 p/H(p).                   (9)
```

Expanding only to expose the obstruction, not to define it,

```text
D=-(c^4 M^2 p/H(p)^5)a^3+O(a^5).                 (10)
```

Thus `D` is generically nonzero for a massive particle. The central finite
difference of the nonlinear dispersion derivative is not the exact derivative
itself. Exact continuity, gauge covariance, and exact field Poynting exchange
do not remove this cubic mismatch.

## 3. Boundary

Equation (9) is an analytic counterexample to a *universal* fixed-step
matter-work identity for the selected coat action and production dispersion.
It does not close every self-consistent reciprocal dynamics: the uniform mode
is an external harmonic sector and is not the periodic Gauss field of one net
charge. A neutral, self-consistent coupled-pair transaction remains the next
registered gate; FTD-0546 later executes it and also closes negative.

No lapse, energy projection, discrete-gradient force, extra variable, toggle,
scenario, or production dynamics follows from this theorem.
