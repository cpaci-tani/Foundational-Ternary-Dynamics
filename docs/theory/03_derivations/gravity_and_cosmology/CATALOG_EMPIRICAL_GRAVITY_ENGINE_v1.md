# CATALOG — Empirically measured gravity phenomena vs the FTD engine

**Tag:** `[SYNTHESIS]` — coverage map, not a derivation. Moves no LEDGER tag.
**Date:** 2026-08-19
**LEDGER:** FTD-1016 companion (the geometric integrator), plus FTD-1017 / 1018 / 1019 / 1020 / 1021 / 1022 coverage. Introduces no new derivation.
**Does not claim** that FTD reproduces GR, nor that every laboratory gravity result is an engine output.

This catalog answers the owner question “do we have all empirically measured phenomena?” for **gravity**. Non-gravity empirics (QED loops, mass ratios, mixing angles, …) live in [`CATALOG_PARAMETRIC_INSERTIONS.md`](../../07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) at their own tags and are **not** this campaign.

FTD’s recovered “time aspect” is the **clock / \(g_{00}\)** sector (FC-2 + FTD-1013 + U-8), not licensed GR. The engine already stores `voxel.latency` and integrates \(\gamma_{\rm FTD}\). Live default gravity was \(F=G_N\nabla|J|\) (FTD-1014 CLOSED-NEGATIVE). FTD-1016 adds a default-off operator that reads that latency.

---

## Status vocabulary

| Status | Meaning |
|---|---|
| **IN** | This campaign (or a named existing engine path) instantiates the phenomenon as an engine update. Tag of the physics claim is unchanged. |
| **ALREADY** | Present in the engine before FTD-1016; this lock does not add a new operator. |
| **OUT** | Empirically measured, and FTD does **not** put it in the engine on this path. Named obstruction stands. |
| **TOY / SELECTED WIRING** | An engine law exists, but the identification with the laboratory constant is not derived (or is falsified). |

---

## Table — measured gravity phenomena

