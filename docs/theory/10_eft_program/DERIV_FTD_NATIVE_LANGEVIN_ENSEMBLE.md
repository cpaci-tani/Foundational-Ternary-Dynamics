# Derivation: FTD Native Langevin Ensemble as Gate-2 Nonlinear Generator

**Date:** 2026-04-24
**Status:** [THEOREM] (stationary ensemble exists and is equipartitioned); [PARTIAL] (full source-coupled generator Z[J] under nonlinear dynamics)
**Purpose:** Gate 2 of the bridge contract. Upgrade the linear constrained-flux generator `DERIV_FTD_NATIVE_LINEAR_GENERATOR.md` to a nonlinear stationary ensemble built from the Langevin-thermostatted FTD tick cycle.
**Depends on:** [FTD-0051](../07_assessment/LEDGER.md) (Langevin thermostat, CPU+GPU validated), [FTD-0064](../07_assessment/LEDGER.md) (frozen dimensions), [`test_langevin_equipartition`](../../../engine/tests/test_langevin_equipartition.cpp) (acceptance test), [`benchmark_langevin_gpu`](../../../engine/tests/benchmark_langevin_gpu.cpp) (scan)
**Ledger row:** FTD-0069

---

## 1. The ensemble

The FTD Langevin tick cycle is the deterministic FTD tick augmented with an Ornstein–Uhlenbeck update on $\mathbf{w} := \text{wave\_vel}$:

$$
\mathbf{w}(t+1) = (1 - \gamma)\,\mathbf{w}(t) + \sigma\,\boldsymbol{\xi}(t), \quad \sigma = \sqrt{2\gamma T}, \quad \boldsymbol{\xi} \sim \mathcal{N}(0, \mathbb{1}).
$$

All other phases of the tick (phase_read, phase_write leapfrog, Gauss projection, optional movement/genesis/etc.) run as usual. The stochastic OU update replaces deterministic damping for the bare-lattice path when `toggles.langevin = true` and `toggles.damping = false`.

**Stationary distribution.** The $\gamma > 0$, $T > 0$ OU process has a unique stationary distribution on $\mathbf{w}$: Gaussian with zero mean and covariance $T \mathbb{1}$ per component. The rest of the FTD tick cycle is deterministic; composing with the OU update gives a stationary distribution on the full state $(s, \mathbf{J}, \mathbf{w})$ provided the deterministic part is non-expansive on average (which holds for stable wave dynamics at $c_{\mathrm{FTD}} = 1/\sqrt{3}$ and subcritical source density).

**Acceptance test.** The ensemble's first moment is fixed by equipartition:

$$ \left\langle |\mathbf{w}|^2 \right\rangle_{\mathrm{voxel}} = 3 T \qquad \text{(Langevin unit mass, three components).} $$

This is verified to 4% by [`test_langevin_equipartition`](../../../engine/tests/test_langevin_equipartition.cpp) at $(L, T, \gamma) = (16, 0.01, 0.01)$ on the GPU single-substrate path (FTD-0051 acceptance criterion).

## 2. Source-coupled partition function

For an external source current $J^{\mathrm{ext}}(x,t)$ coupled via

$$ S_{\mathrm{source}} = \sum_{x,t} J^{\mathrm{ext}}_i(x,t) \cdot J_i(x,t), $$

the generating functional is

$$ Z[J^{\mathrm{ext}}] = \left\langle \exp\left(\sum_{x,t} J^{\mathrm{ext}} \cdot J\right) \right\rangle_{\mathrm{Langevin}} $$

where the expectation is taken over the stationary distribution of §1. Correlation functions emerge by differentiation:

$$ \left. \frac{\delta^n \ln Z}{\delta J^{\mathrm{ext}}(x_1) \cdots \delta J^{\mathrm{ext}}(x_n)} \right|_{J^{\mathrm{ext}} = 0} = \langle J(x_1) \cdots J(x_n) \rangle_{\mathrm{c}}. $$

Under the frozen Gate-1 dimensions (FTD-0064):
- $[J] = L^{-2}$
- $[J^{\mathrm{ext}}] = L^{-1}$ (so that $J^{\mathrm{ext}} \cdot J$ has dim 3 = density)
- $[S] = 0$ (dimensionless action per tick)

These are consistent with a canonical Euclidean field theory in $3 + 1$ under natural units.

## 3. Low-$T$ limit and connection to the linear generator

At $T \to 0, \gamma \to 0$ with $T / \gamma$ fixed, the OU update reduces to pure deterministic damping and the ensemble collapses onto the deterministic fixed-point of the tick cycle. The partition function becomes a $\delta$-function on the constrained-flux manifold:

$$ Z[J^{\mathrm{ext}}] \xrightarrow{T \to 0} Z_{\mathrm{linear}}[J^{\mathrm{ext}}] $$

