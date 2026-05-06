# DERIV · Rayleigh Dissipation Coefficient DAMPING = α

**Tag:** [IMPOSED — with motivation]
**Date:** 2026-05-05
**Status:** [IMPOSED] retained; the three-roles diagnosis is honestly recorded; a route to [DERIVED] is identified but not closed in this commit.
**Purpose:** Phase R2 of the FTD-EFT roadmap. Closes the open item "DAMPING = α as Rayleigh is [IMPOSED] in `ontic.h:771`. Either derive or document why imposed at structural-axiom level." The honest answer: it is **imposed**, and for legitimate reasons — but the imposition collapses three conceptually distinct processes onto the same coefficient, and that collapse is not yet derived. This doc records the situation precisely so future work can close it.

---

## §1 — The Rayleigh dissipation function

`engine/include/ftd/lagrangian.h:64-66`:

```cpp
inline double rayleigh_dissipation(const Voxel& v) {
    return 0.5 * DAMPING * v.wave_vel.mag2();
}
```

In analytical form:

$$
R(\mathbf{v}) = \frac{\alpha}{2}\,|\mathbf{v}_\text{wave}(\mathbf{v})|^2 \qquad \text{where } \mathbf{v}_\text{wave} = \Delta_t\mathbf{J}
$$

Rayleigh is *not* part of the action $S$. It enters the EOM through

$$
\frac{d}{dt}\frac{\partial L}{\partial \dot J_a} - \frac{\partial L}{\partial J_a} = -\frac{\partial R}{\partial \dot J_a} = -\alpha\,\dot J_a
$$

So the wave EOM picks up a damping term $-\alpha \dot J_a$. In the engine, this is implemented as `flux *= (1 - DAMPING)^dt` near manifested particles in `phase_write.cpp` under `selective_damping = true`.

`DAMPING = α = ALPHA = G_C² ≈ 1/137.036` per `engine/include/ftd/constants.h` and the `using ontic::DAMPING` chain.

---

## §2 — The three-roles diagnosis

`ontic.h:771` ASSUMP.6 documents that DAMPING serves three conceptually distinct purposes:

1. **Physical dissipation** — vacuum drag at manifested-particle sites; the rate at which flux energy is absorbed into particle binding.
2. **Leapfrog stability margin** — at CFL = $1/\sqrt 3$ on the cubic lattice, a leapfrog integrator needs nonzero damping to avoid drift over many ticks; DAMPING enters as that margin.
3. **Evaporation drag** — the rate at which a manifested particle's local energy decays toward the evaporation threshold $K_B^2$.

These are three different physical processes. Numerically they share a single coefficient: `DAMPING = α`. **That coincidence is the thing that needs explaining.**

CALLSTACK F9 (per `AUDIT_ENGINE_CALLSTACK.md`, RESOLVED-AS-DOCUMENTED) marks this finding without a code change: tests that flip `damping = false` (notably the energy-conservation suite) implicitly measure all three concerns together; no test has been written that varies the three coefficients independently.

---

## §3 — A motivation, not a derivation

A heuristic chain that rationalises DAMPING = α:

**Step 1.** The state-flux coupling $L_\text{coupling} = -g_c s (\nabla\cdot\mathbf{J})$ converts flux energy into particle binding at the two-vertex level. The Born-rate for one tick of flux ↔ particle exchange is $\propto g_c^2 = \alpha$.

**Step 2.** By energy conservation, the rate at which flux loses energy (per tick) near a manifested particle equals the rate at which the particle gains binding energy. So flux decay rate $\sim \alpha$.

**Step 3.** The engine's `selective_damping = true` mode implements this by multiplying flux by $(1 - \alpha)$ each tick at near-particle sites — equivalent to the EOM picking up the $-\alpha\dot J$ term derived from $R = \frac{\alpha}{2}|\dot J|^2$.