| Phenomenon | Laboratory / astrophysical witness | Engine status | Tag of record | Notes |
|---|---|---|---|---|
| Universal free fall / weak EP | Eötvös, MICROSCOPE, lunar laser ranging (weak-EP subset) | **IN** (toggle `geometric_gravity`, default OFF) | FTD-1013 `[THEOREM given FC-2]` inside class C; live \(m_i=m_g\) `[IMPOSED]` (FTD-0402); class C `[SELECTION]` | Extra forces off. Composition (colour/EM on) is not free fall. |
| Gravitational redshift / clock dilation | Pound–Rebka, GPS, Pound–Snider, gravity-probe clocks | **ALREADY** (operator) + **MEASURED on a sourced well (FTD-1019 FOUND)** | FC-2 `[AXIOM]`; U-8 Newton + leading \(g_{00}\); FTD-0208 clock hypothesis folded into FC-2 | Latency → \(d\tau/dt=\sqrt{1-v^2/C^2-\mathcal{L}^2}\). FTD-1019: same frozen Poisson \(\mathcal{L}\) as FTD-1017 falling; rest clocks \(\rho_\tau=1\) without re-solving Poisson. Not Pound–Rebka ppm. `de_broglie_redshift` remains a different (KG) clock. |
| Newtonian inverse-square falling | Cavendish, Kepler, lunar/planetary ephemerides (Newtonian limit) | **IN as sourced wiring (FTD-1017 freeze FOUND; FTD-1021 live 1-voxel FOUND; FTD-1022 live 3³ FOUND; freeze not test-body at 1/125 or 27/125)** | Phase G geometric Coulomb `[THEOREM]` for the *well shape* of Poisson; FTD-0131 physical \(G_N=1/100\) **falsified**; \(\alpha_G(e,e)\) `[SMC]` | Heavy source writes \(\mathcal{L}\) via `latency_field`. Freeze: FTD-1016 falls a light probe, \(r_{\rm on}=0.999542\). Live re-solve (FTD-1021): operator still tracks live \(\mathcal{L}\) (\(r_L=0.9857\)), but \(a_L\approx 4.7\,a_Z\) because the probe’s own \(L\)-depth multiplies the external tilt. A locked 3³ at the same COM (FTD-1022) does not restore freeze≈live (\(r_L=0.9546\), \(\delta_a=3.18\)). Self-force \(\rho_S\sim10^{-16}\). Engine \(G_N=0.01\) remains the lattice toy coupling. Not a \(1/r^2\) re-proof. Not two-mass Nordtvedt. |
| Light deflection / gravitational lensing | Eddington, VLBI, lensing surveys | **OUT** | FTD-0361 \(g_{rr}\) **RETRACTED**; Gap 10.1; FTD-1020 **FOUND class 0** | Frozen vacuum Poisson \(\mathcal{L}\) does not curve live `wave_propagation` characteristics (\(\theta_{\rm diff}=0\) vs non-vacuous \(\theta_{1911}=-5.84\times10^{-2}\)). Class 0 is not “nature has no lensing.” Do not fake a refractive index. |
| Shapiro time delay | Cassini, planetary radar | **OUT** | same \(g_{rr}\) gap | Null-geodesic extra path; not a clock-only effect. |
| Perihelion / PPN beyond \(g_{00}\) | Mercury, solar-system PPN | **OUT** | U-8: Newton + leading \(g_{00}\) only | |
| Frame dragging / geodetic precession | Gravity Probe B, LAGEOS | **OUT** | no shift vector / SO(1,3) connection | |
| Gravitational waves (quadrupole, plus/cross) | Hulse–Taylor, LIGO/Virgo/KAGRA | **OUT** | FTD-1015 `[CLOSED NEGATIVE]`; FTD-0209 catalog; P6C-G unadopted | Spatial \(\Omega\) kinematics do not isolate two TT shears. Do not posit \(h_{\mu\nu}\). |
| Strong-field BH phenomenology | EHT shadows, ringdown, ISCO | **OUT** | FTD-0184 / imported GR metrics | Engine BH wells exist as latency clamps, not as derived Kerr. |
| Cosmology (Hubble, BAO, CMB peaks) | Planck, DESI, SH0ES | **OUT** | FTD-0331 \(\Lambda=0\) `[BOUNDARY]`; Friedmann ansatz `[CONJECTURE]` | |
| Strong EP / Nordtvedt (self-gravity in falling) | LLR Nordtvedt | **OUT** of v1 | FTD-0349 GNC `[PARTIAL]` | Self-well / binding energy is a later lock. |

---

## What FTD-1016 actually put in the tick

When `geometric_gravity` is ON (`gravity` and `forces` required; native CUDA + CPU, default OFF):

\[
\mathbf F = M_{\rm INERTIAL}\,C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L},\qquad
\mathcal{L}=\texttt{voxel.latency}.
\]

When OFF (production default, golden profile): \(F=G_N\nabla|J|\) remains, and FTD-1014 still holds.

Sourcing is measured: FTD-1017 **FOUND** (\(r_{\rm on}=0.999542\)). Native CUDA parity of the falling operator is FTD-1018 **FOUND** (\(\Delta F=\Delta v=0\)). One frozen Poisson well driving both rest clocks and falling is FTD-1019 **FOUND** (\(\rho_\tau=1\), \(r_{\rm on}=0.999542\)). Frozen-well vacuum characteristics are FTD-1020 **FOUND class 0** (\(\theta_{\rm diff}=0\)). Live Poisson occupancy is FTD-1021 **FOUND wiring** (\(r_L=0.9857\)) with the freeze **not** a test-body approximation at \(1/125\) (\(\delta_a=3.71\)). A locked 3³ occupant is FTD-1022 **FOUND wiring** (\(r_L=0.9546\)) with freeze still **not** recovered at \(27/125\) (\(\delta_a=3.18\)). None of these retune \(G_N\), adopt P6C-G, or license lensing or GWs.

---

## Honest residual

“All empirically measured phenomena” **of gravity** are not all in the engine. The recovered sector is clocks + weak falling. FTD-1021 and FTD-1022 show that falling in a *live* well is not the FTD-1017 photograph: occupant \(L\)-depth multiplies the external tilt at both \(N_p=1\) and \(N_p=27\). Spatial curvature, radiation, and cosmology remain OUT. FTD-1020 remains the wave-sector structural null.
