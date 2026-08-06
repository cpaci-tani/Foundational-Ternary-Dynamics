# Theorem — Orientation degree does not determine Gauss charge (FTD-0564)

**Status:** `[THEOREM — SCOPED TO THE SELECTED OCTAHEDRAL OBSERVER AND REGULAR NONCOMPACT FIELD VARIABLES]`  
**Dependencies:** FTD-0392, FTD-0398, FTD-0502, FTD-0563; Berg–Lüscher degree and Poincaré–Hopf are imported mathematics.  
**Production effect:** none.

## 1. Statement

Let the six vertices of the unit octahedron be

\[
V=\{\pm e_x,\pm e_y,\pm e_z\},
\]

with its eight triangular faces oriented outward. For a nonvanishing vertex field `J`, define

\[
Q_{\rm dir}[J]=\frac1{4\pi}\sum_f \Omega_f(\widehat J),
\qquad \widehat J=J/|J|,
\]

using the Berg–Lüscher signed solid angle. Define `Phi[J]` as the exact surface flux of the piecewise-affine interpolation of the vertex values through the geometric octahedron.

Then neither observable determines the other. In particular, for any `A>0` and polarity `p=+/-1`,

\[
H_{A,p}(n)=pA n,
\qquad
T_{A,p}(n)=pA(n+2e_z)
\]

satisfy

\[
Q_{\rm dir}[H_{A,p}]=p,
\qquad
Q_{\rm dir}[T_{A,p}]=0,
\qquad
\Phi[H_{A,p}]=\Phi[T_{A,p}]=4pA.
\]

Consequently:

1. equal nonzero Gauss flux can occur with unequal direction degree;
2. fixed nonzero direction degree can occur with arbitrarily rescaled Gauss flux;
3. a direction-map degree alone cannot quantize electric-charge magnitude.

## 2. Proof

### 2.1 Hedgehog degree

For `H`, normalization removes `A` and leaves `p n`. Each octahedral face maps to one spherical octant. The eight consistently oriented solid angles sum to `4 pi p`, hence

\[
Q_{\rm dir}[H_{A,p}]=p.
\]

This also proves positive amplitude rescaling invariance directly.

### 2.2 Translated-image degree

For `p=+1`, every boundary vector in the affine extension of `T` has strictly positive `z` component after adding `2e_z`, because the unshifted octahedron has `z>=-1`. Its normalized image therefore lies entirely inside the open northern hemisphere. A map whose image lies in a contractible hemisphere is null-homotopic, so its degree is zero. Multiplication by `p=-1` moves the image to the southern hemisphere, which is also contractible. Therefore

\[
Q_{\rm dir}[T_{A,p}]=0.
\]

The field is nowhere zero on the surface; this is not an undefined-degree counterexample.

### 2.3 Exact octahedral flux

For one octahedral face, the area is `sqrt(3)/2`, the outward unit normal is `(s_x,s_y,s_z)/sqrt(3)`, and the mean of its three radial vertex values is `(s_x,s_y,s_z)/3`. Thus the radial field contributes

\[
\frac{\sqrt3}{2}\frac1{\sqrt3}=\frac12
\]

per face. Eight faces give

\[
\Phi[H_{A,p}]=4pA.
\]

The added vector `2pA e_z` is constant. Its closed-surface flux is the constant dotted with the sum of the eight oriented face-area vectors, which vanishes exactly. Hence

\[
\Phi[T_{A,p}]=\Phi[H_{A,p}]=4pA.
\]

This proves both counterexamples and the theorem.

## 3. Periodic consequence

FTD-0502 proves that, on a connected periodic cubic lattice with `N=L^3` sites,

\[
\operatorname{rank}D=N-1,
\qquad
\operatorname{im}D=\{\rho:\sum_x\rho_x=0\}.
\]

Therefore every zero-sum site source can be represented as a face-field divergence. Gauss routability alone cannot distinguish a topological source from an ordinary polarization source or an arbitrarily selected neutral source profile.

If a regular vector field on the periodic three-torus has isolated zeros, the imported Poincaré–Hopf theorem gives

\[
\sum_a \operatorname{index}(J,a)=\chi(T^3)=0.
\]

Local defect/anti-defect pairs remain possible, but a net hedgehog index is excluded without a puncture, boundary, singularity, or additional bundle variable.

## 4. Nonlinear-source classification

For a proposed local effective electric source built from the frozen fields, three cases exhaust the immediate options.

1. If `rho_eff=D P(J,W,...)`, it is a polarization/bound source and is globally neutral on the torus. Its magnitude is set by `P`, not topology.
2. If `rho_eff=F(J,W,...)` is not a divergence, periodic Gauss solvability requires its zero mode to vanish. Enforcing that by subtracting the mean introduces the same global compensator identified by FTD-0563.
3. A quantized source can instead use a singular, punctured, compact, quotient, or bundle-valued variable. That is an additional structure relative to the regular real `J/W` field space and still needs a native action to relate its integer class to a Gauss-flux magnitude.

## 5. Engine scope

The production voxel stores `J`, `W`, and the dual `L/R` copies as ordinary real `Vec3` values. This field space is contractible and permits zeros. The optional SU(2)/SU(3) link buffers do not repair the electric-charge mechanism:

- they are default-off;
- their Wilson-staple relaxation is `[IMPOSED]`;
- the gauge-link regression proves they are write-only with respect to the substrate;
- there is no native compact U(1) electromagnetic link feeding the production state/flux dynamics.

These are source-provenance facts, not a claim that compact link variables are impossible.

## 6. What survives

The surviving constructive mechanism is narrower and better defined:

\[
\boxed{\text{protected defect class}}+
\boxed{\text{nonlinear common action that locks flux magnitude}}.
\]

Topology may supply a sign or sector. The action must independently supply the charge unit, radius-independent flux, energy floor, recoil, transport, and stability against crossing `J=0`. On a periodic lattice the defects must occur in net-zero combinations unless an environmental boundary or puncture carries the complementary class.

This mechanism remains `[OPEN]`. FTD-0564 closes only “topology alone quantizes electric charge.”

## 7. Non-implications

- The result does not prove that native charge cannot emerge nonlinearly.
- It does not prove that topological defects cannot exist.
- It does not turn FTD-0392/0398 into electric-charge measurements.
- It does not establish or exclude physical magnetic monopoles.
- It does not authorize a new field variable, force, scenario, or production toggle.

