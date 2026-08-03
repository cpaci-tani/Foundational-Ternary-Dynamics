# PRE-REGISTRATION — Native Temporal Occupancy v1

**Date locked:** 2026-08-02  
**Identifier:** `FTD-0772`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0640`, `FTD-0658`, `FTD-0659`, `FTD-0770`, `FTD-0771`  
**Campaign type:** `[LOCKED RETROSPECTIVE REANALYSIS]` of an immutable native
corpus; this is a falsifier/pilot, not a prospective confirmation.  
**Scope:** observer-only analysis. No engine state, update rule, Hamiltonian,
clock variable, phase variable, coupling, calibration, toggle, scenario,
production path, or golden state may change.

## 0. Question and epistemic firewall

The registered question is:

> Does the signed, pre-existing fixed-ray coordinate of the only currently
> registered native phase-bearing candidate possess a stationary,
> amplitude-invariant tick-occupancy law consistent with the quartic
> coordinate law rather than the quadratic or sextic controls?

The campaign may derive exact mathematical consequences of an assumed
one-dimensional natural Hamiltonian and may compare a locked native observer
with those consequences. It must not insert `q^4`, fit a power `m`, choose a
new mode after inspection, normalize by an observed sample maximum, apply a
nonlinear remapping of the coordinate, or call a finite tick histogram an
exact continuous invariant measure.

The following claims remain outside scope even after a positive pilot:

- a derivation of `G*` from the substrate;
- an intrinsic rest clock or universal physical time;
- synchronization to the primitive tick;
- a derived action, phase-response curve, coupling, compliance, or holonomy;
- a global quartic potential; and
- a production engine primitive.

## 1. Locked exact target

For the selected comparison family

```text
h_m(q,p) = (p^2+|q|^m)/2,       even m >= 2,
A = (2E)^(1/m),
x = q/A,
```

lock the continuous-time coordinate occupancy and its absolute moments:

```text
rho_m(x) = m/[2 B(1/m,1/2)] / sqrt(1-|x|^m),   -1 < x < 1,
E_m[|x|^r] = B((r+1)/m,1/2)/B(1/m,1/2),        r > -1.
```

For `m=4`, with `G*=Gamma(1/4)/Gamma(3/4)`, require

```text
rho_4(x) = 2/[sqrt(pi)G* sqrt(1-x^4)],
mu_1 = E[|x|] = sqrt(pi)/G*,
mu_2 = E[x^2] = 4/G*^2,
mu_4 = E[x^4] = 1/3,
G_rms = 2/sqrt(mu_2),
G_abs = sqrt(pi)/mu_1.
```

The two `G` expressions are algebraically distinct consistency checks on one
trajectory, not statistically independent discoveries.

The exact comparison CDF is

```text
F_m(x) = 1/2 + sign(x)/2 I_(|x|^m)(1/m,1/2),   |x| < 1,
F_m(x) = 0 for x <= -1,                         F_m(x) = 1 for x >= 1.
```

Only `m={2,4,6}` may be evaluated. There is no fitted exponent, parameter
scan, near-miss search, or post-hoc observable substitution.

## 2. Conditional characterization theorem

The exact certificate must prove the following statement.

> **Fixed-coordinate quartic occupancy characterization.** Let `I` be a
> nonempty interval of positive turning amplitudes and let `V` be an even
> `C^2` potential with `V(0)=0` on the region swept by those amplitudes.
> Assume a fixed unit-mass natural coordinate `q`, finite-period regular
> oscillations traversing `[-A,A]`, and `V(A)>V(q)` for `|q|<A`. The
> amplitude-normalized continuous-time occupancy equals `rho_4` for every
> `A in I` if and only if `V(q)=lambda q^4`, `lambda>0`, throughout the swept
> region.

The result is invariant under constant rescaling of time and affine changes
of the coordinate unit. It is not invariant under a general nonlinear
observable transformation. It characterizes a fixed natural coordinate; it
does not select that coordinate from FTD.

## 3. Locked native corpus

Use only the already generated FTD-0659 artifacts:

```text
engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_v1.json
  SHA256 DB6CA66770812E4C8FC94411B109F23E424FFF1CE3173A5D16AB43B5949ACEEE
engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_arms_v1.csv
  SHA256 4F7D2E38B0FE4D6EF33F137E2AA753E4143B3AD541F6934CF39FD11772844941
engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_ticks_v1.csv
  SHA256 4EF51456F161E6CD836518B72EBAACE4A5007F5EF5525E07CD097B343566634A
```

The parent protocol SHA256 is
`FF9566F6D6B7BCAEB7970359043C62F643A6A8315AF43C01EE0C5CFD21ECC342`.
The corpus contains `74` arms and ticks `0..256`; only the `72` nonzero arms
enter occupancy analysis. The two exact rest controls must remain zero and
must not be assigned an occupancy.

The campaign must not rerun or overwrite FTD-0659. A provenance, schema,
coverage, or finite-value failure gives `NATIVE_TEMPORAL_OCCUPANCY_EXECUTION_INVALID`.

## 4. Locked coordinate and normalization

The native candidate is the prepared first non-rigid FTD-0640 doublet with
modal coordinates `q=(q_6,q_7)`. For the polarization registered before each
FTD-0659 history, lock

```text
u_0 = (1,0),
u_1 = (0,1),
u_2 = (1,1)/sqrt(2),
Q_u(t) = u dot q(t),
x(t) = Q_u(t)/A,
A = modal_amplitude from the parent arm record.
```

`Q_u` is the signed mass-metric projection along the prepared ray. Under an
orthogonal basis change inside the degenerate doublet, `q` and `u`
co-transform and `Q_u` is unchanged. Its origin is the registered dressed
rest state. Its sign is fixed by the parent preparation; no sign fold is
allowed.

Do not replace `Q_u` by `sqrt(q_6^2+q_7^2)`: the radial norm erases the two
turning directions and can map a rotating doublet to a constant envelope.
Do not normalize by `max|Q_u|`, a fitted amplitude, a window-dependent
amplitude, or any nonlinear function of `Q_u`.

For each arm define the transverse fraction

```text
epsilon_perp = sum_t (|q(t)|^2-Q_u(t)^2) / sum_t |q(t)|^2.
```

Require `epsilon_perp<=0.05`. Failure makes the one-dimensional fixed-ray
observer inapplicable; the analysis may not switch observables.

## 5. Locked sampling and diagnostics

There are `18` primary cells indexed by `(orientation,polarization,amplitude)`.
Within each cell:

- quadrature `0` (`q(0)=A u`, `p(0)=0`) is the primary history;
- quadrature `2` is the sign-mirror control;
- quadratures `1` and `3` are quarter-cycle/time-origin controls;
- every recorded tick `0..256` receives equal weight;
- the primary history is split into the fixed windows `0..85`, `86..171`,
  and `172..256`;
- empirical CDF distance uses the two-sided one-sample Kolmogorov distance;
  pairwise sample comparisons use the two-sided two-sample Kolmogorov
  distance; and
- no KDE, endpoint-singular density bin, bootstrap p-value, or effective
  independent-sample claim is permitted.

For every arm/cell report sample count, maximum `|x|`, signed mean, `mu_1`,
`mu_2`, `mu_4`, `G_rms`, `G_abs`, `D_2`, `D_4`, `D_6`, transverse fraction,
and the parent action/phase/support diagnostics. Report aggregate maxima; do
not hide a failing cell behind a pooled mean.

Because global time is discrete, the measured object is the atomic empirical
tick measure

```text
mu_N = (1/N) sum_n delta_(x_n).
```

Comparison with `rho_m(x)dx` is a coarse-grained/equidistribution pilot. An
exact finite `P`-tick orbit never has the absolutely continuous target law.
No continuous native occupancy theorem is licensed without a separate
within-tick flow, refinement limit, or invariant-circle/equidistribution
result.

## 6. Locked applicability and stationarity gates

All primary cells must pass all of the following before any quartic-shape
result can be promoted even to a retrospective candidate:

1. at least `8` physical cycles, measured from the parent's doubled-phase
   span divided by `4*pi`;
2. `epsilon_perp<=0.05` for every quadrature;
3. `max|x|<=1.05` globally;
4. each of the three primary windows reaches `max|x|>=0.85`;
5. maximum pairwise two-sample CDF distance among the three primary windows
   is `<=0.10`;
6. the window spread of each of `mu_1`, `mu_2`, and `mu_4` is `<=0.05`;
7. each control quadrature differs from the primary by CDF distance `<=0.10`
   and by at most `0.05` in every registered moment; and
8. the parent execution, energy, inverse-recovery, phase-defined, support,
   leakage, and common-action gates pass.

The parent's harmonic-proxy action drift is reported but is not itself a
quartic applicability gate: that proxy is not the conserved action of a
quartic natural Hamiltonian. Amplitude loss is instead tested directly by
the fixed-`A` window and stationarity gates above.

Across the three amplitudes at fixed orientation/polarization, require
pairwise CDF distance `<=0.075` and spread `<=0.04` in each registered moment.
The same thresholds apply across the two orientations and three
polarizations after matching amplitude index. These are amplitude/covariance
controls, not additional fitted samples.

Failure of recurrence, one-dimensionality, stationarity, or covariance gives
`NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED`. Descriptive control
distances and moments must still be emitted, but they cannot establish an
invariant native clock.

## 7. Locked quartic-shape gate

For each of the `18` primary cells require simultaneously:

```text
D_4 <= 0.04,
D_4 < D_2 and D_4 < D_6,
|mu_1-sqrt(pi)/G*| <= 0.02,
|mu_2-4/G*^2| <= 0.02,
|mu_4-1/3| <= 0.02,
|mean(x)| <= 0.02,
relative_error(G_rms,G*) <= 0.02,
relative_error(G_abs,G*) <= 0.02,
relative_difference(G_rms,G_abs) <= 0.02.
```

The full-CDF and all moment guards must pass; one matching moment is not
sufficient. The closest member of the locked `m={2,4,6}` control set is
reported descriptively by minimum CDF distance. It is not a fitted power.

## 8. Natural-coordinate firewall

The FTD-0659 corpus does not record a complete native acceleration closure
for `Q_u`. Therefore it cannot establish that acceleration is a
single-valued function of `Q_u`, independent of velocity, history, hidden
field state, and branch. The characterization theorem must be reported as
**inapplicable to native-potential inference** in v1 even if the empirical
quartic-shape gate passes.

An unexpected positive pilot requires a fresh preregistered run with a
complete-state recurrence observer, a natural-coordinate closure test,
longer histories, and volume controls before any effective quartic-potential
claim.

## 9. Verdict map

- provenance/schema/coverage failure:
  `NATIVE_TEMPORAL_OCCUPANCY_EXECUTION_INVALID`;
- fixed-ray, cycle, stationarity, amplitude, control-quadrature, parent, or
  covariance gate failure:
  `NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED`;
- every applicability gate passes but the complete quartic-shape gate fails:
  `NATIVE_QUARTIC_OCCUPANCY_CLOSED_NEGATIVE_FOR_FTD0659`;
- every applicability and quartic-shape gate passes:
  `NATIVE_QUARTIC_OCCUPANCY_RETROSPECTIVE_CANDIDATE` plus
  `FRESH_CONFIRMATION_REQUIRED`;
- any claim of an exact continuous measure, derived native potential, derived
  `G*`, intrinsic rest time, or universal time from this corpus:
  `SCOPE_PROMOTION_INVALID`.

The final deliverable must include the immutable protocol hash, exact
certificate, versioned analysis script, versioned ignored result artifacts,
independent result certificate, canonical analysis, and synchronized theory
navigation.
