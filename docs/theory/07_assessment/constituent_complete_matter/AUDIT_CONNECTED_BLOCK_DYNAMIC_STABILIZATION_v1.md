# Audit — FTD-0625 connected-block dynamic stabilization

> **FTD-0626 successor correction:** the measured failure is scoped to the
> strict one-record-per-anchor chart. Its reaction-surface interpretation is
> withdrawn; the fibre-enabled successor is reversible.

**Status:** `[AUDIT — RIGID CIRCULATION CLOSED NEGATIVE / OPPOSITE-POLARITY
REACTION SURFACE CONFIRMED / STABLE MATTER OPEN]`  
**Date:** 2026-07-27

## Findings

1. **Rigid circulation is not a stabilizer.** Both registered energies, both
   signs, and both cyclic controls fail before the 16-tick horizon.

2. **The negative is not an amplitude-search artifact.** Amplitudes are fixed
   by `K(A)=B_x` and `K(A)=4B_x`; root residuals are below `1e-13`. No empirical
   value or post-run replacement is used.

3. **The negative is not a solver failure.** Every failed endpoint solve
   converges near `1e-14`; the graph and common-action equations remain well
   defined. Unique ternary site projection alone fails.

4. **All collisions are reaction-like.** Every failed arm contains two and
   only two opposite-polarity conflicts. There are zero same-polarity pairs.

5. **More circulation worsens the registered lifetime.** One-barrier arms fail
   at tick two; four-barrier arms fail at tick one. Higher rigid rotational
   energy is not a centrifugal repair.

6. **The static control distinguishes the intervention.** Zero circulation
   remains exact, coherent, conflict free, and state-only reversible for 16
   ticks. The circulation itself opens the earlier reaction route.

7. **No stable-particle conclusion follows.** The result closes one selected
   rigid-motion family. It does not exclude non-rigid topology, a reaction
   transaction, or a newly priced species/phase fibre.

## Correct statement

For the frozen connected neutral block, model-internal zero-total-momentum
rigid circulation at one or four Peierls-barrier energies accelerates
opposite-polarity ternary occupancy conflicts and fails as a short-horizon
dynamic stabilizer. The next native-first calculation is an atomic reaction
transaction, not a larger unregistered circulation amplitude.

## Verification

- Protocol SHA-256: `E95F2EB5...751CA`.
- Result: `RIGID_CIRCULATION_DYNAMIC_STABILIZATION_CLOSED_NEGATIVE`.
- Run-record hashes: JSON `99F6337B...A4C02`, arms
  `3BC30ADA...C5DF4`, ticks `DD77EC9C...981DB`.
- Independent certificate: `37/37` checks pass.
- Production tick, defaults, toggles, CUDA, WASM, and scenarios are unchanged.
