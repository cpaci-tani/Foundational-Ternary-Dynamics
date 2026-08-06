# FTD-0701 — Connected-bipole Cherenkov form factor

**Status:** `[THEOREM — IDEALIZED RIGID STRUCTURE FACTOR]` +
`[POST-HOC NUMERICAL FACT — REFINED STATIC GEOMETRY]`  
**Production status:** unchanged

## 1. Idealized connected object

The width-two selected connected object consists of four equal transverse
copies of the axial polarity sequence

```text
x = -3/2, -1/2, +1/2, +3/2
q =   +1,   +1,   -1,   -1.
```

With transverse coordinates `y,z in {-1/2,+1/2}`, its point structure factor
is

\[
F(\mathbf k)=\sum_j q_j e^{-i\mathbf k\cdot\mathbf x_j}
=16i\,\sin k_x\cos\frac{k_x}{2}
       \cos\frac{k_y}{2}\cos\frac{k_z}{2}.
\]

For a rigid axial translation, the leading convective current inherits this
constituent factor, multiplied by the registered coupling-coat and path
factors.

## 2. Exact screening at the zone edge

The FTD-0700 exact transverse field witness has

\[
v=\frac12,
\qquad
\mathbf k=\left(\pi,\frac\pi2,0\right).
\]

The ideal connected-bipole factor vanishes exactly because `sin(k_x)=0` and
`cos(k_x/2)=0`. In fact, for `k_x=pi-epsilon`,

\[
F(\mathbf k)=O(\epsilon^2),
\qquad |F(\mathbf k)|^2=O(\epsilon^4).
\]

The selected geometry therefore screens the lowest axial-threshold edge more
strongly than neutrality alone. The exact FTD-0700 field channel is not an
actual radiation witness for this idealized rigid object.

## 3. The zero does not screen the full resonant curve

At `v=1/2`, the oblique phase-matched curve satisfies

\[
\sin^2\frac{k_y}{2}
=3\sin^2\frac{k_x}{4}-\sin^2\frac{k_x}{2},
\qquad
\frac{2\pi}{3}<k_x\le\pi.
\]

The structure factor vanishes at isolated symmetry points, not on this entire
curve. For the fixed off-edge point `k_x=0.9 pi`, the equation gives

```text
k_y/pi = 0.3619038206454286
Omega(k) - (1/2) k_x = 0
```

and the refined FTD-0638 point geometry has

```text
|F|/16       = 0.04100135985779407
|F|^2/16^2   = 0.001681111510188327.
```

Thus the edge zero softens onset near `v_edge`; it does not provide a general
radiation-free moving state at `v=1/2`.

## 4. Refined-state edge residual

The FTD-0638 refined static coordinates are close to, but not exactly, the
ideal half-cell geometry. Evaluated post hoc at the exact FTD-0700 witness,
their point structure factor is

```text
|F|/16       = 5.117229666019852e-5
|F|^2/16^2   = 2.618603945479364e-9.
```

This is a strong finite-geometry suppression, not an exact zero. It is also
not yet the complete deposited-current form factor: the quadratic transverse
coat, finite straight segment, nonlinear recoil, and co-moving field dressing
must be included by the held-out moving-state campaign.

## 5. Matter-dynamics consequence

The combined FTD-0700/0701 picture is not “fast matter must decay.” It is:

1. dispersive field channels become kinematically available above a
   directional threshold;
2. the composite's internal polarity geometry filters those channels;
3. the present bipole strongly suppresses threshold-edge radiation but leaves
   off-edge phase-matched modes available;
4. only the complete reciprocal moving solution can determine whether
   nonlinear dressing cancels the remaining outgoing field.

The next measurement must therefore record the *deposited current form factor*
and outgoing field spectrum together. A field-only resonance scan or a point
structure factor alone cannot qualify radiation, a wake, or lifetime.
