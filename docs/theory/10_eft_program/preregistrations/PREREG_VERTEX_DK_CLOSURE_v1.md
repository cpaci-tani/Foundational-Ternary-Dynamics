# PREREG — Vertex Program v1: Dirac–Kähler Evolution + Noise-Controlled Bivector Closure

**Tag:** [PRE-REGISTRATION] — expectations committed BEFORE any measurement runs
**Reserved LEDGER ids:** FTD-0379 (M1, DK evolution), FTD-0380 (M2, noise-controlled closure)
**Date locked:** 2026-07-10
**Program:** the vertex program — §7-bivector → §7-dirac critical path (`SPEC_OPEN_MATH_BY_SECTOR.md` §2), Branch-A/Branch-B fermion accounting
**Companions:**
- [`DERIV_DIRAC_KAHLER_IDENTIFICATION.md`](../../09_mathematical/algebra/DERIV_DIRAC_KAHLER_IDENTIFICATION.md) (FTD-0089) — M1 executes its §A1.5 verification path, never before run
- [`DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md`](../../09_mathematical/algebra/DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md) (FTD-0086/0087/0088) — M2 executes the noise-controlled re-test named in its §3.5.3 as "most consequential follow-up," never before run
- [`DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md`](../../09_mathematical/number_theory/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md) (FTD-0073) — the no-go both measurements must respect (both readouts are non-site-local bilinear/trilinear, outside its scope)
- [`SPEC_ALPHA_READOUT_CONTRACT.md`](../../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md) — exclusion rules inherited (§2 below)

> **Pre-registration.** This document is committed to the repository *before either measurement runs.* Expectations, pass/fail criteria, outcome taxonomy, and priors are specified up-front. If a measurement produces a surprising number, this spec is **not** edited to match — the result is reported honestly in the analysis doc and the theory must explain the surprise. Compile checks are permitted before the lock commit; **no measurement output may be observed before the lock commit.**

---

## 0 · What is being decided

The vertex program asks whether FTD's substrate carries a native matter (fermionic) sector, or whether fermion content is a Branch-B selection with an imposed coupling. Prior work leaves exactly two decisive, fully-scoped, unexecuted measurements:

- **M1** — the Dirac–Kähler identification (FTD-0089 A1) is currently a *static-skeleton* match: FTD's measured 4-grade structure (S, Vᵢ, Pᵢⱼ, T) has the component structure of a DK field, but whether the engine's *evolution* satisfies the discrete DK equation has never been tested. If it does, the substrate carries a dynamical DK (fermionic) mode — the matter half of the vertex exists natively. If it does not, the DK identification is kinematic only.
- **M2** — the plaquette-bivector su(2) closure failed at 4-injection scale (FTD-0087 Part D), and FTD-0088 reinterpreted the failure as dynamical noise, prescribing the decisive discriminator: re-run with time-averaged readouts, larger L, lower A. If closure recovers under noise control, the noise hypothesis is confirmed and the bivector subalgebra closes; if it is robust to all controls, the algebra is fundamentally approximate.

Neither measurement touches α, x₊, G*, or any imported physical constant. Both are dimensionless algebraic-structure measurements on the flux field. **Ramification grade: 0** (unit territory throughout; no δ square-class step anywhere; any future α-claim built on these results must separately pass `SPEC_ALPHA_READOUT_CONTRACT.md` §2.5).

## 1 · Epistemic rules binding this campaign

