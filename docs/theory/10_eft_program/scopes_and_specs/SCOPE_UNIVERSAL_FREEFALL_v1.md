# SCOPE — Universal free fall (weak EP) as geometry of an external well

**Tag:** `[SCOPE / OPEN PROGRAM]` — Q0 desk step **FOUND** 2026-08-19 (FTD-1013); default-operator alignment **CLOSED-NEGATIVE** 2026-08-19 (FTD-1014); selected geometric integrator **FOUND** 2026-08-19 (FTD-1016). This SCOPE is not itself a theorem.
**Date:** 2026-08-19
**Q0 result:** [`DERIV_UNIVERSAL_FREEFALL_Q0_v1.md`](../../03_derivations/gravity_and_cosmology/DERIV_UNIVERSAL_FREEFALL_Q0_v1.md) — `[THEOREM given FC-2]` for test-body UFF inside locked class \(\mathcal{C}\); class \(\mathcal{C}\) remains `[SELECTION]`; live \(m_i=m_g\) remains `[IMPOSED]`.
**Engine-align result:** [`ANALYSIS_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md`](../../03_derivations/gravity_and_cosmology/ANALYSIS_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md) — `[MEASURED — CLOSED NEGATIVE]`; default \(F=G_N\nabla|J|\) is not Q0 \(g_{\rm ext}\) on the locked fixture.
**Geometric-integrator result:** [`ANALYSIS_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md`](../../03_derivations/gravity_and_cosmology/ANALYSIS_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md) — `[MEASURED — SELECTED INTEGRATOR, FOUND]`; default-off `geometric_gravity` reproduces Q0 on the same fixture.
**Serves:** `SPEC_OPEN_MATH_BY_SECTOR.md` §12-EP (no canonical anchor). Sibling to [`SCOPE_GW_AREA_HOLONOMY_v1.md`](SCOPE_GW_AREA_HOLONOMY_v1.md): that memo is radiating transport; this one is **falling in a static well**.
**Does not mint** a LEDGER row. Does not move FTD-0250, FTD-0349, FTD-0402, FTD-0208, or U-8.

**Companions:**
- [`SPEC_FTD_LAGRANGIAN.md`](../../01_reference/SPEC_FTD_LAGRANGIAN.md) §4.1–§4.3 — inertia, latency source, clock budget: equality of mass roles **imposed**; EP **not derived**
- [`DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md`](../../03_derivations/foundational_mechanics/DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md) — FTD-0349: cluster inertia = \(N\cdot M_{\rm REST}\) iff GNC, which the action does not force
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../../03_derivations/gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md) — FTD-0131: universal *sourcing*, not universal *falling*
- [`FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md`](../../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md) §14.4 — “emergent EP from shared budget” is `[SELECTION]` narrative, not an anchor

---

## 0 · What universal free fall is

Weak EP: in a given external gravitational field, a test body’s kinematic acceleration does not depend on its mass or composition.

That is **not** “every mass sources gravity.” Sourcing is already the Poisson law \(\rho = K_B\cdot n\) (every manifested voxel writes \(\mathcal{L}\)). Free fall is the **response** law: does a feather and a cannonball, a charged cluster and a neutral one, follow the same worldline when only gravity is on?

If gravity is a force \(F = m_g g\) and inertia is \(a = F/m_i\), EP is the extra statement \(m_g = m_i\). If gravity is clock-transport geometry, \(m_g\) should not appear on the test body at all: the well tells clocks how to accumulate, and occupants that only carry clocks all accumulate the same way.

---

## 1 · What is already true, imposed, or blocked

| Piece | Status | Content |
|---|---|---|
| Universal sourcing | licensed Newton chain | Every manifested voxel sources \(\mathcal{L}\). Small bodies source too. |
| \(M_{\rm INERTIAL}=M_{\rm GRAVITATIONAL}=K_B\) | **[IMPOSED]** (FTD-0402) | Engine names two roles and sets them equal. Spec §4.2: this is **not** a variation of the Born–Infeld term. |
| Per-voxel update \(a = F/M_{\rm INERTIAL}\) | selected implementation | Forces accumulate, then one \(P/M\) kick. Gravity in the live loop is still a **force**. |
| Cluster inertia \(a_{\rm COM}=F/(N M_{\rm REST})\) | **[IMPOSED]** (FTD-0250) | Needed so \(F_{\rm tot}\propto N\) does not make big clusters fall faster. |
| GNC (gradient-normalization) | **[PARTIAL]** (FTD-0349) | Field kinetic coefficient equals count-weighted rest mass iff a texture the action does not force. Spatial twin of the clock axiom. |
| Clock / bandwidth law | **[AXIOM]** (FC-2 / FTD-0208) | Turns \(\mathcal{L}\) into \(d\tau/dt\). Spec §3.7: does not derive EP. |
| Composition (colour, EM, lock) | live extra forces | “Free” fall means those channels off. Colour on is an EP-violating composition channel until shown otherwise (FTD-0400 split-bookkeeping). |
| Strong EP / self-force | out of v1 | Binding energy, self-well, tides. Do not smuggle into weak EP. |

