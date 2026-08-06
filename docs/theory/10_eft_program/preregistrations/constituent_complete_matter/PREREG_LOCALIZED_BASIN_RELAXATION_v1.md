# PREREGISTRATION — Localized-basin relaxation v1

**Identifier:** `FTD-0678`  
**Status:** `[PREREGISTERED — NOT YET EXECUTED]`  
**Date locked:** 2026-07-28  
**Branch:** selected connected Moore-block common action; observer only  
**Production changes:** forbidden

## Question

Does a fresh, smaller excitation of the first internal cubic doublet move
toward the evolving unexcited control in the translation/boost-quotiented
internal phase metric while a positive control-relative field disturbance
becomes spatially remote, before periodic self-contact?

This is a finite-window discriminator for a **localized rest-basin candidate**.
It is not a preregistered proof of asymptotic attraction, a particle, radiation,
or an ontological object/environment cut.

## Frozen dependencies

- FTD-0676 result JSON SHA256:
  `6592E523EDDC37648A39FE39CFF02FF4371555CAEF6DE830D822114D98858206`;
- FTD-0676 tick CSV SHA256:
  `D1BB98C6C178201D9B8A289FD5E3026439D57239BEDDE235FE9010A44B888AA4`;
- embedded FTD-0676 runner SHA256 after guard-only reuse change:
  `64A8CE696E1A21C09E965348F82503F1F8D457CEDFBCE4396DF0B20862F289DE`;
- embedded FTD-0674 runner SHA256:
  `CE5B31902189D11956A4FDFC81937579797D4BF0A06CDE005A593F50731D0991`;
- embedded regional runner SHA256:
  `C7CCE67A1887672DA45EB7D50B88E926A51423F034DBAEC3080D22CC760779BC`;
- FTD-0677 observer header SHA256:
  `C2FC41FA50E187F516C4EA758248BDDBC3FFF471C082A4CDD52ADE8E800B7955`;
- FTD-0677 observer source SHA256:
  `59871E15394EF15BD562913AEC900012FA2206335A8C335D4B14B576977AE3B2`;
- FTD-0677 qualification SHA256:
  `C4D0DCC89BE5186D38E409B73EC7088A24C3324DC21D27B2D41371BC41537480`;
- FTD-0677 exact certificate SHA256:
  `D21C946A8C8B00880AB792B0721FBDA5DF1A056D673F489E705AB963E9A4BDAB`.

The campaign runner will embed the SHA256 of this protocol.  Its own hash is
recorded after implementation and before execution.  No campaign output may be
inspected before both hashes are recorded.

## Locked state and dynamics

- periodic volume `L=97`;
- horizon `t=0..80` inclusive;
- the same refined connected control, analytic Hessian, first internal cubic
  doublet `{6,7}`, matched field normalization, shared-anchor chart, sparse
  local current, and exact selected common-action step as FTD-0676;
- two excited histories, polarity signs `-1,+1`, plus one evolving unexcited
  control;
- maximum constituent momentum exactly `2.5e-7`, not used previously by the
  FTD-0676 campaign;
- initial matter differs only in momentum and all three initial matched fields
  must be bitwise identical;
- no reactions, collisions, legacy forces, source patches, damping, or
  production tick;
- complete forward evolution followed by state-only reversal of all three
  histories.

The horizon remains one tick below the registered first periodic self-contact
at tick 81.

## Locked observer

At every tick compare each excited state to the simultaneous control using
FTD-0677 with:

```text
origin       = instantaneous control constituent center,
R_in         = 8 storage cells,
R_out        = 24 storage cells,
omega_ref    = (omega_6+omega_7)/2,
beta         = matched field-work coefficient,
c            = C_SPEED,
m            = M_INERTIAL.
```

Record the collective center and mean-momentum offsets, `D_x`, `D_p`,
`D_phase`, maximum edge-length-squared change, and the near/intermediate/far
difference-field terms.  Also record the canonical target-doublet energy and
the exact common-action, total-energy, sector, observer, and inverse residuals.

The principal matter observable is

```text
R_core(t) = D_phase(t)/D_phase(0).                 (1)
```

The field observables are the three unnormalized positive shell terms and
their fractions of `H_delta`.  No shell term is called bound energy or
radiation.

## Locked algebra and execution gates

The execution is valid only if all of the following pass for both signs:

1. parent fingerprints and all frozen dependency hashes match;
2. the doublet is degenerate and the FTD-0677 observer qualification passes;
3. the two excited initial fields are bitwise identical to the control field;
4. `|p_max-2.5e-7|<=1e-15`;
5. the initial identity `D_phase(0)=2 E_target(0)` has relative residual
   `<=1e-8`;
6. every tick retains the charge/graph/sector signature and a valid observer;
7. maximum field-partition residual `<=1e-12`;
8. maximum selected-energy drift and common-action residual `<=1e-10`;
9. the complete forward/reverse recovery is `<=1e-8`.

Failure of any item yields only
`LOCALIZED_BASIN_RELAXATION_EXECUTION_INVALID`.

## Locked fit and physical classifiers

Fit `log R_core(t)=a-Gamma_core t` over every integer tick `8..64`.  Compare
that two-parameter line to a one-parameter constant with BIC.  No fit window is
changed after execution.

For each sign define:

- **core decline:** `1-R_core(64)/R_core(8) >= 0.20`;
- **exponential evidence:** `Gamma_core>0`, `DeltaBIC>=10`, and `R^2>=0.995`;
- **remote field at tick 80:** `H_far(80)>H_near(80)` and
  `H_far(80)>0`.

Polarity consistency requires:

```text
relative Gamma_core difference <= 1e-4,
RMS difference of R_core histories <= 1e-5,
absolute far-fraction difference at tick 80 <= 1e-4.
```

The ordered verdict is:

1. invalid algebra/execution ->
   `LOCALIZED_BASIN_RELAXATION_EXECUTION_INVALID`;
2. either core decline fails ->
   `LOCALIZED_BASIN_INTERNAL_RELAXATION_ABSENT`;
3. either exponential-evidence gate fails ->
   `LOCALIZED_BASIN_INTERNAL_RELAXATION_NONEXPONENTIAL`;
4. either remote-field gate fails ->
   `LOCALIZED_BASIN_REMOTE_FIELD_NOT_DOMINANT`;
5. polarity consistency fails ->
   `LOCALIZED_BASIN_RELAXATION_SIGN_DEPENDENT`;
6. otherwise ->
   `LOCALIZED_BASIN_RELAXATION_TOWARD_CONSTRUCTIVE`.

The final verdict licenses only: within the registered pre-contact window, the
internal phase state moves toward the evolving rest control while the positive
difference-field norm is more abundant outside radius 24 than inside radius 8.
It does not license “stable particle,” “bound dressing,” “radiation,” or
asymptotic decay.

## Run of record

- Release CPU executable registered as CTest `localized_basin_relaxation`;
- versioned JSON summary and per-tick CSV under `engine/results/ftd_0678/`;
- independent Python verifier must recompute every fit, residual, and verdict
  from the CSV/JSON without importing engine code;
- toolchain: pinned MSVC `14.44.35207`;
- runner returns nonzero only for execution-invalid, not for a physical
  closed-negative classification.
