# EXPLR — The Substrate's Native Angle Is the Symplectic (Quadrature) Clock

**Document type:** Exploratory measurement + interpretation
**Status:** `[MEASURED]` (the dynamical facts) + `[SELECTION]` (the quadrature identification) + `[BOUNDARY]` (separation from the α-readout). **No claim is promoted; FTD-0013 and MC-T4.3 are unchanged; this is not a derivation of QM.**
**Created:** 2026-06-07
**Runner:** [`engine/tests/test_substrate_angle_probe.cpp`](../../../engine/tests/test_substrate_angle_probe.cpp) (CTest `substrate_angle_probe`, CPU, 10/10 PASS)
**Plan:** `.claude/plans/plan-an-intuitive-path-twinkling-gizmo.md`
**LEDGER row:** FTD-0251.

---

## 0 · One-paragraph result

The question "what, if anything, in the substrate plays the role of a measurement-angle / basis choice?" is answered empirically. Under the bare wave + Gauss dynamics, the substrate carries **two kinds of angle that behave oppositely**: the **transverse spatial orientation** `arg(J_x + iJ_y)` is **dynamically frozen** (a y-polarised mode never grows an x/z component — leakage `~10⁻¹⁶`, machine zero), while the **symplectic phase** `arg(q + ip)` of each mode (`q = Σ_x J_y sin kx`, `p = Σ_x ∂_t J_y sin kx`) **winds at the dispersion frequency `ω(k)`** (net winding tens of radians; the multi-tick winding rate matches the single-tick eigenvalue `ω` to 0.1–1.0%, the residual growing with mode as the leapfrog `O((ωΔt)²)` signature). A symmetric left/right dual-substrate injection stays a perfect **mirror** (`|J_L − J_R| = 0` exactly). The substrate's **only** native dynamical angle is therefore the symplectic oscillation phase — which is exactly the classical **quadrature angle** `x_θ = q cosθ + p sinθ` swept by the clock `θ = ωt` `[SELECTION]`. That angle is real but strictly **commutative**: `{q,p} ≠ 0` (Poisson — the winding is real) yet `[q,p] = 0` (observable — `THEOREM_COMMUTATIVITY_INDEPENDENCE`), so all quadratures co-measure. The substrate supplies the angle; it does not supply the **incompatibility** that would make choosing one a genuine measurement. That incompatibility is the injected measurement map `M` (a 6th-postulate-class input), not a substrate property.

---

## 1 · The question

A QM "measurement angle" is a choice of which basis/direction to project onto; the canonical phase-space version is the **quadrature** `x_θ = q cosθ + p sinθ` (homodyne angle θ). The substrate is known to provide an *arena* for such an angle — the transverse 2-plane `ψ = J_x + iJ_y` left after the Gauss constraint removes one flux component ([`DERIV_QM_FROM_LATTICE.md` §"Hermitian inner product"](../03_derivations/quantum_mechanics/DERIV_QM_FROM_LATTICE.md)) — and a single *fixed preferred basis*, the Existence Filter `E(x) = Re(x)` = manifestation onto `{−1,0,+1}` ([`FOUND_THE_EXISTENCE_FILTER.md`](FOUND_THE_EXISTENCE_FILTER.md), [`DERIV_COLLAPSE_MECHANISM.md`](DERIV_COLLAPSE_MECHANISM.md)). The open, checkable question this doc settles: does the substrate carry a *native dynamical* angle — does some phase actually wind under the bare dynamics — or is "orientation" purely an observer-imposed, frozen label?

---

## 2 · Three candidates and their priors

| # | Candidate angle | Prior |
|---|---|---|
| 1 | **Transverse spatial** `arg(J_x + iJ_y)` | **Frozen.** The 18-pt scalar Laplacian evolves Cartesian components independently, and Gauss touches only the longitudinal part; a transverse mode has `div J = 0`, so Gauss is a no-op and `J_x` can never grow. |
| 2 | **Symplectic** `arg(q + ip)`, `q` = modal amplitude, `p` = modal velocity | **Winds at `ω(k)`.** This is the `{q,p}` simple-harmonic-oscillator phase — *expected*, the substrate's native angle. Verified self-consistently against the single-tick eigenvalue `ω` so no external dispersion formula is trusted. |
| 3 | **Dual-substrate L/R relative phase** | **Inert mirror.** With `weak_transmutation` OFF, L and R are independent identical copies of the same wave equation (cf. the imposed — not emergent — parity split, FTD-0248). |

The symplectic-winding prior is "obvious" (it is just the SHO), so confirming it is **not itself a result**; the informative outcomes are *deviations*. None occurred — see §3.

---

## 3 · Probe and results

Engine probe `test_substrate_angle_probe.cpp`: `L=32`, modes `n ∈ {1,2,4}`, 300 ticks, CPU, bare wave + Gauss. It is **read-only on the physics phases** (samples voxel `flux`/`wave_vel` per tick; alters no tick logic), so the golden gate is unchanged — verified: `0xc13713f0e11a96da` @ L=17 before and after. `ω` is measured two independent ways: the single-tick eigenvalue (`ω² = |wave_vel_after / J_before|` from rest, the `campaign_dispersion.cpp` method) and the multi-tick winding of the symplectic phase; agreement is the self-consistency check.

