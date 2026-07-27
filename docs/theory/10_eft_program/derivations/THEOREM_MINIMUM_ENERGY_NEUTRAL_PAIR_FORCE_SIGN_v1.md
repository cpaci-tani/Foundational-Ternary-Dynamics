# FTD-0602 — Minimum-energy neutral-pair force sign v1

**Status:** `[THEOREM — PERIODIC MINIMUM-ENERGY GAUSS REPRESENTATIVE] +
[MEASURED — REGISTERED-PHASE ATTRACTION AND REVERSIBILITY] + [CLOSED NEGATIVE
— ISOLATED MATCHED PSEUDOMOMENTUM]`  
**Protocol:**
[`PREREG_MINIMUM_ENERGY_NEUTRAL_PAIR_FORCE_SIGN_v1.md`](../preregistrations/PREREG_MINIMUM_ENERGY_NEUTRAL_PAIR_FORCE_SIGN_v1.md),
SHA-256 `1ECB8957CCBA4AE5770FDB310E883357F745418DD36AD30CD5C7E7D35366F341`  
**Verdict:** `MINIMUM_ENERGY_ATTRACTION_RESTORED_MOMENTUM_CHANNEL_MISSING`

## Minimum-energy theorem

Let `D` be the periodic face divergence and let `rho` have zero mean. If

\[
(D D^T)\phi=\rho,\qquad E_{\min}=D^T\phi,
\]

then every other Gauss-realizing field is `E=E_min+T` with `DT=0`. Periodic
adjointness gives

\[
\langle E_{\min},T\rangle=\langle\phi,DT\rangle=0,
\qquad
\frac12\|E\|^2=\frac12\|E_{\min}\|^2+\frac12\|T\|^2.
\]

Thus `E_min` is the unique zero-harmonic minimum-norm representative. The
independent certificate verifies `div curl=0`, longitudinal/transverse
orthogonality, and the energy decomposition exactly over rationals.

## Measurement

The unchanged FTD-0601 transaction was initialized with this field. All 12
forward and 12 state-only inverse arms pass. The initializer Gauss residual is
`6.80e-14`, curl-adjoint residual `1.39e-17`, worst common gate `6.80e-14`, and
worst inverse `3.01e-15`. Adding a divergence-free challenge raises field
energy by `6.22e-4` while preserving Gauss.

At the registered placement the pair has inward impulse `3.274e-4`; separation
falls from `6.6413804` to `6.6411669` after one step and `6.5882410` after 16
steps. Sixteen reverse steps recover state to `4.97e-14`, with energy drift
`4.44e-16`.

The matched pseudomomentum defect remains `1.908e-4`, so exact isolated recoil
does not close. FTD-0603 further shows that the attractive sign is not robust
under fractional rigid translation; this result licenses attraction only at
the registered phase.

## Correct statement

The repulsion in FTD-0601 was an initial-dressing artifact. The selected
minimum-energy field yields reversible attraction at one placement, but the
compact carrier still lacks phase-robust force and exact matched
pseudomomentum. No electromagnetic or particle claim follows.

