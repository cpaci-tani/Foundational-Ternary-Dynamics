# FTD-0477 — Selected-Force Moving-Source Reciprocity Discriminator v1

**Date locked:** 2026-07-25  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parents:** `FTD-0288`, `FTD-0297`, `FTD-0435`, `FTD-0438`, `FTD-0476`  
**Production tick:** frozen; observation and scenario admission only

## 1. Question

Can a spatially separate finite flux packet cause a previously resting
manifested polarity to move through the existing selected production force,
and does the resulting matter-conditioned field qualify as a co-moving
dressing, a wake, a detached outgoing field, or a reciprocal energy-momentum
exchange?

This is the required successor to FTD-0476.  The source velocity is never
prescribed.  It begins at rest, and every later remainder change, integer hop,
and velocity change must be produced by the frozen tick.

## 2. Scope restriction

The accelerating channel is the toggle-gated extension

$$
F_s=G_Cs\,\nabla |J|_{r=2}.
$$

FTD-0435 established that this channel is a mixed-polarity
self-field/interference response, not ordinary electric `qE` and not a result
of the five frozen postulates alone.  Therefore every positive outcome below
is restricted to the **selected production extension**.  No outcome may be
called native electromagnetism, a photon, a gauge field, or an ontological
proof of an aura.

## 3. Frozen profile and initial data

- backend: Windows/MSVC CPU;
- lattice: `L=65`, periodic computational boundary;
- ticks: `72`, with recorded samples at `0,16,32,48,56,72`;
- `dt=1`, FULL 18-point Moore wave stencil;
- source location: `(32,35,32)`;
- source state: `s=+1` or `s=-1`, unlocked, zero velocity, zero remainder;
- driver: the existing divergence-free transverse `+x` packet constructed as
  `curl(psi e_x)`;
- driver centre: `(12,32,32)`;
- driver widths: `sigma_x=sigma_t=3`;
- driver amplitude: `0.5`;
- driver carrier: `k=0`, phase `0`;
- source-site `J` and `wave_vel` must be exactly zero at tick zero;
- active Boolean terms: `wave_propagation`, `coupling`, `forces`, `movement`,
  `emergent_forces`, and `strict_validation` only;
- all damping, projection, reaction, gravity, Poisson, Lorentz, color, strong,
  weak, dual-substrate, stochastic, counterterm, and auxiliary gauge terms are
  off.

No amplitude, width, offset, duration, or gate scan is permitted.  A failed
motion gate is a result, not authorization to strengthen the driver.

## 4. Matched arms

1. `driver_only`;
2. `positive_source_only`;
3. `negative_source_only`;
4. `locked_positive_plus_driver`;
5. `mobile_positive_plus_driver`;
6. `mobile_negative_plus_driver`;
7. exact repeat of `mobile_positive_plus_driver`.

The driver-only arm supplies the field counterfactual.  Source-only arms
separate self-response from driver-caused motion.  The locked arm establishes
that the scenario contains no scripted transport.  The opposite polarity arm
measures, but does not assume, odd/even response.

## 5. Frozen observables

### 5.1 Causation and coast

- particle trajectory including integer position and fractional remainder;
- production movement-journal count;
- force and velocity histories;
- driver activity within radius four of the initial source;
- combined-minus-source-only displacement and velocity;
- peak interaction force during ticks `16..48`;
- RMS interaction force during ticks `57..72`.

`CAUSED_INTEGER_MOTION` requires:

- all sources survive;
- locked-source displacement `<=1e-12`;
- both source-only displacements `<=1e-9`;
- driver-near-source peak activity `>1e-4`;
- mobile combined-minus-source-only displacement `>=0.5`;
- at least one production movement event;
- exact-repeat residual `<=1e-12`.

`COAST_INTERVAL` additionally requires final speed `>=1e-3` and post-driver
RMS interaction force no greater than `0.25` of the earlier peak.

### 5.2 Matter-conditioned field