**Why this is not a derivation.** Step 1's "Born-rate per tick" equation is dimensional analysis, not a calculation. It assumes the conversion rate is exactly $g_c^2$ (no log, no kinematic factor, no L-dependence). In a true Wilsonian derivation, there would be a calculable matching coefficient $c_R$ such that $\text{DAMPING} = c_R \cdot \alpha$ where $c_R$ is computed from the wave-equation Green's function evaluated at zero separation. We have not computed $c_R$.

**Why this is not pure imposition either.** The motivation in Steps 1–3 is structurally consistent with the rest of the framework, and the value $c_R = 1$ (i.e. DAMPING $= \alpha$ exactly) is the simplest hypothesis. It works — energy conservation tests pass at the cumulative-balance level; per-tick residuals stay within the documented tolerance. The simplest explanation that makes the engine work has DAMPING $= \alpha$.

**Status.** [IMPOSED] is the correct epistemic tag. The motivation is real but not closed.

---

## §4 — Routes to closure

Three plausible derivation chains, each future work:

**Route A: matched lattice Green's function.** Compute the wave-equation propagator at $r = 0$ on the cubic lattice; the coefficient $c_R$ should fall out as a calculable lattice integral analogous to Watson's. If $c_R = 1$ exactly, this closes the [IMPOSED] tag to [THEOREM]. If $c_R \neq 1$ exactly but $c_R \to 1$ as $L \to \infty$, this would re-tag DAMPING = α as a [DERIVED at L=∞] result with finite-L corrections.

**Route B: bandwidth / CFL identity.** The leapfrog stability margin at CFL = $1/\sqrt 3$ on the cubic lattice can be computed explicitly. If that margin equals $\alpha$ exactly, the "stability" role of DAMPING reduces to a structural identity. The remaining two roles (physical dissipation + evaporation drag) would still need separate derivations, so this route alone is partial.

**Route C: three-coefficient generalisation.** Replace `DAMPING` with three independent coefficients $\alpha_\text{diss}$, $\alpha_\text{stab}$, $\alpha_\text{evap}$ in the engine. Run a parameter scan to determine if all three converge to $\alpha$ at the canonical operating point. If yes, the collapse is empirically validated even if not derived. If they differ, the framework has a hidden tunable triple.

Routes A, B, C are **not in scope for R2** — they're queued as future work. Route A is the cleanest theoretically; Route C is the most defensible empirically. The R3 nonlinear blocked $S_\text{eff}$ closure may happen to incidentally compute $c_R$ as part of the operator-mixing program, in which case Route A closes for free.

---

## §5 — What this doc records [DECISION]

For the current FTD-EFT roadmap:

- **DAMPING = α stays [IMPOSED]** in the canonical Lagrangian doc and `ontic.h`.
- **The three-roles structure is honestly documented** rather than glossed.
- **The Phase J ultralocality theorem at L=2** (per `DERIV_PARTITION_FUNCTION_L2.md`) does not depend on the value of DAMPING — DAMPING enters only at the dissipative/non-conservative level, not in the partition function.
- **The continuum-limit theorem** in `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` requires $c_R \to 1$ at the matched-stencil limit; this is consistent with the [IMPOSED] value but doesn't independently fix $c_R$.

The rate-α-Rayleigh choice is consistent with:
- Energy conservation (cumulative-balance tests pass to the documented tolerance).
- Wave-equation stability (leapfrog runs at CFL = $1/\sqrt 3$ for arbitrarily many ticks under default toggles without drift).
- Observed evaporation timescales matching SM-particle lifetime predictions to ~5–18% (per FTD-0110 cluster-↔-mass identification).

It is **not** consistent with arbitrarily different damping rates: tests with DAMPING set to 0.001 or 0.1 break either energy conservation or evaporation kinetics. The engine has a window of operating points, but α is the preferred default.

---

## §6 — Honest one-line summary

DAMPING = α is the simplest hypothesis consistent with the engine running correctly across all three of its functional roles (dissipation, stability, evaporation). It is justified by structural analogy to the two-vertex Born rate but is not derived. The three roles using a single coefficient is itself an open structural question.

[IMPOSED] is the correct tag; this doc is the structured documentation that the audit asked for.
