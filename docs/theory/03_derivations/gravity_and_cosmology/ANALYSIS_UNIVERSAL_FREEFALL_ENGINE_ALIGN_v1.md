# ANALYSIS — Universal free-fall engine alignment v1

**Tag:** `[MEASURED — CLOSED NEGATIVE]`
**Date:** 2026-08-19
**LEDGER:** FTD-1014
**Lock:** [`PREREG_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md) — prefix SHA256 `5A99CA4FD0EB3EACA1FE9712688E0D8D118F9756CB53EC86DD860C8FCAAA3E9F`. Git tag `preregister-universal-freefall-engine-align-v1` pending owner commit; this result is **`anchored-late`** until that tag resolves.
**Instrument:** `engine/tests/test_universal_freefall_engine_align.cpp` SHA256 `BC8554773E71FF27649E7D04D18B8D023E34DF1EAE87D8F2C3CD99CA86D40E90`. CTest name `universal_freefall_engine_align` — protocol **Passed**; physics gate A1 classifies and does not fail CTest.
**Parent:** [`SCOPE_UNIVERSAL_FREEFALL_v1.md`](../../10_eft_program/scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md) step 5. Desk Q0 = FTD-1013.
**Does not move:** FTD-1013, FTD-0250, FTD-0349, FTD-0402, FTD-0208, U-8. No golden tick. No production gravity rewrite.

---

## 0 · Verdict

On the locked external-well fixture (prescribed \(\mathcal{L}=0.05+10^{-3}x\), \(J=0\), extra forces off, CPU, one forces-phase, no `tick()`), Path G reproduces Q0’s weak acceleration and Path F does not.

\[
r(N)=\frac{a_{\rm COM}(N)}{g_{\rm ext}},\qquad
g_{\rm ext}=C_{\rm SPEED}^2\,(\mathcal{L}_0+k\,x_{\rm COM})\,k.
\]

| \(N\) | \(g_{\rm ext}\) | \(r_G\) | \(r_F\) | \(\|f_{\rm gravity}\|\) |
|---|---|---|---|---|
| 1 | \(2.133333\times 10^{-5}\) | \(1.000000\) | \(0\) | \(0\) |
| 8 | \(2.150000\times 10^{-5}\) | \(1.000000\) | \(0\) | \(0\) |
| 27 | \(2.166667\times 10^{-5}\) | \(1.000000\) | \(0\) | \(0\) |

P1–P5 passed. A1 (\(\|r_F-1\|<0.05\) at every registered \(N\)) failed. Classifier: **CLOSED-NEGATIVE**. Live gravity on this fixture is **not** the Q0 action.

---

## 1 · What ran

Path G writes \(v_x=g_{\rm ext}\,\mathrm{d}t\) on locked \(+1\) cubes and does not call the forces phase. That is the Q0 weak EOM as a test-only kick. It is not a production toggle.

Path F calls the public free-function split of private `RenderBridge::phase_forces()` (`phase_forces_solve_potentials` → `phase_forces_build_color_cache` → `phase_forces_main_loop` → `phase_forces_integrate_clusters`), with `forces`, `gravity`, and `cluster_inertia` on, Poisson/`tick()` off. Production gravity remains \(F=G_N\nabla\rho\) with \(\rho=|J|\).

Constructor banners print “GPU backend active”; each bridge then `force_cpu()`. The measured update is the CPU TU split. That is not a GPU campaign and is not IMPROPER under the lock.

---

## 2 · Mechanism

The vacuity firewall named this failure in advance: \(J=0\Rightarrow\nabla\rho=0\Rightarrow F_{\rm grav}=0\Rightarrow r_F\approx 0\neq 1\). The prescribed \(\mathcal{L}\) well is unread by the live operator. Latency enters the integrator only through \(\gamma_{\rm FTD}\) when a force is present; here the force is identically zero, so \(\gamma_{\rm FTD}\) never converts the well into an acceleration.

This is the expected CLOSED-NEGATIVE, not a near-miss. No \(\varepsilon\), \(k\), or \(\mathcal{L}_0\) was retuned after seeing \(r_F\).

CI-5 (injected uniform \(f\), gravity off) was not used as A1. Using it as FOUND would have been IMPROPER.

---

## 3 · What this does not license

- Promotion of FTD-1013, or the claim that UFF is derived from the engine.
- Identification of live \(m_i=m_g\) as anything but `[IMPOSED]` (FTD-0402).
- Adoption of Path G into production, or replacement of \(\nabla\rho\) by \(\mathcal{L}\nabla\mathcal{L}\). That would be a later lock.
- A graviton, \(h_{\mu\nu}\), occupancy telegraph, strong EP, GNC, or self-force as a weak-EP falsifier.
- Closure of §12-EP: Q0 still stands given FC-2 inside class \(\mathcal{C}\); the live tick is still the \(F/M\) split.

---

*CTest protocol Passed. Frozen classifier CLOSED-NEGATIVE. Zero promotions.*
