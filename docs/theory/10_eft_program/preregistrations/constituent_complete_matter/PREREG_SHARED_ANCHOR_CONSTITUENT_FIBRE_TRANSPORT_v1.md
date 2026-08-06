# FTD-0609 — Shared-anchor constituent-fibre transport v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** default-off observer-only ontic-extension discriminator
**Production change:** forbidden
**Protocol lock:** `protocol_sha256=8CA3984F9E3FF2B8BE53BBBEA20028618EACFFC54C1B361994D10AD8B95D4D95`

## 1. Ontological candidate

Interpret a constituent's integer anchor as the coordinate-chart label for
its continuous effective position, not as an assertion that the constituent
exhausts the ternary site's only state slot. Permit at most two distinct
constituent records to share one anchor during the FTD-0608 transport arms.

The primitive site field remains exactly `s in {-1,0,+1}`. Multiple
constituents are not encoded by making `s` multi-valued; their signed compact
coats superpose into the already derived coupling density and face current.
This is an explicit local fibre over the site chart inside the already
selected constituent phase space. It is not derived from the five postulates
and is not adopted into production.

## 2. Frozen equations and only permitted change

Use the exact FTD-0608 phase-15 launch state, 24-start static reproduction,
minimum-energy electric field, zero magnetic half-field, two velocities,
tick counts, common-action equations, dispersion, binding, field update,
current deposition, energy gates, and state-only inverse.

The only permitted algorithmic change is an option named
`allow_shared_anchor_chart`, default `false`, in the observer transaction.
When `true`, solver candidates are not rejected solely because two distinct
constituent records have the same integer anchor. All existing default-false
tests must remain bitwise/numerically unchanged. No force, energy, current,
field, state variable, solver tolerance, or initial state may change.

## 3. Fibre regularity and transport gates

Repeat the `v=1/64` for 128 ticks and `v=1/32` for 64 ticks arms, followed by
the same-number state-only inverses. Require:

- every forward and reverse solve to be valid and common-action qualified;
- every common-action residual at most `1e-12`;
- maximum same-anchor multiplicity at most two;
- at least one state with multiplicity exactly two, proving that the new
  fibre is exercised;
- minimum effective-position distance between any two constituent records
  at least `1e-3`;
- each trimer's internal distances remain in `[0.5,2.0]`;
- trimer-centre separation changes by at most `0.25`;
- longitudinal displacement at least 75% of the nominal two cells,
  transverse drift at most `0.25`, and at least six anchor changes;
- total-energy drift at most `1e-10`;
- state-only reverse recovery at most `1e-9`.

Repeat the first step after an integer `x` translation and require covariance
at `1e-12`. Re-run the default-false FTD-0608 test and its certificate after
the implementation; its failure ticks, strict-site diagnosis, and verdict
must remain unchanged.

## 4. Verdicts

- `SHARED_ANCHOR_FIBRE_COMPACT_MATTER_MOBILE_CONSTRUCTIVE`: static
  reproduction, both complete transport/inverse arms, fibre exercise, and
  covariance pass;
- `SHARED_ANCHOR_FIBRE_TRANSPORT_CLOSED_NEGATIVE`: all solver coverage is
  complete but at least one registered physical or inverse gate fails;
- `SHARED_ANCHOR_FIBRE_EXTENSION_NOT_EXERCISED`: both arms otherwise pass but
  no shared-anchor state occurs;
- `SHARED_ANCHOR_FIBRE_NUMERICALLY_UNRESOLVED`: static, solver, record, or
  default-false regression coverage is insufficient.

A constructive verdict establishes only that this explicitly priced
two-record chart fibre is sufficient for the selected compact family and two
velocities. It does not derive the fibre, a physical particle, exclusion
statistics, a production type, a scenario, electromagnetic `U(1)`, a pole,
Lorentz recovery, or unitarity.
