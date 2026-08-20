# ANALYSIS — Slow-envelope live Newton v1 / v1.1

**Tag:** `[MEASURED — SLOW-ENVELOPE STILL SOURCES]`
**Date:** 2026-08-19
**LEDGER:** FTD-1022
**Locks:** v1 [`PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1.md) prefix SHA256 `4C82033F93AC8A182E4B7AE7538BD6F3065428823C850853D4C03CC4AEDD9D25` (**UNDERDETERMINED** on P6). v1.1 [`PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1_1.md`](../../10_eft_program/preregistrations/gravity_cosmology/PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1_1.md) prefix SHA256 `5D0BB44FFDDEF81C1B3E84DFB46F45A79508DFDEDC9EAE611594776B07843FF9` (**FOUND wiring, envelope still responds**; `anchored-late`).
**Instrument:** `engine/tests/test_slow_envelope_live_newton.cpp` SHA256 `3C4BAE486B4451FE647FD66B4AADD6FA4B3B39C0555198CF727800F1318960C5` (v1.1 member-mean \(\bar g\)).
**Does not move:** FTD-1013–1021, FTD-0131, FTD-0250, FTD-0349, FTD-0361, FTD-0402, FTD-0208, U-8. Does not derive \(m_i=m_g\), physical \(G_N\), \(1/r^2\), or strong EP / Nordtvedt.

---

## Verdict

**v1 UNDERDETERMINED.** \(g\) was the Q0 stencil at the COM voxel; \(a\) was the 27-site cluster mean of local \(C^2\mathcal{L}\nabla\mathcal{L}\). P6 required \(\lvert r_Z-1\rvert<0.05\) and saw \(r_Z=0.887356\). \(a_Z=-2.588788\times10^{-4}\) already matched the member-mean field; \(g_Z=-2.917418\times10^{-4}\) was the COM-only sample (same as FTD-1021 freeze). A1/A2/A3 are not a v1 verdict.

**v1.1 FOUND — live wiring, envelope still responds.** The only repair: \(\bar g\) is the equal-weight mean of the same stencil over all 27 probe members, matching `cluster_inertia` algebra. Protocol P1–P10 Passed. A1 Passed. A2 Failed. A3 Failed. CTest `slow_envelope_live_newton` Passed.

FTD-1016 still matches live Q0 on a 3³ occupant (\(r_L=0.954590\), \(\lvert r_L-1\rvert=0.04541\), inside \(\varepsilon=0.05\)). Freeze≈live does **not** return: \(\delta_a=3.183\), \(\delta_g=3.380\). The FTD-1021 jump was not a one-voxel artefact. Self-force remains null (\(\rho_S=1.16\times10^{-16}\)). Production-tick arm T is identical to split arm L (\(\delta_T=0\)).

---

## Numbers (CPU observer, \(L=32\), source origin \((6,13,13)\) edge 5, probe origin \((17,14,14)\) edge 3, COM \((18,15,15)\), \(\Delta x=+10\))

### v1.1 (member-mean \(\bar g\); physics execution)

| Arm | \(\mathcal{L}\) at COM | \(\bar g_x\) | \(a_x\) | \(r=a/\bar g\) |
|---|---|---|---|---|
| **Z** freeze | \(3.027058\times10^{-2}\) | \(-2.589412\times10^{-4}\) | \(-2.588788\times10^{-4}\) | \(0.999759\) |
| **L** live split | \(3.374207\times10^{-1}\) | \(-1.134269\times10^{-3}\) | \(-1.082762\times10^{-3}\) | \(0.954590\) |
| **T** production tick | \(3.374207\times10^{-1}\) | \(-1.134269\times10^{-3}\) | \(-1.082762\times10^{-3}\) | \(0.954590\) |
| **S** probe only | \(3.241106\times10^{-1}\) | \(-1.606\times10^{-20}\) | \(+3.015\times10^{-20}\) | (noise; not a gate) |

| Ratio | Value | Gate |
|---|---|---|
| \(\delta_a=\lvert a_L-a_Z\rvert/\lvert a_Z\rvert\) | \(3.183\) | A2 \(<0.05\) **fail** |
| \(\delta_g=\lvert\bar g_L-\bar g_Z\rvert/\lvert\bar g_Z\rvert\) | \(3.380\) | A3 \(<0.05\) **fail** |
| \(\delta_T=\lvert a_T-a_L\rvert/\lvert a_L\rvert\) | \(0\) | P10 **pass** |
| \(\rho_S=\lvert a_S\rvert/\lvert a_Z\rvert\) | \(1.165\times10^{-16}\) | P9 **pass** |
| \(\lvert r_Z-1\rvert\) | \(2.41\times10^{-4}\) | P6 **pass** |
| \(\lvert r_L-1\rvert\) | \(0.04541\) | A1 **pass** |
| \(L_L/L_Z\) (COM) | \(11.15\) | (not a gate) |
| \(\lvert\bar g_L/\bar g_Z\rvert\) | \(4.380\) | (not a gate) |

\(a_Z\) is byte-identical to v1. \(\bar g_Z\) moved from the COM sample to the cluster mean and P6 closed. Constructor logs “GPU backend active” then `force_cpu()` — not a GPU campaign, not IMPROPER.

### v1 (COM \(g\); protocol only)

\(g_Z=-2.917418\times10^{-4}\) (FTD-1021 freeze COM field), \(a_Z=-2.588788\times10^{-4}\), \(r_Z=0.887356\). CTest Failed on P6. No physics class.

---

## What the 3³ did and did not change

FTD-1021’s one-voxel occupant sat in a self-peak (\(L_S=0.140\) vs source-only \(L_Z=0.030\)). A locked 3³ at the same COM is spatially broader (\(N_p=27\ll N_s=125\)) but still peaked on the tier-2 stencil: COM \(\pm 2\) still samples outside the cube, and live COM \(\mathcal{L}\) is *deeper* than the one-voxel live well (\(0.337\) vs \(0.169\)) because twenty-seven source voxels write Poisson.

The operator remains local in written \(\mathcal{L}\). Cluster inertia makes \(a_{\rm COM}\) the mean of the 27 local \(C^2\mathcal{L}\nabla\mathcal{L}\) kicks; v1.1 \(\bar g\) is that same mean, so freeze \(r_Z=0.999759\) and live \(r_L=0.9546\) are identity checks, not a new force. The freeze photograph still omits the occupant’s own \(L\)-depth. Continuum \(C^2 L\nabla L=\tfrac12\nabla L^2=-\tfrac12\nabla\phi\) still fails on a peaked well for the same discrete-product reason as FTD-1021.

So:

1. **Wiring.** The operator tracks live \(\mathcal{L}\) for a rigid 3³ (A1, just inside \(\varepsilon\)).
2. **Test-body freeze.** Not recovered at \(N_p/N_s=27/125\). \(\delta_a=3.18\) is the same class of jump as the one-voxel \(\delta_a=3.71\).
3. **Not self-force.** \(\rho_S\sim10^{-16}\). Not Nordtvedt.
4. **Not Newton’s \(G\).** Engine \(G_N=0.01\) still sets the Poisson source (FTD-0131).

---

## What this is not

- Not Nordtvedt: two comparable, moving, self-gravitating bodies were not run.
- Not a derivation of \(m_i=m_g\). Live \(M_{\rm INERTIAL}\) is still imposed.
- Not a \(1/r^2\) theorem.
- Not a reason to insert \(n(\mathcal{L})\) into `phase_read`. Waves remain unread (FTD-1020).
- Not production-default gravity. Golden profile still uses \(\nabla|J|\).
- Not a widening of \(\varepsilon\) after seeing \(r_L=0.9546\). The gate was locked at \(0.05\).

---

## Residual for a later lock

A still-smoother envelope (larger clump, GNC, or a probe whose self-well is filtered out of \(\nabla\mathcal{L}\)) is the natural next test of whether freeze can be a genuine test-body limit. Two comparable masses remain the strong-EP / Nordtvedt lock. This row does not take either.