| mode | `ω_eig` | `ω_wind` | rel. diff | net winding | max&#124;J_x&#124;, &#124;J_z&#124; |
|---|---|---|---|---|---|
| n=1 | 0.113180 | 0.113295 | 0.10 % | 33.99 rad | ≈ 1.6×10⁻¹⁶ |
| n=2 | 0.225271 | 0.226101 | 0.37 % | 67.83 rad | ≈ 1.6×10⁻¹⁶ |
| n=4 | 0.441885 | 0.446237 | 0.98 % | 133.87 rad | ≈ 1.5×10⁻¹⁶ |

L/R: `max|J_L,y − J_R,y| = 0.0` exactly over the run. All three priors confirmed (10/10 checks PASS). The `ω` values also reproduce `ω = 2·C_WAVE·|sin(k/2)|` with `C_WAVE = 1/√3`, and the rel.diff *growing with mode* is the leapfrog `O((ωΔt)²)` dispersion error — a physical signature, not noise. `[MEASURED]`

---

## 4 · What plays the role of the measurement angle

The substrate's **only** native dynamical angle is the symplectic oscillation phase. The transverse spatial orientation — the very plane QM packages as the complex amplitude — is dynamically inert: the substrate never rotates within it, so a spatial "measurement direction" there is a pure observer-set label, not something the dynamics produces. `[MEASURED]`

The genuine identification: rotating `(q,p)` by `θ` in phase space *is* the quadrature `x_θ = q cosθ + p sinθ`, and the symplectic clock rotates `(q,p)` at rate `ω`. Hence **"letting the substrate's clock run for time `t`" = "rotating to quadrature angle `θ = ωt`."** The `(q,p)`-rotation ↔ quadrature correspondence is an exact structural identity (not a numerical coincidence, so it is **not** a look-elsewhere/apophenia artifact); the identification of it with "the measurement angle" is interpretive. `[SELECTION]`

So: the substrate **has** the quadrature angle and can even sweep to any value of it by waiting. What it lacks is that different quadrature angles **co-measure** instead of being complementary — `{q,p} ≠ 0` (Poisson; the winding in §3 is that nonzero bracket made visible) but `[q,p] = 0` (observable commutator; `q` and `p` carry joint definite values every tick), per `THEOREM_COMMUTATIVITY_INDEPENDENCE` ([../10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md](../10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md)) §3. The angle is substrate-native; its **incompatibility** — the property that makes choosing a quadrature a genuine *measurement* choice — is the injected `M`. `[THEOREM for commutativity; BOUNDARY for the rest]`

---

## 5 · Separation from the α-readout boundary (W-CRIT-2)

A natural follow-on hypothesis: is this native symplectic angle the hidden source of the *rotation structure* in the unforced α-readout operator assembly `W` (W-CRIT-2, [`../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`](../07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md), FTD-0242)? **No — clean separation.** `[BOUNDARY]`

Concrete reason: the symplectic angle is a property of the **linear wave dynamics**, parametrised entirely by `C_WAVE = 1/√3` (the CFL speed) and `k`; it carries **zero `G*` content** (`ω(k) = 2·C_WAVE·|sin(k/2)|` contains no `G*`). The W-CRIT-2 assembly `(Tr, Det) = (16G*², 16G*³)` is **number-theoretic** (Watson integral, ζ-determinant ratio, elliptic-curve `|Aut|²`), parametrised entirely by `G*`, with **zero dynamical-phase content**. Disjoint parameter sets ⇒ knowing the substrate carries a commutative quadrature clock places no constraint on whether the readout determinant's odd-`G*` exponent is 1 vs 0/2/3. The two unforced/structural facts are independent. This **closes the "symplectic phase → readout rotation" route before it is walked**, and does **not** re-tread the already-closed `(Tr,Det)` routes (FTD-0204/0205/0233/0242). `[OBSERVATION / SELECTION — a reasoned separation, not a machine-checked no-go]`

---

## 6 · Epistemic ledger

- The dynamical facts (transverse frozen; symplectic winds at `ω(k)`; L/R mirror): **`[MEASURED]`**, engine-canonical, golden-gate-safe.
- `{q,p}` Poisson-nonzero but observable-commutator-zero: **`[THEOREM]`** (inherited, `THEOREM_COMMUTATIVITY_INDEPENDENCE`).
- Symplectic phase = classical quadrature angle: exact as mathematics; its identification with "the measurement angle" is **`[SELECTION]`**.
- Independence of the native angle from the α-readout `W`: **`[BOUNDARY]`** (reasoned, not machine-checked).
- **Nothing is promoted.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; α is not derived here.

---

## 7 · What this is NOT (red-zone guardrails)

- **Not** a derivation of measurement incompatibility, complementarity, or non-commutativity — those are provably injected (`THEOREM_COMMUTATIVITY_INDEPENDENCE` [THEOREM]; FTD-0225/0226/0228 [CLOSED NEGATIVE]).
- **Not** a Born-rule route — the manifestation statistic is not `|ψ|²` here (FTD-0187 [OPEN]; FTD-0200 threshold-crossing→Born [CLOSED NEGATIVE], [`EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md`](EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md)).
- **Not** a recovery of the Bell `2√2` excess — the substrate is `S ≤ 2` (`AUDIT_BELL_ANALYSIS`); the winding angle does not change that.
- A winding symplectic phase being a real angle is exactly the trap FTD-0228 (apophenia, "symplectic budget symmetry," [CLOSED NEGATIVE]) warns against treating as a measurement structure. The discipline here is to label it as the commutative shadow it is.