At each sample, define

$$
(J_s,W_s)=(J,W)_{\rm source+driver}-(J,W)_{\rm driver-only}.
$$

This subtraction is an observer; it is not fed back into the tick.  Activity
uses the exact quadratic modified-wave-energy functional evaluated on
`(J_s,W_s)`.

At ticks `56` and `72`, measure around the actual source position:

- radius-four near fraction;
- leading, trailing, and transverse fractions relative to the net source
  displacement;
- activity-weighted mean radius;
- normalized correlation with the same-tick source-only field translated by
  the source's integer displacement.

`CO_MOVING_DRESSING_CANDIDATE` requires at both ticks:

- near fraction `>=0.75`;
- translated correlation `>=0.80`.

`WAKE_CANDIDATE` requires at tick 72:

- trailing fraction `>=0.15`;
- trailing/leading activity ratio `>=2`.

`DETACHED_OUTGOING_FIELD_CANDIDATE` requires from tick 56 to 72:

- mean-radius growth `>=2.0`;
- near-fraction drop `>=0.20`;
- positive detached activity outside radius four at both samples;
- `COAST_INTERVAL` passes.

These morphology labels may coexist.  None is a radiation claim without the
reciprocity gate.

### 5.3 Energy and momentum reciprocity

For each tick define the inclusion-exclusion energy change

$$
R_E(t)=\Delta E_{s+d}(t)-\Delta E_s(t)-\Delta E_d(t),
$$

where `E` is the production dynamic-energy audit.  Define the selected total
momentum using production particle momentum plus the FTD-0438 central field
translation generator, and form the analogous inclusion-exclusion residual
`R_P(t)`.

Normalize energy by the sum of initial source and driver dynamic energies and
momentum by the maximum of driver field momentum, particle impulse, and
`1e-30`.

`RECIPROCAL_SELECTED_EXTENSION` requires over all ticks:

- maximum normalized `|R_E| <=1e-6`;
- maximum normalized `|R_P| <=1e-6`;
- `CAUSED_INTEGER_MOTION` passes.

The current audits omit some interaction energies.  Failure therefore closes
reciprocity **for the registered audit and production extension**; it does not
prove that no enlarged action could close.

## 6. Decision table

- If caused motion fails: `NO_DYNAMICAL_MOVING_SOURCE_IN_REGISTERED_PROTOCOL`.
- If caused motion passes but reciprocity fails:
  `DYNAMICAL_MOTION_WITHOUT_CLOSED_RECIPROCITY`.
- If reciprocity passes but every morphology gate fails:
  `RECIPROCAL_MOTION_WITHOUT_QUALIFIED_DRESSING_WAKE_OR_DETACHED_FIELD`.
- If reciprocity and one or more morphology gates pass:
  `SELECTED_EXTENSION_<PASSED_LABELS>`.

No electromagnetic, pilot-wave, photon, or physical-radiation terminology is
licensed by this campaign alone.

## 7. Scenario admission

Add `s0-seed-moving-source-reciprocity` with the identical term profile and
initial data scaled only geometrically for the chosen dashboard lattice size.
The visual profile may enable:

- `|J|` volume;
- `J` integral curves;
- ternary state;
- `-wave_vel` field-change proxy;
- the existing Poynting-like overlay.

The scenario description must say that streamlines are instantaneous integral
curves and that `-wave_vel`/Poynting are diagnostic proxies.  Any user toggle
change suspends qualification through the existing profile guard.

## 8. Required artifacts

- read-only campaign: `engine/tests/campaign_reciprocal_moving_source.cpp`;
- mechanical scenario gate:
  `engine/tests/test_reciprocal_moving_source_scenario.cpp`;
- versioned CSV and verdict files under `engine/results/ftd_0477/`;
- audit, manifest, ledger row, trackers, and both navigation indexes;
- CPU focused tests, scenario parity/browser checks, and unchanged golden gate.

