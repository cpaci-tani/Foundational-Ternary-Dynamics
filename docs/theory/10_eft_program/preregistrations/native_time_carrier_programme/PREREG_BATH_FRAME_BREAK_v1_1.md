# Preregistration: the bath-frame break v1.1 (drag + arrest)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED — GATE-RESIZING REPAIR]`
**Parent:** `PREREG_BATH_FRAME_BREAK_v1.md` (lock `ceb7de30`, EXECUTION
GATES FAILED 4/14 — nothing claimed).

## 1. Diagnosis of record (what the v1 gates caught)

The v1 grid (κ ≥ 5×10⁻⁴) was sized for leisurely exponential drag; the
unlocked diagnosis found the phenomenon faster and stronger: velocity
decays at rate **κ exactly** (measured 4.8×10⁻⁴ at κ = 5×10⁻⁴, matching
the two-line momentum argument dP/dt = −κP), and the bion then **stops
dead** — position frozen to the digit from tick ~2000 — the
Peierls–Nabarro arrest: once drag brings the kinetic energy below the
lattice pinning barrier, the soliton locks to a site. Every boosted v1
cell had already arrested before the measurement window opened; the drag
and lifetime fits correctly failed on frozen data.

## 2. Declared phenomenon (amended predictions)

- **P-A drag:** κ_d ≈ κ in the early moving window (the momentum
  argument, now stated as the theoretical prediction).
- **P-B arrest:** terminal position freeze in every moving cell — the
  bath frame does not merely prefer its rest state; it **captures**
  moving bodies into it. (The strongest universality break the programme
  has measured or predicted: O(u) drag plus a terminal frame-restoring
  trap, versus the wave island's (u/C)²(ka)² leak floor of FTD-1011.)
- Lifetime anisotropy (v1's P-B) is **demoted to an ungated exploratory
  report** — arrest makes the moving-phase lifetime window κ-dependent
  and short. Disclosed design change, with reason.

## 3. Pins and design

| artifact | SHA-256 |
|---|---|
| instrument `derive_bath_frame_break_v1_1.py` | `6F287BDD05DA7A88F1E5524BC415CC8890A64AF7AE6874ADAF09272C9301D4E7` |
| frozen v1 machinery (evolve/env, imported) | `AB5D393D5FEC7EE34EEF945B6DB47935D7EF3D9EEEB86745CAE91EBE42C7F426` |
| frozen v2 physics cells | `47DD26FC52ACD1050FD44A411F3003F560E3A6FDEA8338330871CEE9EDB18DBD` |

Grid: κ ∈ {10⁻⁴, 2×10⁻⁴} (moving phase stretched across the run),
u/C ∈ {0.25, 0.50}, λ = 0.05, T = 30000. Drag fitted over
[500, 0.9/κ]; arrest = position range < 0.5 site over the final 5000
ticks. Physics evolution byte-identical to the locked v1 instrument
(imported); only measurement windows and the κ grid changed.

## 4. Gates and outcomes

G1 control (κ=0: exactly 1201 events; drift < 2 sites — inherited);
G2 early-window drag exponentiality R² > 0.99 per moving cell;
G3 κ_d/κ agreement across the two κ within 20% at each u;
G4 arrest in every moving cell. Outcomes: **BREAK CONFIRMED — drag at
rate κ + terminal arrest** (all gates; books the census's first
predicted-then-measured break with its quantitative content) /
**EXECUTION GATES FAILED** (diagnosis only). A drag-free or
arrest-free result would refute the census ranking and is booked as such.

Artifacts: console log; `results/bath_frame_break_v1_1.json`; LEDGER row
covering the v1+v1.1 chain; lock tag `preregister-bath-frame-break-v1-1`.
