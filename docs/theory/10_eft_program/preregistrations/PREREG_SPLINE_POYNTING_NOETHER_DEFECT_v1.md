# FTD-0619 — Spline-Poynting / Noether-defect discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only momentum classification for the FTD-0618 balanced gait
**Production change:** forbidden

## 1. Frozen parent and question

Require the FTD-0618 run-of-record SHA-256
`5F04E64DFD7CBFD10CE3AC779361C4124654C817320DFC81E6D5A482889F54D3`
and reconstruct its exact rest, `sigma=+1`, and `sigma=-1` arms with the
unchanged six-constituent common-action solver.  The action, quadratic coat,
face current, orbit gather, binding, field normalization, dispersion, volume,
initial conditions, solver tolerances, and 128-tick duration are frozen.

FTD-0618 showed that the declared local field pseudomomentum does not close
with matter momentum.  This campaign asks a narrower question: does the
already locked spline reconstruction supply a geometrically normalized
Poynting momentum that closes instead, or is the observed imbalance a genuine
continuous-translation defect of the fixed-lattice action?

## 2. Locked spline field momentum

Use exactly the FTD-0550 reconstructions already used by the particle action:

```text
E_h: B1 in the face-component direction, B2 transversely,
B_h: B2 in the edge-component direction, B1 transversely.
```

At an integer-time state `(E_n,B_(n-1/2))`, define the co-temporal magnetic
field

```text
B_n = B_(n-1/2) - (lambda/2) C^T E_n,
lambda = C_SPEED dt.                                      (1)
```

The only new candidate is

```text
P_spl(E_n,B_(n-1/2))
  = (beta/C_SPEED) integral_cell E_h(x) cross B_n,h(x) d^3x. (2)
```

Here `beta` is the already frozen face-field interaction scale.  The
`1/C_SPEED` normalization is fixed before execution by the long-wave relation
`P=U/c` for a directed source-free wave with `|E|=|B|`; it is not fitted to
the gait.  The periodic-cell integral must be evaluated from exact
one-dimensional B-spline overlap stencils.  A direct piecewise Gauss rule,
exact for the polynomial degree, must reproduce the overlap implementation
within `1e-12` on a deterministic `L=7` challenge.

No shifted stencil, smoothing, fitted scale, rest subtraction, field-only
renormalization, or alternate interpolation may be tried after execution.

## 3. Source-free eligibility gate

Before applying (2) to matter, run the unchanged staggered source-free update
for 256 ticks at `L={16,17}`, all three axes, and both propagation signs.  Use
mode number two and amplitude `0.02`, with the electric and magnetic phases
fixed as in FTD-0473 and the magnetic sign selecting direction.

Record the initial momentum, maximum absolute drift, maximum relative drift,
and sign/axis covariance for both:

1. the existing exact local pseudomomentum `P_sel`; and
2. the new spline-Poynting candidate `P_spl`.

`P_spl` is eligible to close a matter ledger only if every source-free arm has
nonzero initial magnitude, maximum absolute drift at most `1e-10`, maximum
relative drift at most `1e-10`, and sign/axis covariance residual at most
`1e-12`.  Failure is a result: the candidate remains an energy-flow
diagnostic, not a conserved momentum.

## 4. Coupled channel decomposition

For every forward transaction in the three FTD-0618 arms record independently:

```text
Delta p_m       = p_m,1 - p_m,0,
I_E             = sum_a electric_impulse_a,
I_B             = sum_a magnetic_impulse_a,
I_bind          = sum_a binding_impulse_a,
Delta P_sel     = P_sel,1 - P_sel,0,
Delta P_spl     = P_spl,1 - P_spl,0,
R_sel           = Delta p_m + Delta P_sel,
R_spl           = Delta p_m + Delta P_spl.       (3)
```

Require `|Delta p_m-I_E-I_B-I_bind| <= 1e-12` and
`|I_bind| <= 1e-12` on every tick.  Record per-channel maxima, signed axial
sums, cumulative vectors, and active-sign mirror residuals.  These quantities
are decomposed measurements, not a definition of a new substrate state.

The spline candidate closes only if it passed Section 3 and the maximum
cumulative `R_spl` magnitude over all three arms is at most `1e-10`.  The
existing candidate is reported under the same gate.  No residual may be
renamed `P_substrate` or accumulated as an ontic variable: without an
independently specified state function, `R` is only the Noether defect caused
by the absent microscopic continuous-translation symmetry.

## 5. Verdicts

- `SPLINE_POYNTING_CLOSES_BALANCED_GAIT`: all protocol/algebra gates pass,
  `P_spl` passes the source-free gate, and cumulative `R_spl <= 1e-10`;
- `CONTINUOUS_TRANSLATION_DEFECT_MEASURED`: all protocol/algebra gates pass,
  `P_spl` passes the source-free gate but neither field candidate closes the
  coupled gait;
- `SPLINE_POYNTING_NOT_CONSERVED`: all protocol/algebra gates pass but
  `P_spl` fails the source-free eligibility gate;
- `MOMENTUM_CHANNEL_DISCRIMINATOR_NUMERICALLY_UNRESOLVED`: any parent,
  reconstruction, overlap, solve, channel-identity, record, or covariance
  gate fails.

Even the strongest verdict establishes only a momentum ledger for the
selected periodic observer system.  It does not establish a physical
particle, Lorentz recovery, a unique ontological momentum, or derivation from
the five postulates.  A negative verdict does not license a fitted recoil or
new production force; it selects the next question—infrared suppression of
the translation defect or adoption of an explicit canonical/connection
degree of freedom.

**Protocol lock:** `protocol_sha256=F2E97844E14B77C152E986CD2CA317337FEE04E2367F73AD4A73FD76FE61E107`
