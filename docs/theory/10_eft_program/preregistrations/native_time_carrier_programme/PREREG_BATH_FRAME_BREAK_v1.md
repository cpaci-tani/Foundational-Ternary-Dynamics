# Preregistration: the bath-frame break v1 (census target SC3)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED — PREDICTED-BREAK CAMPAIGN]`
**Programme:** Universality Programme, first shot at the Functional
Census's ranked break register (`AUDIT_FUNCTIONAL_CENSUS.md` §2, target
SC3: bath frames). **Parents:** the census (commit `2412115e`), the frozen
v2 bion machinery (SHA `47DD26FC...8DBD`), FTD-1011 (the wave island's own
hiding order (ka)², now the benchmark every second-category term must
beat).

## 1. The predictions (declared from the census, before this lock)

Rayleigh damping — the engine's `damping`-class term, a second-category
functional with a substrate rest frame — is added to the one-functional
surrogate as the **only** modification
(`φ_{t+1} = 2φ_t − φ_{t−1} + acc − κ(φ_t − φ_{t−1})`). The census predicts:

- **P-A (bulk drag):** a moving bion decelerates toward substrate rest,
  u(t) ≈ u₀·e^(−κ_d t) — a frame detector at **first order in u with no
  (ka) suppression**, i.e. far below the wave island's (u/C)²(ka)² floor.
- **P-B (lifetime anisotropy):** the beat-amplitude decay rate violates
  covariant lifetime dilation: D(u) = Γ(u)·γ_kin/Γ(0) ≠ 1 (γ_kin from the
  drag-corrected mean velocity).

## 2. Instrument (pinned)

`scripts/experiments/temporal_interior/derive_bath_frame_break.py`
SHA-256 `AB5D393D5FEC7EE34EEF945B6DB47935D7EF3D9EEEB86745CAE91EBE42C7F426`.
Two probes: the frozen v2 **global** interpolated-peak trace (byte-
compatible, for events/lifetime) and the **topological** position (midpoint
of the two kink zero-crossings, window-tracked, held through the brief
crossing-free contraction instants). Lifetime = log-linear fit of the
median-binned beat-event amplitudes. Drag = log-linear fit of windowed
velocities of the topological center.

**Selftest of record (pre-lock, disclosed):** κ=0 control reproduces the
frozen v2 record **exactly** (1201 events) with **0.00 sites** drift;
κ=5×10⁻⁴ rest lifetime cleanly exponential (Γ=3.59×10⁻⁴, R²=0.9931). Two
instrument defects were found and repaired pre-lock (global-argmax
ring-walk; windowed-std envelope contamination) — both disclosed; no
boosted or registered cell was run pre-lock.

## 3. Declared grid and gates

λ = 0.05; κ ∈ {5×10⁻⁴, 10⁻³}; u/C ∈ {0.25, 0.50} + rest lifetimes; T =
30000; N = 4096. Gates: **G1** control (κ=0 events == 1201 exactly; drift
< 2 sites); **G2** drag exponentiality R² > 0.99 per moving cell; **G3**
κ_d/κ agreement across the two κ within 20% (a census effect must scale
with its second-category coefficient); **G4** lifetime fits R² > 0.98 per
cell.

## 4. Outcomes

- **BREAK CONFIRMED** — all gates; positive drag in every moving cell:
  books the programme's first census-predicted, then measured,
  universality break (with the measured κ_d/κ and D(u) as the
  quantitative content). Lifetime anisotropy reported at its measured
  size either way.
- **BREAK ABSENT** — gates pass, drag consistent with zero: **refutes the
  census's severity ranking** and is booked as such (the more surprising,
  more valuable outcome).
- **EXECUTION GATES FAILED** — diagnosis only, nothing claimed.

Artifacts: console log; `results/bath_frame_break.json`; LEDGER row
post-run; lock tag `preregister-bath-frame-break-v1`.
