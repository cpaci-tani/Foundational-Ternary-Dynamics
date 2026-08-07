# FTD-0606 — Global orientation × strain matter core v1

**Status:** `[THEOREM — EXACT STRAIN/ORIENTATION FACTORIZATION]` +
`[MEASURED — CONTINUOUS STATIC CORE]` +
`[UNRESOLVED — TERNARY-SITE DYNAMICAL PROJECTION]`
**Protocol:**
[`PREREG_GLOBAL_ORIENTATION_STRAIN_CORE_v1.md`](../../preregistrations/constituent_complete_matter/PREREG_GLOBAL_ORIENTATION_STRAIN_CORE_v1.md),
prefix SHA-256 `EC0CECED1CCF40187BCE0C4B38DA34039B5CAD94069AFD05F16420D25D99494A`
**Production status:** unchanged

## 1. Exact factorization

After removing the trimer centroid, three points in three dimensions have six
coordinates. Away from degenerate triangles these split into

\[
  (R,H)\in SO(3)\times\operatorname{Sym}(2),
\]

where `R` is global orientation and the symmetric `2x2` matrix `H` carries
the three relational strain coordinates. With `B` an orthonormal basis of the
reference trimer plane and `z_a=B^T r_a^(0)`, the registered chart is

\[
  r_a(R,H)=R B(I+H)z_a.
\]

This is a reparameterization of the same constituent positions, not a new
state variable. It preserves `sum_a r_a=0`. Since `R^T R=I`, every pair
distance, and hence the selected distance-binding potential, is exactly
independent of `R`.

## 2. Exact binding curvature

For the two charge-conjugate equilateral trimers and

\[
 H=\begin{pmatrix}h_0&h_1\\h_1&h_2\end{pmatrix},
\]

direct differentiation of the unchanged quartic distance binding at `H=0`
gives

\[
 \nabla_h^2 V_{\rm bind}(0)=
 \begin{pmatrix}
  18&0&6\\
  0&24&0\\
  6&0&18
 \end{pmatrix}.
\]

Its determinant is `6912` and its spectrum is `{12,24,24}`. The strain sector
is therefore strictly restored. The three zero modes in the FTD-0605
six-coordinate Hessian are exactly rigid rotations, not missing distance
constraints.

The locked protocol says that this Hessian has the “same three positive
eigenvalues” as the nonzero FTD-0605 spectrum “up to coordinate congruence.”
Taken literally, that is false: the earlier Cartesian-coordinate spectrum is
`{24,36-12 sqrt(3),36+12 sqrt(3)}`. Congruence preserves Hessian inertia, not
numerical eigenvalues. The correct invariant statement is that both Hessians
have exactly three positive strain directions and no negative direction. The
record preserves this as a failed literal protocol-wording gate; no threshold
or result is altered.

## 3. Registered measurement

The 24-start global search qualifies at all 32 fractional translation phases:

- 31 phases terminate all 24 starts; one terminates 23;
- every phase has at least four starts in the best-energy cluster;
- the largest optimized strain component is `1.6568766833e-4`;
- the worst tangent gradient is `1.0772356097e-7`;
- the smallest full tangent-Hessian eigenvalue is
  `2.0192487132e-4`;
- all six tangent modes are positive at every reported minimum;
- algebra, Green/direct field, and static energy checks pass at or below
  `9.03e-16`.

Thus the global chart does contain an interior, isolated static minimum in the
continuous constituent-plus-field representation at every sampled phase. The
FTD-0605 boundary escape was a local orientation-chart artifact.

## 4. Ternary-site projection obstruction observed

The same minima are not all states of the selected FTD-0601 transaction.
After mapping continuous constituent positions to `anchor + remainder`, 24 of
32 minima contain at least one pair of constituent records with the same site
anchor; four phases contain two duplicate pairs. FTD-0601 rejects such a state
because one ternary site cannot simultaneously carry two independent
constituent records.

Only phases

```text
5, 6, 14, 15, 16, 17, 25, 26
```

are collision-free. On those eight phases the unchanged common-action and
state-only inverse residuals are at most `4.47e-15` and `1.78e-15`,
respectively. Four phases attract and four repel.

The locked verdict is therefore

```text
GLOBAL_ORIENTATION_STRAIN_NUMERICALLY_UNRESOLVED
```

because transaction and periodicity coverage are incomplete. It is not a
compact-core no-go. The literal eigenvalue-wording gate also fails for the
coordinate reason above. The precise next discriminator is a preregistered global
search whose objective domain enforces site admissibility from the start.

## 5. Ontological consequence

The result separates three notions previously conflated:

1. relational binding stabilizes strain;
2. the lattice field can stabilize global orientation;
3. a stable continuous constituent configuration need not project to a valid
   one-label-per-site ternary configuration.

Consequently, compact matter in this candidate ontology requires both a
stable dressed minimum and a globally admissible site representation. FTD-0606
establishes the first and leaves the second unresolved. It licenses no
physical particle, electron, production toggle, scenario, pole, Lorentz, or
unitarity claim.
