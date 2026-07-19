# PREREG — The Gravitational-Optical Channel v2 (sharpened instrument)

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-light-deflection-channel-v2` at the registration commit)
**Parent:** [`PREREG_LIGHT_DEFLECTION_CHANNEL_v1.md`](PREREG_LIGHT_DEFLECTION_CHANNEL_v1.md) — v1 adjudicated **Indeterminate on instrument validity** (its OUTCOME section): (1) a periodic-boundary wrap artifact in the off-center packet's centroid made the floor (≈0.61) swamp the θ_γ0 yardstick; (2) the v=0.5 particle at b=10 was captured by the near-horizon well (latency_max 0.54) instead of transiting. **The question, the physics scope, the outcome philosophy, the exclusions (§6), and the code-derived null expectation are unchanged from v1.** This document registers only the instrument sharpening.

## 1 · Instrument changes (v1 → v2)

1. **Symmetric geometry.** The packet ALWAYS travels along the lattice mid-line (y = z = L/2 = 48) — identical boundary geometry in every arm, symmetric in both transverse directions. The impact parameter is realized by OFFSETTING THE MASS: ball center at (48, 48 − b, 48). C0 (no mass) is now an exact geometric twin of every W arm.
2. **Bounded transverse centroid window.** The packet centroid is computed inside |y − 48| ≤ 15, |z − 48| ≤ 15 (in addition to the moving x-window |x − x_pred| ≤ 12) — the dispersing tail that wraps the boundary never enters the weight.
3. **Differential primary observable.** θ_diff(b) ≡ θ_w(W-b) − θ_w(C0), computed from the same fit windows. Common-mode instrument drift cancels. The z-channel of every arm and θ_w(C0) itself remain the floor estimators: Floor = max(3·|θ_z| over W arms, 3·|θ_y(C0) residual after the C0 subtraction ≡ 0 by construction — replaced by 3·|θ_z(C0)|, the C0 z-channel).
4. **Particle validity arm at transit-feasible geometry.** Particle at (30, 48, 48), v = (0.5, 0, 0); mass at (48, 48 − 20, 48) (b = 20, where the well is shallow enough to transit). Same dressing as v1. Gate V2: transit completes (exit v_x > 0.25) AND |θ_p| > 10 × Floor.
5. Everything else — toggle set, mass construction, packet construction, amplitudes, equilibration, baseline subtraction, fit windows, V1/V3/V4 gates, the frozen θ_γ0 formula and its fit method — is v1 §2–§4 verbatim.

## 2 · Outcome map

v1 §5 verbatim, applied to θ_diff(b) in place of θ_w, with the v2 Floor definition above. The sign/absorption disambiguation clause (D-arm) applies unchanged. All v1 §6 anti-gaming clauses apply; additionally, this v2 re-registration is itself the §5-mandated disposition of v1's Indeterminate and may not be presented as a fresh first look.

---

*Registered 2026-07-18, before the v2 instrument's first execution. Author: session 8294fddb, following LOCK-STD v1.*
