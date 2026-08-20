# ANALYSIS — Live sourced Newton v1

**Tag:** `[MEASURED — LIVE SOURCED NEWTON, PROBE SOURCES]`
**Date:** 2026-08-19
**LEDGER:** FTD-1021
**Lock:** [`PREREG_LIVE_SOURCED_NEWTON_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_LIVE_SOURCED_NEWTON_v1.md) prefix SHA256 `9D76CCBB63C05BEDE4B07A71DC68CA96A18305888D9071BA093A206B398D7EEF` (`anchored-late`).
**Instrument:** `engine/tests/test_live_sourced_newton.cpp` SHA256 `CDFA232627E341838C5FA486C2A84167591BF46ABCD8A4B48983C1A7D40FC12E`
**Does not move:** FTD-1013–1020, FTD-0131, FTD-0250, FTD-0349, FTD-0361, FTD-0402, FTD-0208, U-8. Does not derive \(m_i=m_g\), physical \(G_N\), \(1/r^2\), or strong EP / Nordtvedt.

---

## Verdict

**FOUND — live wiring, well responds.** Protocol P1–P10 Passed. A1 Passed. A2 Failed. A3 Failed. CTest `live_sourced_newton` Passed.

FTD-1016 still matches Q0 on a **live** Poisson \(\mathcal{L}\) (\(r_L=0.9857\), inside 0.05). The FTD-1017 freeze is **not** a test-body approximation at \(N_p/N_s=1/125\): the live kick is \(\approx 4.7\times\) the freeze kick. Production-tick arm T is byte-identical to the split arm L (\(\delta_T=0\)).

Self-**force** is not the cause. A probe-only well (arm S) has \(\rho_S=5.5\times10^{-16}\) and \(g_S\sim10^{-19}\). The lone self-well is symmetric under the tier-2 stencil. What fails the freeze is self-**depth**: the probe sits in its own sharp \(L\) peak, and \(F=M C^2\,\mathcal{L}\,\nabla\mathcal{L}\) multiplies the external tilt by that on-site \(L\).

---

## Numbers (CPU observer, \(L=32\), source origin \((6,13,13)\), probe \((18,15,15)\), \(\Delta x=+10\))

| Arm | \(\mathcal{L}\) at probe | \(g_x\) | \(a_x\) | \(r=a/g\) |
|---|---|---|---|---|
| **Z** freeze (1017 replica) | \(3.027058\times10^{-2}\) | \(-2.917418\times10^{-4}\) | \(-2.916081\times10^{-4}\) | \(0.999542\) |
| **L** live split | \(1.686603\times10^{-1}\) | \(-1.392423\times10^{-3}\) | \(-1.372472\times10^{-3}\) | \(0.985671\) |
| **T** production tick | \(1.686603\times10^{-1}\) | \(-1.392423\times10^{-3}\) | \(-1.372472\times10^{-3}\) | \(0.985671\) |
| **S** probe only | \(1.401472\times10^{-1}\) | \(-1.620778\times10^{-19}\) | \(-1.604782\times10^{-19}\) | \(0.990131\) |

| Ratio | Value | Gate |
|---|---|---|
| \(\delta_a=\lvert a_L-a_Z\rvert/\lvert a_Z\rvert\) | \(3.707\) | A2 \(<0.05\) **fail** |
| \(\delta_g=\lvert g_L-g_Z\rvert/\lvert g_Z\rvert\) | \(3.773\) | A3 \(<0.05\) **fail** |
| \(\delta_T=\lvert a_T-a_L\rvert/\lvert a_L\rvert\) | \(0\) | P10 **pass** |
| \(\rho_S=\lvert a_S\rvert/\lvert a_Z\rvert\) | \(5.503\times10^{-16}\) | P9 **pass** |
| \(\lvert r_L-1\rvert\) | \(0.0143\) | A1 **pass** |
| \(L_L/L_Z\) | \(5.572\) | (not a gate) |
| \(\lvert g_L/g_Z\rvert\) | \(4.773\) | (not a gate) |

Arm Z matches FTD-1017 to the printed digits. Constructor logs “GPU backend active” then `force_cpu()` — not a GPU campaign, not IMPROPER.

`RenderBridge::solve_latency_poisson` is private. Arm L re-solves by a public latency-only `tick()` (Rule 3c, forces off), then the same `phase_forces` split as 1017. That is the engine’s public Poisson path. Arm T is the same occupancy with Poisson and forces in one production tick; \(\delta_T=0\) says the split is not a fake.

---

## What the continuum identity would have predicted

Inside a well, \(\mathcal{L}=\sqrt{\mathrm{clamp}(-\phi)}\). If the clamp is inactive and \(\mathcal{L}\) varies slowly,

\[
C^2\,\mathcal{L}\,\nabla\mathcal{L}=\frac{C^2}{2}\nabla(\mathcal{L}^2)=-\frac{C^2}{2}\nabla\phi.
\]

A symmetric self-\(\phi\) has \(\nabla\phi_{\rm self}=0\) at the occupant, so a Newtonian reading of \(\nabla\phi\) would leave \(g_L\approx g_Z\). Arm S confirms the discrete self-gradient is zero. The continuum identity therefore **fails to describe this fixture**, not because the probe self-accelerates, but because the **tier-2 product** is not the discrete \(\nabla(\mathcal{L}^2)/2\) on a peaked well.

The engine evaluates

\[
g_x=C^2\,\mathcal{L}(x)\cdot\frac{\mathcal{L}(x+2)-\mathcal{L}(x-2)}{4}.
\]

That equals \(\tfrac12\nabla(\mathcal{L}^2)\) only when \(\mathcal{L}(x)\approx(\mathcal{L}(x+2)+\mathcal{L}(x-2))/2\). On a slow source well (arm Z) that holds, and \(r_Z=0.9995\). On a one-voxel self-peak, \(\mathcal{L}(x)\) is the peak and the \(\pm 2\) samples sit on the sides. A small *asymmetric* \(\Delta\mathcal{L}\) from the external source is then multiplied by the *peak* \(\mathcal{L}\), not by the side average. Ratio \(L_L/L_Z=5.57\) vs \(\lvert g_L/g_Z\rvert=4.77\) is that amplification, not a new Poisson coupling.

So:

1. **Wiring.** The operator is local in the written \(\mathcal{L}\). Live Poisson is a well it reads (A1).
2. **Test-body freeze.** FTD-1017’s photograph omits the occupant’s own \(L\)-depth. At \(N_p=1\) that depth is first-class (\(L_S=0.140\) vs source-only \(L_Z=0.030\)).
3. **Not self-force.** \(\rho_S\sim10^{-16}\). The later strong-EP / two-mass lock is still unrun.
4. **Not Newton’s \(G\).** Engine \(G_N=0.01\) still sets the Poisson source (FTD-0131).

---

## What this is not

- Not Nordtvedt: two comparable, moving, self-gravitating bodies were not run.
- Not a derivation of \(m_i=m_g\). Live \(M_{\rm INERTIAL}\) is still imposed.
- Not a \(1/r^2\) theorem. Periodic Green’s function is Phase G.
- Not a reason to insert \(n(\mathcal{L})\) into `phase_read`. Waves remain unread (FTD-1020).
- Not production-default gravity. Golden profile still uses \(\nabla|J|\).

---

## Residual for a later lock

A slow-envelope occupant (extended clump, GNC, or a probe whose self-well is filtered out of \(\nabla\mathcal{L}\)) is the natural next test of whether the freeze can be recovered as a genuine test-body limit. Two comparable masses remain the strong-EP lock. This row does not take either.
