# Preregistration: the exact unit-strut tensegrity decision v1 (native C3, step 2)

**Date locked:** 2026-08-14
**Status before execution:** `[PREREGISTERED]`
**Programme position:** `SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md` §8 **step 2** — the
gate the spec records as **never run** (the FTD-0899–0999 chain proceeded to step 3
without it; the 2026-08-13 handoff in `temporal_interior_programme/INDEX.md` §6 names
this the arc's single largest transparency gap).
**Parents:** FTD-0789 (second-order-rigidity criterion), FTD-0800/0801 (clamped-only
screens), FTD-0804/0805 (MVC analysis; the [OPEN — native C3, third formulation]
question; the buckling criterion and the killed axial-lens family).

## 1. Question of record

Verbatim from `ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md` §3.1:

> **[OPEN — native C3, third formulation]** does a finite integer unit-strut
> tensegrity exist under the registered law (struts = single unit bonds carrying
> compression; straight integer-span tension chains; polarity, floor, and `q < 3/2`
> clearances), with the blocking form definite on its full flex space?

## 2. Sources of record (pinned)

| document | SHA-256 |
|---|---|
| `ANALYSIS_MINIMUM_VIABLE_CLOCK_CARRIER_v1.md` | `3CABBD8C34ACAF0FB598189F4103A06D6FF82C40D3B112B246E5E59970FE2706` |
| `SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md` | `2A9F33284088C869E3A78B203F82F0129CC1CE36F22A30DF1CD6D8F02F0C44F8` |
| `DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1.md` | `B712373544641828D96CFD4053CE9AC59E08E7AE076F4506DF4F239E14F3AD86` |

**Decision instrument:** `scripts/experiments/native_unit_strut_tensegrity_decision.py`
SHA-256 `C1EE2DBE0B323758AF21D21D707422564FBCF0EFA6D381C962ABECB96CC25961`.
The instrument is frozen at this hash before the registered run. Its `--selftest` mode
(known-answer calibration: recorded SC L=2 coker dim 0; fabricated-stress blocking
path; certified radical comparisons) was run before the lock and passed 3/3; no
decision cell was executed before the lock.

## 3. Declared scope

The realization class of the question is infinite. This campaign decides a **declared
closed scope** and claims nothing outside it:

- **F1 axial lens** — replication of the recorded exact kill (`s² − 4k² = −1`
  impossible mod 4). *Expectation declared: FAIL (replication).*
- **F3 planar unit wheel, span-1 spokes** — polarity 2-coloring infeasibility
  (odd cycle). *Expectation declared: FAIL.*
- **F4 single-ring axial cage** — exact case tree: (ring-bonded) alternating
  unit-bond ring vs the single-polarity cable constraint (`k+m` odd); (ring-unbonded)
  degree-2 tension joints force on-axis collinearity, then `k+m=1` infeasibility or
  beyond-end station coincidence. Arithmetic steps machine-checked; the two logical
  steps (z-equilibrium requires cables to both strut ends; two non-collinear tensions
  cannot balance) are stated here and carried by the doc, not the code.
  *Expectation declared: FAIL (derived by hand pre-lock, disclosed).*
- **F7 rectangle ladder** — two parallel unit struts, integer sides and diagonals:
  `d² − p² = 1` Pell factorization. *Expectation declared: FAIL (hand-derived
  pre-lock, disclosed).* The general (non-rectangular) two-strut quadrilateral is
  **not** decided by this campaign.
- **Arm C (partial)** — two-term axial closures `√(k²−1) ± √(m²−1) = 1` have no
  integer solutions (exact). Three-or-more-term axial closures remain **open**.
- **F5 prism family** — the live decision. `n ∈ {3,4}`, polygon chain span
  `a ∈ 1..6`, connector chain span `b ∈ 1..6`, wirings {diagonal-strut,
  vertical-strut} × chirality {+1,−1} (288 cells). Gate battery per cell, in order:
  1. **Polarity** — contracted-graph 2-coloring: `n·a` even and `1+a+b` even.
  2. **Closure** — exact symbolic solve of the embedding (`cos φ`, `sin φ`, `h²`)
     with `r = a / (2 sin(π/n))`; admissible iff `h² > 0` certified.
  3. **Clearance** — all sites (joints + chain stations), exact coordinates:
     same-polarity `q ≥ 4/25` (recorded floor r ≥ 0.40); opposite-polarity
     non-bonded `q ≥ 3/2` (the registered support edge).
  4. **Stress** — full rigidity matrix over the exact field; `coker(R)`; existence
     of a stress with compression **exactly** on struts, tension on every chain
     bond (stress dim ≤ 2 decided; dim > 2 → INCOMPLETE, disclosed).
  5. **Blocking** — the form `Σ ω_e |δq_u − δq_w|²` positive definite on the full
     nontrivial flex space: charpoly kernel dimension must equal the trivial part
     exactly, remaining coefficient signs strictly alternating (Descartes).
  *Expectation declared: unknown — this family has never been decided; it is the
  reason the campaign exists.*

**Certificate methods** (per §7's allowance "exact, symbolic, or interval
certificates"): integer/modular arithmetic and `diophantine` (exact); sympy symbolic
solves and sign decisions (exact); numerical comparison at 50 digits with margin
`10⁻²⁰` only where sympy's exact sign query is indeterminate, reported as interval
certificates. Any indeterminate within margin → the cell is **INCOMPLETE**, never
silently passed or failed. Per-cell stage budget 300 s; overruns → INCOMPLETE.

## 4. Hard rules

1. **No target-reading** (spec §8 step 2): the framework period constant is never
   imported, computed, or compared against. Check C02 lints the instrument's own
   source for the banned tokens. This campaign decides **native critical
   quarticity only**; any gearbox question comes later and separately.
2. The registered law enters only through scale-free facts verified in C03
   (`V′(1)=0`, `V″(1)=24ε>0`, support edge `q=3/2`); ε never enters (disclosure D1).
3. Checks C01–C12 are protocol-integrity gates: **all must pass or the execution is
   `[EXECUTION INVALID]`**. Family verdicts are data, not checks.
4. Early stop on the first F5 PASS is declared (a single candidate settles the
   existence question affirmatively).

## 5. Outcome taxonomy (declared before the run)

- **Outcome A** — some F5 cell passes all five gates: a native C3 candidate exists;
  the [OPEN] question is answered **YES within scope**. Next step (separate
  campaign): amplitude-decade and band questions; only then any period identity.
- **Outcome B** — every declared family closes negative with certificates: the
  question is **scope-negative**; the class stays open beyond scope (≥3-term axial
  closures, non-rectangular multi-strut quadrilaterals, larger spans/n, asymmetric
  wirings, non-prismatic topologies). Booked as a boundary-sharpening, not a no-go.
- **Outcome C** — INCOMPLETE cells remain: reported per-cell with reasons; the
  campaign is still valid for all completed cells.
- Any check C01–C12 failing → `[EXECUTION INVALID]`, nothing is booked.

## 6. Disclosures

- **D1** — ε-independence: every gate in this campaign is scale-free; the decision
  is identical for all ε > 0.
- **D2** — F4/F7 verdicts were hand-derived during pre-lock scoping (this session)
  and are declared expectations, not blind outcomes; the run converts them into
  machine-checked certificates. F5 was not analyzed pre-lock beyond the polarity
  parity conditions stated in §3.
- **D3** — This campaign covers a declared finite scope of an infinite class. A
  scope-negative outcome does **not** close native C3; §7 of the clock-minimum spec
  demands exact certificates over a declared class, which is exactly what this
  delivers — for this scope.
- **D4** — The blocking-form gate uses the criterion of record (spec §7 eq. 8–9).
  A PASS therefore means first-order flexible + second-order rigid under the
  registered law's sign structure: native critical quarticity, nothing more.

## 7. Artifacts

- Console log of the registered run (committed alongside the booking).
- `scripts/experiments/results/native_c3/unit_strut_tensegrity_decision.json`
  (checks + full verdict table + outcome).
- LEDGER row to be minted post-run from `scripts/audit/check_registry.py` (main
  only), tag history referencing this prereg and the git lock tag
  `preregister-unit-strut-tensegrity-decision-v1`.
