# Derivation: FTD Native Complete History Action

**Date:** 2026-04-26
**Status:** [SELECTION] canonical microscopic history action; [THEOREM] reduction
to the linear G18 generator; [OPEN] explicit nonlinear effective action after
blocking.
**Purpose:** Close the mathematical form of the native action/measure without
promoting it to a smooth continuum Lagrangian before the nonlinear flow data
exists.

---

## Executive result

The complete native FTD action is not first a local continuum Lagrangian. It is
the negative log of the engine's source-coupled history kernel:

```text
Z_u[eta, h, a, lambda_R] =
  sum_H exp(
      -S_H[H; u]
      + sum_t,x eta rho
      + sum_t,x h . J_T
      + sum_t,F a . I
      + sum_t,x lambda_R S_R
  )
```

where `u` is the declared toggle/backend/closure contract and a history is

```text
H = {
  q_t = (s_t, J_t, w_t, particle data)_t,
  l_t = (I_t, S_R,t, channel_t, energy_t)_t
}
```

with:

```text
rho_t = s_t
J_t = J_L[rho_t] + J_T,t
w_t = wave_vel_t, the canonical momentum-like flux velocity
I_t = oriented finite-volume transport current
S_R,t = non-transport reaction source
channel_t = genesis, evaporation, pair-production, movement, annihilation,
            weak-transmutation, or no-op bookkeeping
```

The microscopic history action is:

```text
S_H[H; u] =
    S_init[q_0]
  + sum_t S_tick[q_{t+1}, l_t | q_t; u]
  + sum_t S_constraint[q_t, l_t; u]
  + sum_t S_noise[q_{t+1}, l_t | q_t; u].
```

This is the exact native generator. The smooth EFT action is the blocked
effective action:

```text
exp(-S_eff[H']) = sum_{H: B_b H = H'} exp(-S_H[H]).
```

So:

```text
microscopic FTD action      = constrained history action
Gaussian EFT action         = quadratic tangent sector
nonlinear Wilsonian action  = blocked effective action, measured from histories
```

---

## 1. Configuration space

For a finite periodic lattice `L^3` and time interval `t = 0..T`, define the
native state:

```text
q_t = (s_t, J_t, w_t, p_t)
```

where:

```text
s_t(x) in {-1, 0, +1}
J_t(x) in R^3                  # or (J_L, J_R) in dual-substrate mode
w_t(x) in R^3                  # flux velocity / leapfrog momentum
p_t                            # manifested particle attributes when present
```

The ledger variables are part of the action, not afterthought diagnostics:

```text
l_t = (I_t, S_R,t, c_t, E_t)
```

where:

```text
I_t(F)       integrated signed transport through oriented faces
S_R,t(x)     local reaction source
c_t          discrete channel labels
E_t          energy-ledger record
```

The exact finite-volume identities are imposed on integrated variables:

```text
D_face Phi_t = Q_t
Delta_t Q_t + D_face I_t = S_R,t
```

with `Q_t` the integrated source in a cell and `Phi_t` the integrated boundary
flux. This keeps the action compatible with `SPEC_FTD_NATIVE_BLOCKING_MAP.md`.

Epistemic status:

```text
state alphabet                   [AXIOM]
rho = s                          [THEOREM] internal convention
face-current continuity form      [THEOREM] finite-volume identity
ledger variables in the action    [SELECTION] required for nonlinear EFT
```

---

## 2. Exact transfer kernel

The engine tick defines a one-step kernel:

```text
K_u(q_{t+1}, l_t | q_t).
```

For deterministic toggle sectors, this kernel is an indicator:

```text
K_u = 1    if the declared FTD tick maps q_t to q_{t+1}
            and produces ledger l_t
K_u = 0    otherwise.
```

Equivalently:

```text
S_tick[q_{t+1}, l_t | q_t; u] =
  0          if the tick is admissible
  +infinity  otherwise.
```

This is not a defect. For a discrete deterministic theory, a delta-function
transfer kernel is the exact action. A smooth Lagrangian is a later effective
description after coarse-graining.

The tick kernel factors by phase:

```text
K_u =
  K_read
  K_write
  K_gauss
  K_forces
  K_movement
  K_ledger
```

with the factor present only when the corresponding toggle is enabled. Backend
choice is not a physics term; it is part of the reproducibility contract.

Epistemic status:

```text
tick kernel as delta transfer map     [DEFINITION]
phase factorization                   [THEOREM] engine architecture
backend as reproducibility contract   [SELECTION]
```

---

## 3. Constraint action

The constraint part is:

```text
S_constraint =
    iota[D_face Phi_t - Q_t = 0]
  + iota[Delta_t Q_t + D_face I_t - S_R,t = 0]
  + iota[locality and propagation bounds]
  + iota[toggle dependency rules]
```

where:

```text
iota[P] = 0          if P is true
        = +infinity  if P is false.
```

The first term is the native Gauss domain. The second is the reaction/transport
continuity domain. The locality term enforces Moore-neighborhood finite
propagation and speed-limit contracts. Toggle dependency rules are included
because a history generated with an invalid toggle combination is not an
element of the declared theory.

This removes an old ambiguity: the ledger is not merely evidence after the
fact. For the nonlinear EFT, it is part of the admissibility structure.

---

## 4. Stochastic and dissipative sectors

When Langevin is disabled and all stochastic channels are disabled, the action
is purely constrained:

