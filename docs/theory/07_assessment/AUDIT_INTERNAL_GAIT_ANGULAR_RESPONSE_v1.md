# Audit — FTD-0617 internal-gait angular response

**Status:** `[AUDIT — MIXED-PARITY RESPONSE RESOLVED; BALANCED GAIT OPEN]`
**Verdict:** `MIXED_PARITY_INTERNAL_GAIT_RESPONSE_RESOLVED`

- protocol SHA-256: `3BBD3276...1065`;
- locked FTD-0616 parent SHA-256: `9EB7E10D...6B40`;
- runner SHA-256: `FD472437...6E05` (guard-only reuse change after the run);
- result JSON SHA-256: `DABFBE34...95C0`;
- certificate SHA-256: `ACCAC926...0F71`;
- independent checks: 19/19 pass;
- run of record: `engine/results/ftd_0617/`.

The registered eight-angle response circle, four proper-cubic controls, and
all 6,144 forward/inverse transactions pass. The certificate independently
reconstructs every DFT coefficient and displacement, the parity RMS values,
tickwise covariance, axis/diagonal selection, algebraic gates, and locked
verdict from the CSV/JSON records.

The result is not an isotropic velocity mode. Axis excitations translate about
`2.456` cells, whereas the predeclared diagonals translate only
`0.101...0.271` cell. Both parity sectors are material (`R_even=0.8596`,
`R_odd=1.5158`), and the sampled third odd harmonic slightly exceeds the first.
This is a finite eight-point lattice response; higher angular harmonics alias
modulo eight, so no continuum cubic coefficient is inferred.

The response remains externally neutralized and the pseudomomentum defect
reaches `1.215e-3`. The result licenses a preregistered symmetry-paired
six-constituent common-action test. It does not license vector addition of
independent runs, self-propulsion, an inertial particle, a pole, or an
electromagnetic interpretation.