1. **No numerical fishing.** All thresholds below are fixed now. No post-hoc threshold adjustment.
2. **No relabeled insertions.** A positive M1 does not make FTD "derive the Dirac equation" — it makes the DK evolution a **[MEASURED]** property of the engine at the tested protocol, upgradable only per the impact table in §5.
3. **Either branch is a valid outcome.** Negative results close branches honestly and are preserved as provenance.
4. **Hard exclusions** (inherited from `SPEC_ALPHA_READOUT_CONTRACT.md` §3): no physical α/CODATA input; no free parameter whose only role is tuning toward a target; no reuse of closed-negative mechanism classes (both measurements use non-site-local readouts, outside FTD-0073's scope by construction).

## 2 · M1 — Dirac–Kähler evolution test (FTD-0379)

### 2.1 Fields

Local grade fields built from the flux field J on the full periodic L³ lattice (FTD-0088 observables, localized):

- φ⁰(x) = S(x) = |J(x)|²  (0-form, sites)
- φ¹ᵢ(x) = Vᵢ(x) = Jᵢ(x)  (1-form, edges)
- φ²ᵢⱼ(x) = Pᵢⱼ(x) = Jᵢ(x)Jⱼ(x+êᵢ) − Jᵢ(x+êⱼ)Jⱼ(x), i<j  (2-form, plaquettes)
- φ³(x) = T(x) = Jₓ(x)J_y(x)J_z(x)  (3-form, cubes)

### 2.2 Discrete operators (exact conventions)

Forward/backward differences with periodic wrap: (∇⁺ᵢf)(x) = f(x+êᵢ) − f(x), (∇⁻ᵢf)(x) = f(x) − f(x−êᵢ).

Exterior derivative d (ε-signed, correcting the sign gloss in FTD-0089 §A1.3 for the (x,z) plaquette and the 3-form assembly):

- (dφ⁰)ᵢ = ∇⁺ᵢφ⁰
- (dφ¹)ᵢⱼ = ∇⁺ᵢφ¹ⱼ − ∇⁺ⱼφ¹ᵢ  (i<j)
- (dφ²)ₓᵧ_z = ∇⁺ₓP_yz − ∇⁺ᵧP_xz + ∇⁺_zP_xy

Codifferential δ (formal adjoint under the site-sum inner product; FTD-0089's divergence convention):

- (δφ¹) = Σᵢ ∇⁻ᵢφ¹ᵢ
- (δφ²)ᵢ = Σⱼ ∇⁻ⱼφ²ᵢⱼ with φ²ⱼᵢ = −φ²ᵢⱼ
- (δφ³)ₓᵧ = ∇⁻_zT, (δφ³)ₓ_z = −∇⁻ᵧT, (δφ³)ᵧ_z = ∇⁻ₓT

**Built-in self-checks (must pass before any dynamics run, else the harness is invalid):** on pseudorandom fields, the signed adjoint identities that the conventions above actually satisfy — ⟨d⁰f, W⟩ = −⟨f, δ¹W⟩, ⟨d¹V, Q⟩ = +⟨V, δ²Q⟩, ⟨d²P, u⟩ = −⟨P, δ³u⟩ (the sign alternation is a consequence of the FTD-0089 divergence convention absorbing the antisymmetry bookkeeping unevenly across grades; derived by hand 2026-07-10, asserted numerically to relative 10⁻¹²) — plus nilpotency d¹∘d⁰ = 0, d²∘d¹ = 0, δ¹∘δ² = 0, δ²∘δ³ = 0 to the same tolerance.

The eight DK component equations tested, (d−δ)Φ − mΦ grade by grade:

```
eq0:      −(∇⁻ₓVₓ + ∇⁻ᵧVᵧ + ∇⁻_zV_z)                = m·S
eq1x:     ∇⁺ₓS − (∇⁻ᵧP_xy + ∇⁻_zP_xz)               = m·Vₓ
eq1y:     ∇⁺ᵧS − (−∇⁻ₓP_xy + ∇⁻_zP_yz)              = m·Vᵧ
eq1z:     ∇⁺_zS − (−∇⁻ₓP_xz − ∇⁻ᵧP_yz)              = m·V_z
eq2xy:    ∇⁺ₓVᵧ − ∇⁺ᵧVₓ − ∇⁻_zT                     = m·P_xy
eq2xz:    ∇⁺ₓV_z − ∇⁺_zVₓ + ∇⁻ᵧT                    = m·P_xz
eq2yz:    ∇⁺ᵧV_z − ∇⁺_zVᵧ − ∇⁻ₓT                    = m·P_yz
eq3:      ∇⁺ₓP_yz − ∇⁺ᵧP_xz + ∇⁺_zP_xy              = m·T
```

### 2.3 Dynamical test

**Primary form (first-order, DK):** ∂ₜΦ = (d−δ)Φ − mΦ, midpoint discretization — LHS = Φ(t+1) − Φ(t); RHS evaluated on Φ̄ = (Φ(t)+Φ(t+1))/2. Single scalar m fitted by least squares jointly over all 8 component equations, all lattice sites, all fit-window ticks, all seeds of a configuration. Report m*, per-grade residuals ρₖ (k = 0..3) and joint ρ_all, where ρ = ‖LHS − RHS(m*)‖₂ / max(‖LHS‖₂, ε_floor). Also report residuals at m = 0 (the massless A1.5 form).

**Comparator form (second-order, KG):** ∂ₜ²φ⁽ᵏ⁾ = c²Δφ⁽ᵏ⁾ − μ²φ⁽ᵏ⁾ per grade, (c², μ²) fitted per grade by least squares over the same window (Δ = 7-point lattice Laplacian). Report ρ_KG per grade. The discriminator DIRAC-FORM vs KG-FORM per grade: whichever residual is smaller by ≥ 0.10 absolute; else TIE.

### 2.4 Protocol

- L = 8, A = 10, the 8 FTD-0088 seeds (0xF4170517…0xF417051E).
- Initialization: 2-injection (χ_f on axis f, tick, χ_g on axis g, tick), all three off-diagonal pairs (x,y), (x,z), (y,z).
- Then evolve 30 further ticks recording all four grade fields per tick. Fit window: ticks 4…28 after the second injection (transient excluded, fixed now).
- **CONFIG-N (primary):** the FTD-0088 full non-local toggle set (wave_propagation, gauss_projection, genesis, movement, forces, emergent_forces, pair_production, weak_transmutation, exchange_force, strong_force, triad_binding, color_forces).
- **CONFIG-M (control):** minimal linear set (wave_propagation, gauss_projection only). Null control: pure second-order wave dynamics should favor KG-FORM; if DK-FORM wins here, the harness is suspect.

Known toggle-set drift, disclosed now: FTD-0087's closure test used `dual_substrate` where FTD-0088's multigrade test used `exchange_force` + `strong_force`. M1 replicates FTD-0088's set (its identification is the one under test); M2 replicates FTD-0087's set (its failure is the one under re-test). This is a pre-existing inconsistency between the two predecessor tests, reported, not silently harmonized.

### 2.5 Pre-registered outcomes (M1)

| Outcome | Criterion (CONFIG-N, aggregated over pairs & seeds) | Consequence |
|---|---|---|
| **DK-DYNAMICAL** | ρ_all < 0.15 AND every ρₖ < 0.25 AND fitted m* stable across the three injection pairs (relative spread < 30%) | DK identification gains dynamical content: [MEASURED — dynamical DK at tested protocol]. §7-dirac reopens as Branch-A candidate. Strongest possible M1 result. |
| **DK-PARTIAL** | At least one grade equation with ρₖ < 0.15 while ρ_all ≥ 0.15 | DK holds on a subsector (e.g. continuity). Report which; the surviving subsector becomes the vertex program's native foothold. |
| **DK-STATIC-ONLY** | All ρₖ ≥ 0.50, or fit degenerate (denominator floor triggered on all grades) | FTD-0089 A1 confirmed as *kinematic* identification only. The matter sector is composite/Branch-B; the vertex program proceeds on the imposed-coupling path. |
| **UNDETERMINED** | Anything between the bands above | Report measured values; no tag movement; design v2 with the measured noise floor. |

**Priors (stated now):** DK-DYNAMICAL 10%, DK-PARTIAL 35%, DK-STATIC-ONLY 45%, UNDETERMINED 10%. Rationale: the flux field obeys second-order wave dynamics plus per-tick Gauss projection; a first-order grade-coupled evolution emerging from that is not generically expected (DK ⇒ each component KG, but not conversely). Note eq0 is *not* the engine's charge conservation — S = |J|² is flux-energy density, not charge — so even DK-PARTIAL via eq0 is a substantive finding, not a tautology.

**KG comparator expectation:** KG-FORM should win on grade 1 (V = J is literally the wave field). Pre-registered as the harness sanity anchor: if grade 1 in CONFIG-M is not KG-FORM or TIE, the harness is suspect and the run is re-examined before any interpretation.

## 3 · M2 — Noise-controlled bivector closure re-test (FTD-0380)

### 3.1 Construction

Exact replication of FTD-0087 Part D: for each cyclic triple ([B_xy,B_yz]→B_xz, [B_xz,B_xy]→B_yz, [B_yz,B_xz]→B_xy), the 8 signed 4-injection sequences with prefactor 1/4, WH weight-1 modes, 1 tick after each injection, plaquette readout on the 2³ block, FTD-0087 toggle set (including `dual_substrate`). Same sequence-sign table as `test_bivector_closure.cpp`.

### 3.2 Noise-control sweep (the FTD-0088 §3.5.3 prescription, executed)

Full factorial over:
- **L ∈ {8, 16, 32}** (boundary-effect control)
- **A ∈ {1, 3, 10}** (non-linearity control; bilinears scale ~A², so all criteria below are ratio-based, never absolute)
- **Readout ∈ {instant, time-averaged}** (instant = read at sequence end, as FTD-0087; time-averaged = mean of readouts at sequence end and after each of 4 further ticks)
- **Seeds: 16** (the 8 FTD-0087 seeds 0xF3170517…1E plus 0xF3170617…1E)

= 18 cells × 3 triples × 8 sequences × 16 seeds.

### 3.3 Pre-registered criteria

Per cell, per triple: signal = |mean over seeds on expected plaquette|; off = max |mean| on the other two plaquettes; sem = stderr of the expected-plaquette values across seeds.

- **Triple CLOSES** iff signal > 2·off AND signal > 2·sem.
- **Cell closes** iff 3/3 triples close.
- **Concentration ratio** per triple: r = signal / max(off, sem) — the monotonicity statistic.

| Outcome | Criterion | Consequence |
|---|---|---|
| **CLOSURE-RECOVERED** | ≥ 1 cell closes 3/3, and that cell differs from the FTD-0087 baseline (L=8, A=10, instant) in the noise-controlled direction (lower A, higher L, or time-averaged) | FTD-0088's noise hypothesis CONFIRMED. su(2) bivector closure [MEASURED] at the identified regime. §7-bivector advances: the substrate carries a closed bivector subalgebra. |
| **CLOSURE-TREND** | No cell closes 3/3, but r improves monotonically with decreasing A at fixed (L, readout) for ≥ 2 of 3 triples in ≥ 2/3 of columns (Spearman ρ > 0, all three A points ordered) — or equivalently with increasing L | Approximate algebra with controllable deviation (FTD-0087 Path 2). Quantify the deviation scaling; closure remains open with a measured extrapolation. |
| **CLOSURE-ROBUST-FAIL** | No cell closes AND no monotone improvement in either control direction | The 4-injection failure is structural, not noise. FTD-0088's reinterpretation is REFUTED; the bivector subalgebra is fundamentally non-closing at accessible protocols. Branch-B selection becomes the settled accounting for the vertex; the matching signature (FTD-0086) stands as a constraint on selections, nothing more. |
| **UNDETERMINED** | Mixed pattern fitting none of the above | Report per-cell table in full; no tag movement. |

**Priors (stated now):** CLOSURE-RECOVERED 25%, CLOSURE-TREND 35%, CLOSURE-ROBUST-FAIL 30%, UNDETERMINED 10%.

**Baseline replication check:** the (L=8, A=10, instant) cell restricted to the 8 original seeds must qualitatively reproduce FTD-0087 Part D (no closure). If it does not, the engine has drifted since FTD-0087 and the whole comparison is re-based before interpretation (drift reported, not hidden).

## 4 · Artifacts and locks

| Item | Path | SHA256 |
|---|---|---|
| M1 runner | `engine/tests/test_dk_evolution.cpp` | `cb3083d4180127d76b38cd504a8107aea44c874b04592588a0e5fbac81696f96` |
| M2 runner | `engine/tests/test_bivector_closure_v2.cpp` | `c6c74322dbf2f46fdfcefe9252391feb13364dc37e2dc1bb0ae028c0f9f8637a` |
| This prereg | `docs/theory/10_eft_program/preregistrations/PREREG_VERTEX_DK_CLOSURE_v1.md` | (locked by the commit) |

Output: both runners print full per-cell / per-grade tables and a verdict line against the criteria above; console output captured to `engine/results/vertex_dk_closure_2026-07-10/` (gitignored per convention; manifest row added to `REF_PREREGISTER_MANIFEST.md`). CTest names: `dk_evolution`, `bivector_closure_v2` (LABELS native eft vertex; long TIMEOUT — these are campaign runners, not smoke tests).

## 5 · Tag-impact table (what each outcome can and cannot move)

- No outcome of M1 or M2 moves x₊=1/α [SMC], FC-W [AXIOM], MC-T4.3 [FOUNDATIONAL OBSTRUCTION], or any spine tag. These are matter-sector structure measurements, not α-readouts.
- DK-DYNAMICAL / CLOSURE-RECOVERED yield **[MEASURED]** rows at the tested protocols — not [THEOREM], not [DERIVED]. Any Branch-A "native fermion" claim would additionally require the analytic derivation chain and survives only at the tag that chain earns.
- Negative outcomes yield [CLOSED NEGATIVE] rows for the specific hypotheses (dynamical DK at this protocol; noise-recoverable closure), preserved as provenance with the runner as evidence.
- The FTD-0073 mode-erasure theorem is untouched by any outcome (both probes are outside its scope by construction).

## 6 · Declared non-goals

M1/M2 do not attempt: doubling analysis, spinor-bundle construction, mass-hierarchy content (FTD-0089 A2 no-go stands), any α or coupling readout, or any continuum limit. Those are downstream of the branch these two measurements decide.
