# FTD-0660 — Internal-mode action-transfer ledger v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only selected-action campaign  
**Parent:** FTD-0659 `NATIVE_EXCITED_MATTER_CLOCK_MIXED`

## 1. Question

When the first internal constituent doublet loses its matter-only action, does
the complete common-action state place the complementary excitation in the
instantaneous co-moving dressing, a dynamic residual field, other constituent
modes, or an unresolved mixture?

FTD-0659 already measured the action loss. This campaign is a prospective
destination ledger, not an independent rediscovery of that loss.

## 2. Exact field decomposition

For every complete state `z=(X,p,E,B)`, construct an observer-only
instantaneous longitudinal dressing `D(X)` by redressing the same constituent
geometry with the already qualified Poisson solve. Define

\[
E_r=E-E_D(X),\qquad B_r=B,
\]

and, using the selected modified field quadratic `H_F`,

\[
H_D=H_F[E_D,0],\quad H_R=H_F[E_r,B_r],\quad
H_X=H_F[E,B]-H_D-H_R.
\]

`H_X` is the exact quadratic interference term. The identity

\[
H_F=H_D+H_R+H_X
\]

must hold record by record. `H_R` is a dynamic-residual field energy, not yet
radiation.

Also record the positive residual-field cell norm

\[
N_R={\beta\over2}\sum_x(|E_r(x)|^2+|B_r(x)|^2)
\]

in periodic cell-index shells `near: r<3`, `middle: 3<=r<5.5`, and
`far: r>=5.5` about the constituent centre. This norm is a morphology observer,
not an additional energy term.

## 3. Locked arms

Reuse the FTD-0659 first internal doublet and state preparation at `L=17`.
Use:

- two cyclic orientations;
- polarizations `(1,0)` and `(1,1)/sqrt(2)`;
- maximum displacement-equivalent amplitudes `4e-6` and `8e-6`;
- momentum quadratures `pi/2` and `3pi/2`, so the initial constituent geometry
  is the exact dressed rest geometry and the initial dynamic residual field is
  zero to numerical precision;
- `128` forward ticks followed by `128` state-only inverse ticks.

This gives `16` nonzero arms. Add one zero-amplitude arm per orientation for
`18` total. No force, reaction, field packet, parameter change, or production
toggle is allowed.

## 4. Locked observers

At every forward state record:

- matter-doublet harmonic energy `E_2=omega I` and ratio to its initial value;
- total kinetic/rest, binding, actual field, dressing, dynamic-residual, and
  interference energies relative to the exact rest reference;
- exact field-decomposition and complete-energy residuals;
- all matter-mode coordinates/momenta and non-target group norm;
- total, near, middle, and far positive residual-field norms;
- common-action, Gauss, continuity, sector, chart, hop, energy, and inverse
  diagnostics.

For each shell define onset as the first tick at which its norm reaches `25%`
of that shell's maximum over ticks `0..128`.

## 5. Gates and classifications

### 5.1 Exact execution

Require all `18` arms to initialize, complete, remain in sector with no hops,
and invert; common-action residual `<=1e-10`, total-energy drift `<=1e-12`,
field-decomposition residual `<=1e-12`, and inverse recovery `<=1e-10`.
Zero controls must keep every perturbation energy/norm `<=1e-20`.

### 5.2 Transfer detection

For every nonzero arm require:

- matter-doublet energy falls below `60%` of its initial value by tick `128`;
- maximum dynamic-residual field energy or positive norm is at least `5%` of
  the initial total excitation energy;
- amplitude-normalized energy and norm histories agree within `5%`;
- sign-mirrored histories agree in every quadratic ledger within `5%`;
- cyclic-orientation histories agree within `5%`.

### 5.3 Morphology classification

- **`INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE`:** exact and transfer
  gates pass, the far shell reaches at least `10%` of maximum total residual
  norm, and shell onsets satisfy `near <= middle <= far` in every nonzero arm.
- **`INTERNAL_MODE_LOCAL_HYBRID_TRANSFER_CONSTRUCTIVE`:** exact and transfer
  gates pass, far-shell fraction stays below `10%`, and matter-doublet energy
  recovers above `80%` after first falling below `60%`.
- **`INTERNAL_MODE_ACTION_TRANSFER_MIXED`:** exact execution passes but neither
  complete morphology conjunction does.
- **`INTERNAL_MODE_ACTION_TRANSFER_CLOSED_NEGATIVE`:** a valid arm cannot
  execute as an exact bounded reversible history.
- **`INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID`:** provenance, coverage,
  redressing, solver, or record completeness fails.

The dynamic-field classification is not permission to call the residual a
photon or asymptotic radiation. Volume scaling and an outgoing-flux/return-time
test are required for that language.