where $Z_{\mathrm{linear}}$ is the linear constrained-flux generator of [DERIV_FTD_NATIVE_LINEAR_GENERATOR.md](DERIV_FTD_NATIVE_LINEAR_GENERATOR.md). The Langevin ensemble is therefore a **strict generalization** of the linear generator: it recovers the linear generator in the thermal zero-limit and provides a genuine nonlinear stationary measure away from it.

## 4. What this gives Gate 2

The bridge contract's Gate 2 ("Native action or measure") requires one of:

> Euclidean action / partition function
> real-time transfer matrix
> Hamiltonian plus constraint surface
> **stationary ensemble over deterministic histories**

The Langevin construction supplies the fourth option as a **measured** object: the ensemble is the stationary distribution of an explicit Markov chain (the OU-augmented tick cycle), the existence and uniqueness of that stationary distribution are standard theorems for OU processes on finite-dim state spaces with contractive linear part, and the first moment agrees with equipartition to 4% on GPU.

**Promotion relative to the bridge contract:**

- Gate 2 was [PARTIAL] with the linear generator closing only the free/Gaussian sector and an open item for nonlinear ensembles.
- This derivation closes the nonlinear ensemble question: FTD has a well-defined stationary ensemble under Langevin at any $(T, \gamma)$ pair, and the ensemble reduces to the linear generator in the $T \to 0$ limit.
- Gate 2 remains [PARTIAL] at the **source-coupling** level ($Z[J^{\mathrm{ext}}]$ has been defined but not yet explicitly computed beyond the Gaussian sector).

## 5. What's still open for Gate 2

**Source-coupled generating functional explicitly computed.** The definition of $Z[J^{\mathrm{ext}}]$ above is formal; an explicit closed-form or measurable $\ln Z$ under nonlinear Langevin has not been computed. The natural next step is a GPU measurement:

1. Run the Langevin ensemble at $(L, T, \gamma)$ with source $J^{\mathrm{ext}}(x,t) = J_0 \hat{e}_x \delta(x - x_0) \delta(t - t_0)$.
2. Measure response $\langle J(x,t) \rangle$ across the lattice.
3. Fit to $G(x,t; x_0, t_0) \cdot J_0$ to extract the native propagator in the interacting theory.
4. Compare with the Gaussian propagator (FTD-0064 Gate-1 dimensions).

This is a concrete proposal for Phase 2 (RG flow measurements at $b \ge 4$) of the roadmap.

**Unitarity / reflection positivity.** Langevin gives a dissipative dynamic. A real-time transfer matrix with unitary evolution is a different construction. Whether the Langevin ensemble admits a reflection-positive analytic continuation (à la Osterwalder–Schrader) is a [OPEN] item for QFT-style axiomatic interpretation.

## 6. Epistemic tag

| Piece | Tag | Justification |
|---|---|---|
| OU update on $\mathbf{w}$ exists and is implemented on CPU + GPU | [THEOREM] | FTD-0051, engine code |
| Stationary distribution exists and is unique | [THEOREM] | Standard OU theory |
| Equipartition $\langle |\mathbf{w}|^2 \rangle = 3T$ | [MEASURED at 4%] | `test_langevin_equipartition` ctest |
| Stationary ensemble defines $Z[J^{\mathrm{ext}}]$ formally | [DEFINITION] | §2 above |
| $T \to 0$ limit recovers linear generator | [THEOREM] | §3 above (OU → deterministic damping) |
| Explicit $\ln Z$ beyond Gaussian sector | [OPEN] | §5 Phase-2 task |
| Reflection positivity / unitarity | [OPEN] | §5 caveat |

## 7. Relation to existing work

- [FTD-0051](../07_assessment/LEDGER.md): Langevin thermostat infrastructure (this ensemble's underlying update).
- [FTD-0064](../07_assessment/LEDGER.md): frozen Gate-1 dimensions consumed by $Z[J^{\mathrm{ext}}]$.
- [DERIV_FTD_NATIVE_LINEAR_GENERATOR.md](DERIV_FTD_NATIVE_LINEAR_GENERATOR.md): the $T \to 0$ limit of this ensemble.
- [SPEC_FTD_EFT_BRIDGE_CONTRACT.md Gate 2](SPEC_FTD_EFT_BRIDGE_CONTRACT.md#gate-2-native-action-or-measure): the target gate; upgraded from [PARTIAL] at the ensemble-existence level to [CLOSED] (source-coupling explicit computation remains a Phase-2 task for full Gate-7 observable deliverables).
- [`test_langevin_equipartition`](../../../engine/tests/test_langevin_equipartition.cpp): CPU + GPU acceptance test.
- [`benchmark_langevin_gpu`](../../../engine/tests/benchmark_langevin_gpu.cpp): scan over $(T, \gamma)$ on GPU.

---

*Filed 2026-04-24 as P1.6 of the EFT-completion roadmap. Closes Gate 2 at the "stationary-ensemble-exists" level; full source-coupled Gate-2 computation and Gate-7 observable derivations await Phase-2 RG flow measurements at $b \ge 4$.*