The engine can *look* like EP by cancelling two scalings (\(F\propto N\) and \(m\propto N\)), both put in by hand. That is not universal free fall from the ontology.

The FOUND_SPACETIME line that EP “emerges because motion and gravity both spend tick budget” is a local-indistinguishability story about SR vs gravity (Einstein’s elevator), tagged `[SELECTION]`. It does not prove mass-independence of fall.

---

## 2 · The geometric content (test-body limit)

Fix an **external** latency field \(\mathcal{L}_{\rm ext}\) (source not the test body; slow spatial variation across the body, tides neglected).

**Claim to prove or fail (weak EP, geometry slot):**

> If a test occupant’s action is homogeneous of degree one in its only gravitational coupling (a single factor of \(K_B\) multiplying the proper-time element built from \(\mathcal{L}_{\rm ext}\) and \(u\)), then the Euler–Lagrange equation for the worldline does not contain \(K_B\), nor the occupant’s \(N\), nor its other charges.

That is the standard reason a test particle of mass \(m\) on \(S = -m\int d\tau[g]\) follows a geodesic independent of \(m\). It is **not** yet FTD’s live update. Spec §4.1–§4.2 say the engine does not obtain gravity by varying that action: it adds forces and divides by an imposed \(M_{\rm INERTIAL}\).

So the program splits:

1. **Q0 (desk).** **FOUND 2026-08-19 (FTD-1013).** Unique action in locked class \(\mathcal{C}\) is \(S=-\alpha\int d\tau\) with FC-2 \(d\tau\); weak EL is \(a=-\nabla\Phi_N\), \(\Phi_N=-(C_{\rm SPEED}^2/2)\mathcal{L}^2\). Tag: `[THEOREM given FC-2]` inside \(\mathcal{C}\). Class \(\mathcal{C}\) itself stays `[SELECTION]`. Phase-G \(1/r\) is cited, not re-proved.
2. **Engine alignment (locked, executed).** **CLOSED-NEGATIVE 2026-08-19 (FTD-1014).** Default live \(F=G_N\nabla|J|\) does not reproduce \(g_{\rm ext}=C_{\rm SPEED}^2\,\mathcal{L}\,\nabla\mathcal{L}\) on the prescribed-well fixture. Path G (test-only Q0 kick) gives \(r_G=1\); Path F gives \(r_F=0\). Live \(m_i=m_g\) stays `[IMPOSED]`.
3. **Geometric integrator (locked, executed).** **FOUND 2026-08-19 (FTD-1016).** Default-off CPU toggle `geometric_gravity` applies \(F=M_{\rm INERTIAL} C^2\,\mathcal{L}\,\nabla\mathcal{L}\). Same fixture: \(r_{F{\rm-on}}\approx 0.9979\) (A1 pass); toggle-off residue \(r_{F{\rm-off}}=0\). Production default and golden tick unchanged. Coverage of measured gravity phenomena: [`CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md`](../../03_derivations/gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md).

Per-voxel geometry also dissolves the naive cluster paradox: if every voxel falls at local \(g\), the COM falls at \(g\) without a collective \(N\cdot M\) integrator. Cluster inertia is then a *rigid-body* device for locked clumps, not the bearer of EP. GNC remains the question whether a locked clump’s *inertial* kinetic term matches \(N\cdot K_B\) when other forces act — a different theorem.

---

## 3 · What would violate UFF (falsifiers of a later claim)

- Residual \(a(N)\) in an external well with extra forces off, after the geometric update (toggle ON).
- Composition: two test bodies, same \(N\), different colour/charge, extra forces off, different \(a\). (Extra forces *on* is not free fall.)
- Using self-field or GNC failure as a weak-EP falsifier — wrong theorem.
- Citing \(M_{\rm INERTIAL}=M_{\rm GRAVITATIONAL}\) in code as a derivation.

---

## 4 · Program sequence

