# FTD-0606 — Global orientation × strain matter core v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Scope:** observer-only compact-core discriminator using the unchanged
FTD-0601 common-action transaction and FTD-0602 minimum-energy Gauss field.  
**Production change:** forbidden.  
**Protocol lock:** `protocol_sha256=EC0CECED1CCF40187BCE0C4B38DA34039B5CAD94069AFD05F16420D25D99494A`

## 1. Ontological question

Does the constituent phase space already selected in FTD-0600 contain a
stable dressed compact core once global orientation is separated from local
strain, or was the FTD-0605 boundary escape only a coordinate-chart artifact?

No new constituent, force, field component, memory, phase, connection, or
persistent state is introduced. This protocol only reparameterizes the same
six zero-centroid constituent coordinates as three global orientation and
three internal strain coordinates.

## 2. Frozen configuration family

Use the FTD-0602 neutral pair at `L=17`, with unchanged charges, group centres,
relative centre separation, quadratic polarity coat, face-flux normalization,
minimum-energy periodic Gauss field, production dispersion, quartic
intratrimer binding, and FTD-0601 common-action solver.

Let `z_a in R^2` be the body-frame coordinates of the FTD-0601 reference
trimer in an orthonormal basis of its plane. Define

\[
 H(h)=\begin{pmatrix}h_0&h_1\\h_1&h_2\end{pmatrix},\qquad
 A(h)=I+H(h),\qquad
 r_a(R,h)=R B A(h)z_a,
\]

with `R in SO(3)`. The second trimer uses the exact charge-conjugate mirror
offsets `-r_a`. The strain basin is `|h_i| <= 0.20`, with an interior margin
of `1e-4`; `A` must be positive definite with minimum eigenvalue at least
`0.70`. Global orientation has no artificial coordinate boundary.

The exact identities to verify before accepting any numerical result are:

- `sum_a r_a = 0`;
- `det R = +1` and `R^T R = I`;
- rigid rotation leaves all three internal distances and binding energy
  unchanged to `1e-12`;
- the strain-only binding Hessian has the same three positive eigenvalues as
  the nonzero spectrum of the FTD-0605 six-coordinate Hessian, up to the
  explicitly derived coordinate congruence.

Every internal pair distance must remain in `[0.5,2.0]`.

## 3. Static energy and global search

At each trial state minimize the unchanged static functional

\[
 U_f(R,h)=V_{\rm bind}(h)
 +\beta\,\frac12\langle\rho(R,h),G_L\rho(R,h)\rangle.
\]

Use the same deterministic periodic Green kernel and independent direct-field
rebuild as FTD-0605. For each phase `f=j/32`, `j=0,...,31`, launch 24
independent deterministic searches from the 24 proper cubic rotations, all at
zero strain. Each search uses a six-dimensional local coordinate
`(omega_x,omega_y,omega_z,h_0,h_1,h_2)`, with
`R=exp([omega]_x) R_start`, and deterministic Nelder-Mead coefficients
`(1,2,1/2,1/2)`. Initial steps are `0.03` radians in orientation and `0.01` in
strain; the cap is 1,500 objective evaluations per start; termination requires
simplex diameter at most `1e-7` and energy spread at most `1e-14`.

No warm start across phase, favourable-phase selection, widened strain basin,
or post-inspection restart is allowed.

## 4. Coverage, stationarity, and transaction gates

For every phase:

- at least 18 of 24 starts terminate within the cap;
- at least three terminated starts land within `1e-10` energy of the best
  terminated state;
- the best state is at least `1e-4` inside the strain basin and satisfies the
  positive-definite and distance gates;
- a local tangent-space central gradient with `h=1e-4` is at most `5e-7`;
- the symmetric tangent-space `6x6` Hessian with `h=2e-3` has no eigenvalue
  below `-5e-6` and all six eigenvalues exceed `1e-6`;
- relaxed energy does not exceed the unrotated zero-strain reference by more
  than `1e-12`;
- direct Gauss, curl-adjoint, Green/direct energy, and rigid-rotation identity
  residuals are at most `1e-11`;
- the unchanged FTD-0601 forward common-action residual is at most `1e-12`;
- state-only inverse recovery is at most `1e-10`;
- exact integer translation agrees to `1e-12`.

Phase-robust attraction additionally requires inward impulse greater than
`1e-10` and decreasing centre separation at every phase. Reference and
relaxed barriers are recorded only when all 32 phases have qualified states.

## 5. Verdicts

- `GLOBAL_ORIENTATION_STRAIN_PHASE_ROBUST_CONSTRUCTIVE`: every algebraic,
  coverage, stationary-core, transaction, periodicity, and attraction gate
  passes;
- `GLOBAL_ORIENTATION_STRAIN_STABLE_FORCE_SIGN_FAILS`: every gate except
  phase-robust attraction passes;
- `GLOBAL_ORIENTATION_STRAIN_COMPACT_CORE_CLOSED_NEGATIVE`: global-search
  coverage passes at every phase, but at least one phase has no interior
  positive-definite stationary core or fails the distance/stability/energy
  gate;
- `GLOBAL_ORIENTATION_STRAIN_NUMERICALLY_UNRESOLVED`: search coverage,
  algebraic identity, direct-field, transaction, inverse, or periodicity
  coverage fails, so the ontological branch is not classified.

A negative closes only this compact `SO(3) ×` local-strain family. An
unresolved result authorizes an improved preregistered numerical method, not a
physical conclusion. A constructive result establishes only a selected
compact-core existence result. No verdict licenses a physical particle,
electron, electromagnetic ontology, pole, Lorentz recovery, toggle, scenario,
or production adoption.
