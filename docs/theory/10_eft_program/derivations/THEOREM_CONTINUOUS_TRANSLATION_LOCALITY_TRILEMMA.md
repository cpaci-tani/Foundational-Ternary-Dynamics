# THEOREM — Continuous translation versus strict lattice locality

**Identifier:** `FTD-0554`  
**Status:** `[THEOREM — HOMOGENEOUS UNITARY FINITE-RANGE TRANSLATION NO-GO] +
[CONSTRUCTIVE — NONLOCAL BAND-LIMITED ESCAPE]`  
**Scope:** exact microscopic continuous translation group on a discrete
lattice. No continuum spacetime, Lorentz symmetry, field equation, or specific
polarity kernel is assumed by the no-go.

## 1. Finite-range homogeneous operators

In one dimension, a homogeneous finite-range linear operator is convolution
by finitely many coefficients `c_n`. Its Fourier symbol is a Laurent
polynomial

```text
p(z)=sum_{n=a}^{b} c_n z^n,       z=exp(ik),       (1)
```

with nonzero extreme coefficients `c_a,c_b`.

### Laurent-unitary lemma

If the convolution is unitary, `|p(exp(ik))|=1` for all `k`. The Fourier
coefficients of `|p|^2` at every nonzero lag must vanish. At the largest lag
`b-a`, only one product contributes:

```text
[z^(b-a)] |p(z)|^2 = c_b conjugate(c_a).          (2)
```

This cannot vanish when `a<b`. Therefore `a=b` and

```text
p(z)=exp(i theta) z^m.                            (3)
```

Every finite-range homogeneous unitary is only an integer shift times a
global phase. This proof allows complex and signed kernels; positivity and
regularity are irrelevant.

## 2. Continuous-group obstruction

Suppose `U(t)` is a continuous one-parameter group of finite-range homogeneous
unitaries, with `U(0)=I` and `U(1)=T`, the one-site shift. By (3),

```text
U(t,k)=exp(i theta(t)) exp(i m(t) k),   m(t) in Z. (4)
```

Continuity makes the integer-valued `m(t)` locally constant. The real line is
connected and `m(0)=0`, so `m(t)=0` for all `t`. But `U(1)=T` requires
`m(1)=+1` up to convention. Contradiction.

Therefore the following four requirements cannot coexist:

1. exact continuous translation composition;
2. homogeneous unitary/energy-preserving translation;
3. exact agreement with the integer lattice shift;
4. strict finite-range support at every fractional translation.

An axis restriction proves the three-dimensional tensor case immediately.
The result is distinct from FTD-0540: that theorem prices smoothness,
positivity, cardinality, and first moments; this theorem remains even after
all of those representation choices are relaxed.

## 3. Exact nonlocal escape

On an odd periodic lattice `L=2M+1`, choose the canonical Fourier generator.
The fractional translate of a cardinal site is

```text
S_f(n)=1/L sum_{m=-M}^{M}
       exp[2 pi i m(n-f)/L].                       (5)
```

Its Fourier coefficients have unit modulus and phase `exp(-ikf)`. Hence

```text
U(a)U(b)=U(a+b),
||S_f||_2=1,
S_r(n)=delta_(n,r) for integer r.                 (6)
```

For noninteger `f`, (5) is the periodic sinc/Dirichlet kernel. It is real but
signed and nonzero at every site. In three dimensions, generic fractional
translation of a cardinal voxel has tensor support `L^3`.

## 4. Why the escape removes Peierls energy

For any integer-offset composite with structure factor `A(k)`, exact
band-limited translation gives

```text
rho_f_hat(k)=exp(-ik dot f) A(k).                 (7)
```

For every positive translation-invariant quadratic field kernel `G(k)`,

```text
U(f)=1/(2L^D) sum_k G(k)|rho_f_hat(k)|^2
    =1/(2L^D) sum_k G(k)|A(k)|^2.                (8)
```

Thus the Peierls potential vanishes exactly. FTD-0553's compact quadratic coat
fails precisely because its Fourier magnitude depends on the subcell phase;
the phase-only property (7) is what exact translations require.

## 5. Why the escape violates the face-current mainline

For a nonzero fractional move, `rho_1-rho_0` from (5) has global support.
Exact continuity requires

```text
rho_1-rho_0+D K=0.                                (9)
```

The divergence of a compactly supported current is compactly supported.
Therefore a globally supported density change cannot be produced by a local
current. The locked Fourier construction confirms that `K` itself has full
lattice support.

The pinning-free periodic-sinc coat is consequently not an admissible repair
of the frozen FTD face-flux model. It would make an arbitrarily small particle
move alter coupling weights and source current everywhere at once.

## 6. Physical consequence

Exact microscopic continuous translation is too strong for a strictly local
discrete ontology. The honest alternatives are:

1. accept a microscopic Peierls potential and require it to become negligible
   for extended low-energy excitations;
2. use nonlocal band-limited coupling and abandon local causality;
3. abandon point-carrier deposition and derive matter as a native extended
   lattice excitation whose motion is a sequence of local field updates; or
4. retain only integer microscopic motion and seek continuous translation and
   common-cone behavior in the infrared.

Options 1, 3, and 4 can coexist. They imply that the correct mobile object is
not a continuously sliding ternary point with a compact interpolation coat.
It must be an extended configuration or hopping quasiparticle whose Peierls
barrier is measured and shown to scale away. This theorem selects no such
configuration and licenses no production change.