```text
S_H = S_init + sum_t iota[valid FTD tick].
```

When Langevin is enabled, the OU update on `w = wave_vel` contributes the
Onsager-Machlup one-step cost:

```text
S_Langevin =
  sum_t,x |w_{t+1}(x) - (1 - gamma) w_det,t+1(x)|^2 / (4 gamma T)
  + normalizer(gamma, T).
```

Here `w_det,t+1` is the value produced by the deterministic part of the tick
before the OU noise draw.

For discrete stochastic reaction channels, the contribution is the Bernoulli or
categorical negative log-likelihood:

```text
S_channel =
  - sum_t,x log p_c(c_t(x) | q_t, u),
```

with `p_c = 1` for deterministic channels. This covers evaporation-like
channels without pretending they are quadratic fields.

Epistemic status:

```text
OU cost for Langevin sector       [THEOREM] from implemented OU update
discrete channel log-likelihood   [DEFINITION]
explicit nonlinear p_c catalogue  [OPEN] needs channel-by-channel audit
```

---

## 5. Quadratic tangent sector

Around the no-reaction, neutral, low-amplitude sector, the constrained history
action reduces to the already-derived linear generator:

```text
Gamma_lin[rho, J_T, Pi_T] =
    1/2 sum_{k != 0} |rho(k)|^2 / sigma_18(k)
  + 1/2 sum_{k != 0} (
        |Pi_T(k)|^2
      + c_FTD^2 sigma_18(k) |J_T(k)|^2
    ).
```

with:

```text
D_18 . J_L = rho
D_18 . J_T = 0
c_FTD = 1 / sqrt(3)
```

This reduction is the consistency check that the history action is not a new
unanchored object. It contains the G18 constrained-flux generator as its
Gaussian tangent sector.

Therefore:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
g_sJ^FTD = 1
```

in the bare native scheme.

Epistemic status:

```text
reduction to Gamma_lin        [THEOREM] given the G18 constraint selection
G18 direct-response branch    [SELECTION]
native tuple values           mixed tags per DERIV_FTD_NATIVE_RESPONSE_TUPLE.md
```

---

## 6. Source insertions

The complete generator must support the observable map:

```text
eta(x,t) rho(x,t)             source/source response
h_i(x,t) J_T,i(x,t)           transverse flux response
a_i(F,t) I_i(F,t)             transport-current response
lambda_R(x,t) S_R(x,t)        reaction-sector response
zeta_a(x,t) O_a(x,t)          registered operator moments
```

Functional derivatives of `ln Z` define native observables:

```text
delta^2 ln Z / delta eta delta eta       -> rho-rho correlator
delta^2 ln Z / delta h delta h           -> transverse flux propagator
delta^2 ln Z / delta a delta a           -> current response / Z_j
delta^2 ln Z / delta lambda_R delta zeta -> reaction mixing
```

This is where the observable registry belongs: it declares which source term is
allowed, which domain it probes, and which epistemic tag the measured result
receives.

---

## 7. Blocking and effective action

Use the native finite-volume map:

```text
B_b: (rho, Phi, I, S_R, O_a) -> (rho', Phi', I', S_R', O'_a).
```

The Wilsonian effective action is defined by integrating out fine histories
consistent with a coarse history:

```text
exp(-S_eff[H']) =
  sum_{H : B_b H = H'} exp(-S_H[H]).
```

Equivalently:

```text
S_eff[H'] =
  -log sum_{H : B_b H = H'} exp(-S_H[H]).
```

The nonlinear operator mixing matrix is then:

```text
O'_a = M_ab(b) O_b + irrelevant corrections.
```

The Gaussian sector gives the identity flow already measured/theorized. The
full nonlinear matrix remains a measured object:

```text
M_ab^nonlinear(b)     [OPEN]
fixed point data      [OPEN]
continuum symmetry    [OPEN]
```

---

## 8. What this resolves

This document selects the complete native action object:

```text
FTD is generated by a constrained source-coupled history measure.
```

It resolves the false fork between:

```text
"We need a smooth Lagrangian now"
```

and:

```text
"The engine has only rules, no action"
```

The exact microscopic action is the transfer-measure action of the rules. The
smooth continuum action is the blocked effective object extracted from that
measure.

---

## 9. What remains open

This does not complete the full EFT by itself. It supplies the generator that
the full EFT must use.

Remaining work:

```text
1. catalogue every stochastic/discrete channel probability p_c
2. select the production Gauss representation
3. implement long-run GPU history reductions for source-coupled observables
4. measure the nonlinear operator mixing matrix M_ab(b)
5. fit S_eff through the registered operator basis
6. test scaling across L and b
7. only then discuss continuum or physical-QED matching
```

---

## 10. Verdict

The complete FTD action is:

```text
S_H = -log mu_0(q_0) - sum_t log K_u(q_{t+1}, l_t | q_t)
      - source insertions,
```

with exact Gauss, continuity, locality, and toggle-contract constraints inside
`K_u`.

In deterministic sectors this is a hard-constrained action. In Langevin and
other stochastic sectors it gains the corresponding noise/log-likelihood cost.
In the low-amplitude no-reaction sector it reduces to the G18 Gaussian
generator. Under finite-volume blocking it defines the nonlinear Wilsonian
effective action.

That is the mathematically complete native object. The remaining physics is to
measure and classify its blocked nonlinear operator content.