1. This SCOPE.
2. **Q0** hash-lock — **executed FOUND** (`PREREG_UNIVERSAL_FREEFALL_Q0_v1.md`, prefix SHA `1F8A8005…15B23`, verifier 15/15, `anchored-late` pending git tag).
3. §12-EP now has a desk anchor (FTD-1013). It is **not** closed: live \(m_i=m_g\) remains `[IMPOSED]`; class \(\mathcal{C}\) remains `[SELECTION]`.
4. Booked: `[THEOREM given FC-2]` for the *test-body, external, composition-clean* statement only. Strong EP and GNC stay separate rows. Engine \(m_i=m_g\) unmoved.
5. Engine campaign — **executed CLOSED-NEGATIVE** (`PREREG_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md`, prefix SHA `5A99CA4F…A3E9F`, CTest protocol pass / A1 fail, `anchored-late` pending git tag). Default operator remains \(\nabla|J|\).
6. Geometric integrator — **executed FOUND** (`PREREG_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md`, prefix SHA `B8253510…C7287`, CTest P1–P6 + A1 pass, `anchored-late` pending git tag). Selected, default-off. v1 lock CPU-only; native CUDA port is FTD-1018. Does not derive \(m_i=m_g\).
7. Sourced well — **executed FOUND** (`PREREG_SOURCED_GEOMETRIC_FREEFALL_v1.md`, prefix SHA `A4289563…0D39`, CTest P1–P6 + A1 pass, `anchored-late` pending git tag). Poisson writes \(\mathcal{L}\); frozen well; probe falls, \(r_{\rm on}=0.999542\). Not \(1/r^2\), not strong EP.
8. GPU port — **executed FOUND** (`PREREG_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md`, prefix SHA `624969CA…EE9E`, \(\Delta F=\Delta v=0\), `anchored-late` pending git tag).
9. One-well clocks + falling — **executed FOUND** (`PREREG_ONE_WELL_REDSHIFT_FALLING_v1_1.md`, prefix SHA `5DB20B6F…D0F5`; v1 UNDERDETERMINED on P1). Frozen Poisson \(\mathcal{L}\) drives FC-2 rest clocks (\(\rho_\tau=1\)) and FTD-1016 falling. Not Pound–Rebka.
10. Frozen-well characteristics — **executed FOUND class 0** (`PREREG_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md`, prefix SHA `B6BF393F…2C034`). Live `wave_propagation` unread of frozen vacuum \(\mathcal{L}\) (\(\theta_{\rm diff}=0\)). Lensing stays OUT. Not Eddington.
11. Live sourced Newton — **executed FOUND wiring, well responds** (`PREREG_LIVE_SOURCED_NEWTON_v1.md`, prefix SHA `9D76CCBB…7EEF`). Operator reads live \(\mathcal{L}\) (\(r_L=0.9857\)). Freeze is not test-body at \(1/125\) (\(\delta_a=3.71\)). Self-force null. Not Nordtvedt.
12. Slow-envelope live Newton — **executed FOUND wiring, envelope still responds** (`PREREG_SLOW_ENVELOPE_LIVE_NEWTON_v1_1.md`, prefix SHA `5D0BB44F…43FF9`; v1 UNDERDETERMINED on P6). Locked 3³ at the 1021 COM. Operator tracks live \(\mathcal{L}\) (\(r_L=0.9546\)). Freeze is not test-body at \(27/125\) (\(\delta_a=3.18\)). Self-force null. Not Nordtvedt.

No golden-tick change. No graviton. No occupancy telegraph.

---

## 5 · Relation to “gravity is not a force carrier”

Universal sourcing: manifestation writes the well (all sizes).
Universal free fall: occupants that only carry clocks follow that well the same way.
Neither step is boson exchange. The first is already in the Newton chain. The second is this program: Q0 is booked (FTD-1013) and still rides FC-2; the default tick is still the imposed \(F/M\) split (FTD-1014 CLOSED-NEGATIVE); a selected default-off integrator instantiates the Q0 kick (FTD-1016 FOUND).

---

*This document names the §12-EP job. Q0 is booked at FTD-1013; default-operator alignment is booked at FTD-1014 CLOSED-NEGATIVE; the selected geometric integrator is booked at FTD-1016 FOUND; the CUDA port at FTD-1018 FOUND; one-well clocks+falling at FTD-1019 FOUND; frozen-well characteristics at FTD-1020 FOUND class 0; live sourced Newton at FTD-1021 FOUND wiring (freeze not test-body at 1/125); slow-envelope 3³ at FTD-1022 FOUND wiring (freeze not test-body at 27/125). The SCOPE does not move engine mass-role rows and does not license lensing or GWs.*
