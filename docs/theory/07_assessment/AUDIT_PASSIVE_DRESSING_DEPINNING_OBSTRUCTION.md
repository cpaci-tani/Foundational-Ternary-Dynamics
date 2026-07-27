# Audit — Passive Dressing Depinning Obstruction (FTD-0581)

**Date:** 2026-07-26  
**Verdict:**
`PASSIVE_DRESSING_CANNOT_DEPIN_ACTIVE_TRAVERSAL_COSTS_FINITE_EXCITATION`

## Findings

1. **The chord action has a finite production-kinematic threshold.** Solving
   the production dispersion exactly gives `p_dep=0.00830..0.01214` and
   `v_dep=0.01624..0.02373` over the registered arms.

2. **The threshold is not a numerical artifact of a force integrator.** It is
   the exact energy inequality `K(p)>=C_d/4` derived before any trajectory
   update is selected.

3. **The relaxed field dressing already defines the Peierls curve.** The
   quadratic Hodge field plus linear source completes to that minimum plus a
   nonnegative field-deformation norm.

4. **Passive deformation cannot lower the barrier.** A lagged, distorted, or
   otherwise nonstationary field lies above the relaxed curve at the same
   subcell coordinate.

5. **The integer-site minimum has a linear cusp.** The periodic curve has
   one-sided slopes `-C_d,+C_d`; a locally Lipschitz perturbation of a stable
   quadratic equilibrium begins at `O(r^2)` and cannot cancel it.

6. **The obstruction survives zero modes.** A positive-semidefinite zero
   direction can leave energy unchanged but cannot provide a negative
   `-C_d|r|` term.

7. **Zero-momentum active traversal costs finite excitation.** Any internal
   mode must start with at least `epsilon_0=C_d/4`; across the registered arms
   that is `6.74048e-5..1.44093e-4` in engine energy units.

8. **The equality-budget path is not smooth through the saddle.** Its
   positive oscillator coordinate is proportional to `|r-1/2|`, so its two
   derivatives disagree. Larger budgets are smooth but remain finitely
   excited at the lattice sites.

9. **An energy budget is not a mobile solution.** No native coupling or
   recurrent phase transfer was derived. The active `(J,W)` traversal branch
   remains open and must be tested dynamically.

   **FTD-0582 successor:** the frozen tick has no native field-to-momentum
   write path with selected forces disabled; this active branch is closed for
   production and survives only as a possible new selected extension.

10. **The surviving candidate is an internally clocked object, not a passive
    aura.** It must carry a finite phase-resolved field excitation at vanishing
    external momentum and return that excitation after each hop.

11. **All registered algebraic arms close.** The observer covers 104 threshold
    arms, 416 passive fixtures, 3,744 passive samples, 2,808 active-budget
    samples, and 24 proper rotations. The largest inverse-momentum residual is
    `7.26e-16`.

12. **Production remains unchanged.** No action, source, force, movement
    phase, toggle, default, scenario, renderer, or primitive state was edited.

## Reproducibility

- theorem:
  `docs/theory/10_eft_program/derivations/THEOREM_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION.md`
- preregistration:
  `docs/theory/10_eft_program/preregistrations/PREREG_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION_v1.md`
- preregistration SHA-256:
  `CB525DEF5A5E6B92127C4DFD9C72DCF1F7799E7D97113519EDF2C732E56B0DDC`
- native observer: `test_passive_dressing_depinning_obstruction`
- independent exact proof:
  `scripts/proofs/proof_passive_dressing_depinning_obstruction.py`
- run record: `engine/results/ftd_0581/windows_msvc_cpu.json`
- production changed: no
