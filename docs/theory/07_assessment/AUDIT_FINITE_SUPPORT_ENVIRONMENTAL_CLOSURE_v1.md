# Audit — finite-support environmental closure v1

**Identifier:** FTD-0745  
**Status:** `[AUDIT PASS — REGISTERED M2 LADDER CLOSED NEGATIVE AT E5]`  
**Verdict:** `ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL`  
**Date:** 2026-07-29  
**Production status:** unchanged

## 1. Audit verdict

The FTD-0745 held-out run is transaction-valid and negative under its locked
verdict order. The independent certificate reproduces E0--E6 and the E5
failure from the frozen records rather than trusting the engine label.

The result closes this six-shell M2 candidate negative. It does not close the
broader environmental-closure question and does not negate the FTD-0739 M1
formation witness.

## 2. Execution integrity

- locked protocol SHA-256: `D5FB9923…456888`;
- corrected pre-execution static conformance: `63/63 PASS`;
- focused batched observer CTest: `1/1 PASS`;
- frozen runner: `7F2205D6…28776E`;
- frozen Release executable: `B140CE30…A6688`;
- complete histories: `5/5`;
- complete forward rows: `925/925`;
- independent record certificate: `131/131 PASS`;
- proof SHA-256: `C1256E51…F35B`;
- CSV SHA-256: `58D85CB5…6C41C`;
- raw summary SHA-256: `B6325EFB…1DC2A`.

The launching shell timed out while the child remained active. The original
child completed and wrote both records; it was not restarted. The earlier
pre-result implementation abort is separately disclosed in
[`AUDIT_FINITE_SUPPORT_ENVIRONMENTAL_CLOSURE_PREEXEC_v1.md`](AUDIT_FINITE_SUPPORT_ENVIRONMENTAL_CLOSURE_PREEXEC_v1.md).

## 3. Gate reconstruction

| gate | result | audit statement |
|---|---|---|
| E0 exact transaction | pass | every forward/reverse solve, exactness, source-support, energy, recoil, speed, and inverse condition passes |
| E1 causal prefix | pass | discrete prefix exact; maximum scalar difference `2.200e-14` |
| E2 control/polarity | pass | bound control stable; body conjugates identical |
| E3 longer core | pass | all four unbound histories have at least 64 terminal negative ticks |
| E4 near field | pass | all late radius-eight minima exceed `1.494e-3` |
| E5 shell arrival | **fail** | radii 32 and 48 never exceed `1e-8` by tick 184 |
| E6 no return | pass where defined | every threshold-crossed shell has no registered inward increment |

The verdict must stop at E5. The radius-32 maxima lie between
`7.905e-9` and `7.983e-9`; radius 48 remains at the numerical floor. The
registered `1e-8` threshold is not relaxed.

## 4. Serialization defect disposition

The raw summary is not strict JSON because one unused bound-control
`late_inside_8_minimum` sentinel is written as bare `inf`. The audit does not
hide or rewrite it. The certificate hashes the raw file, requires exactly one
such token in the bound-only slot, maps it to `null` in memory, and independently
reconstructs every physics gate.

E0 lists transaction and support conditions and does not make this unused
bound near-field slot a physics gate. The audit therefore retains the E5
physics verdict while recording the output-schema defect as mandatory repair
for any successor runner. Strict JSON consumers must not consume the raw
summary directly.

## 5. Claim boundary

The strongest licensed positive statement is:

> Across the frozen `L=145` prefix and held-out `L=193` continuation, every
> unbound selected-action history retains a negative relational core and a
> noncollapsing localized near field while source-free energy moves outward
> monotonically through radius 24 before periodic contact.

The audit rejects promotion to:

- complete finite-ladder environmental closure;
- survival after outer-field arrival or environmental return;
- radiation, wake, aura, pilot-wave, or photon identification;
- an invariant/metastable family or physical particle;
- native reduction, conserved charge, mass, spin, statistics, or Lorentz
  recovery.

## 6. Roadmap consequence

M1 remains constructive at the selected scope. FTD-0745's M2 implementation is
closed negative at E5, while the broader M2 question remains unresolved. M3
does not advance.

The next admissible campaign must match volume and horizon to a preregistered
outer-shell arrival prediction and then test post-arrival persistence. It must
not rerun FTD-0745 with a lower threshold or drop radius 48 after reading this
result.

