# FTD-0610 — Single-core neutralizer control v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** default-off observer-only matter-dynamics discriminator
**Production change:** forbidden
**Protocol lock:** `protocol_sha256=DB4363D2A132BB84BFF10218FCE8B4B20BC4C677F6FE813815F368E38A4EED85`

## 1. Question and frozen matter state

Determine whether the slow-arm failure of FTD-0609 is intrinsic to one compact
charged trimer, is induced by the localized neutralizer field, or requires the
second trimer's dynamical response.

Use the exact phase-15 static state selected by FTD-0608/0609. Extract only
constituents 3, 4, and 5, preserving their anchors, remainders, charges,
binding law, continuous effective positions, and minimum constituent distance.
This produces the charge pattern `(+1,+1,-1)` with net charge `+1`. No shape
optimization, force amplification, new binding, fitted parameter, or state
variable is permitted.

Use the FTD-0609 two-record site-chart fibre with
`allow_shared_anchor_chart=true`. That option remains default false and may
only relax the rejection of distinct constituent records sharing one integer
anchor. The strict `site_projection_valid` diagnostic remains unchanged.

## 2. Registered neutralizer controls

Run at periodic `L=17` with zero magnetic half-field. For each control, form
the total initial density from the moving trimer's three exact quadratic coats
plus the registered stationary density and initialize the unique zero-mean,
minimum-energy longitudinal electric field.

1. **Uniform control:** set the stationary density at every site to
   `-1/L^3`. This cancels the charged sector's zero mode while introducing no
   localized neutralizer gradient.
2. **Frozen-partner control:** set the stationary density equal to the exact
   quadratic-coat density of phase 15 constituents 0, 1, and 2. It has net
   charge `-1` and remains fixed in the laboratory chart during transport.

Both neutralizers are selected external control apparatus, not candidate
matter ontology. A constructive result under either control does not establish
an isolated charged particle or a closed momentum channel.

The total density sum, Poisson residual, initial Gauss residual, initial curl
residual, and minimum-field energy cross-check must each be at most `1e-11`.
Otherwise the corresponding arm is numerically unresolved.

## 3. Rest and transport arms

Use the unchanged FTD-0600 charged-trimer common-action transaction and the
unchanged production dispersion. For each neutralizer run:

- a zero-momentum rest arm for 16 forward ticks and 16 state-only inverse
  ticks;
- `v=1/64` for 128 forward ticks and 128 state-only inverse ticks;
- `v=1/32` for 64 forward ticks and 64 state-only inverse ticks.

All three constituents receive the same registered launch momentum. No
forward current, impulse, endpoint, solver branch, or neutralizer response may
be supplied to the reverse API.

For every tick require:

- a valid converged common-action solve;
- every registered continuity, Gauss, work, energy, causal-speed, and
  equation residual at most `1e-12`;
- same-anchor multiplicity at most two and effective constituent separation
  at least `1e-3`;
- all three internal pair distances in `[0.5,2.0]`.

For each complete forward/reverse arm require total-energy drift at most
`1e-10` and state-only recovery at most `1e-9`. The rest arm additionally
requires centre displacement and centre-momentum change at most `1e-10`.
Each moving arm additionally requires longitudinal displacement at least 75%
of the nominal two cells, transverse drift at most `0.25`, at least three
constituent anchor changes, and at least one shared-anchor state. Record, but
do not gate on or interpret as isolated momentum conservation, the field-plus-
matter pseudomomentum defect because the stationary compensator is external.

## 4. Covariance and locked comparison

Repeat the first moving step after an integer `x` translation. Translate the
frozen-partner density together with the trimer; the uniform density is
unchanged. Require state, field, and diagnostic covariance at `1e-12`.

Use the already locked FTD-0609 slow dynamic-pair result only as a reference:
128/128 solves completed but longitudinal displacement was `0.2833`, pair
separation changed by `1.0626`, and that arm failed. Do not rerun, refit, or
change the FTD-0609 gates in response to FTD-0610.

## 5. Verdicts

- `SINGLE_CORE_MOBILE_LOCALIZED_NEUTRALIZER_FORCE_ISOLATED`: the uniform rest
  and both uniform transport arms pass, while at least one frozen-partner
  transport arm fails a physical gate after complete solver coverage;
- `SINGLE_CORE_MOBILE_DYNAMIC_PARTNER_RESPONSE_ISOLATED`: both controls' rest
  and transport arms pass, leaving the moving partner response as the
  registered difference from the failed FTD-0609 slow arm;
- `SINGLE_CORE_MOBILE_FROZEN_PARTNER_COMPATIBLE`: the uniform arms pass and
  both frozen transport arms pass, but a frozen rest arm fails; mobility is
  constructive while the extracted static reference is not stationary in the
  localized field;
- `SINGLE_CORE_STATIC_REFERENCE_NOT_ISOLATED`: the uniform rest arm fails its
  physical rest gate after complete forward/reverse coverage;
- `SINGLE_CORE_COMPACT_TRANSPORT_CLOSED_NEGATIVE`: the uniform rest arm passes,
  all uniform solver coverage is complete, and at least one uniform moving arm
  fails a registered physical or inverse gate;
- `SINGLE_CORE_NEUTRALIZER_CONTROL_NUMERICALLY_UNRESOLVED`: state construction,
  minimum-field initialization, solver coverage, covariance, or records are
  incomplete.

The verdict hierarchy is numerical coverage first, then uniform rest, then
uniform transport, then frozen-partner discrimination. A constructive mobility
verdict licenses only the selected three-record core under external periodic
neutralization. It does not derive the constituent phase space, chart fibre,
charge, statistics, a production particle, a scenario, a pole, Lorentz
recovery, or unitarity.
