# SCOPE — Discrete-Feynman-Integral Program (lattice periods, genus, and the two-loop question)

**Tag:** `[SCOPE]` — a scoped research-program statement. Introduces no theorem, promotes no tag, forces nothing. It organizes existing FTD one-loop machinery and states the genuinely open computation (the two-loop BCC period).
**LEDGER:** maintenance-log line; content rides on FTD-0002 (Watson/G* identity), the CM-curve table (`DERIV_WATSON_GSTAR_IDENTITY.md` Part VII, `[THEOREM]`), and FTD-0013 (`x₊=1/α`, `[SMC]`).
**Audience:** anyone extending FTD's lattice-loop calculus, or asking "what would a discrete Feynman calculus buy, and what can it not do?"

---

## §0 — One-line scope

The one-loop lattice Feynman integral (the return Green's function at the origin) is **already** computed and its period **already** classified by CM point. The program's job is the **two-loop** period — *does the BCC regulator stay lemniscatic (ℤ[i], Γ(1/4)) as loops climb, or does it climb like the continuum sunrise (→ Γ(1/3))?* — plus the vocabulary bridge to the elliptic-Feynman-integral (Bessel-moment) literature. It **cannot** force α (MC-T4.3), and it is a **deterministic resolvent series, not a path integral** (the confinement obstruction).

---

## §1 — What already exists (build on, do not duplicate)

| Object | Status | Source |
|---|---|---|
| BCC Green's function `W₃ = G*²/(2π) = Γ(1/4)⁴/(4π³)` as the **one-loop lattice self-energy/tadpole** | `[THEOREM]` (identity); one-loop framing stated | `DERIV_WATSON_GSTAR_IDENTITY.md` §1.4 |
| **CM-point table**: BCC→ℤ[i]/Γ(1/4) (j=1728), FCC→ℤ[ω]/Γ(1/3) (j=0), SC→disc −24/Γ(k/24) — "**the curve is forced by the lattice axiom**" | `[THEOREM]` | ibid Part VII §7.1–7.4 |
| Geometric-series factorization `W₃ = Σ[C(2m,m)/4^m]³` (why BCC is multiplicative) | `[THEOREM]` | `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` |
| genus-0 (π) vs genus-1 (Γ(1/4)) period language | exposition | `FTD_Discrete_Continuous_Bridge.tex` §"Genus-0 Projection" |
| Higher-D Watson `W^(D) = _DF_{D-1}(…)`, only D=3 gives a clean Γ-ratio | `[THEOREM]` | `EXPLR_HIGHER_DIM_WATSON.md` |
| One-loop period map across BCC/FCC/SC (computational **confirmation** of the §7.4 theorem) | script, 5/5 | `scripts/exploration/lattice_period_map.py` (this session) |
| Numerical φ³ loop machinery: `c1=9/47` one-loop tadpole **0.8%** `[DERIVED]`; `c2=5/64` ~83% from sunset `[PARAMETRIC]`; `c3=4/141` `[MOTIVATED]` (not a computed integral) | mixed | `DERIV_ONE_LOOP_LATTICE_ALPHA.md`, `gauge_loops.py`, `compute_c2.py`, `verify_two_loop.py` |

**Load-bearing caveat (do not blow past it).** `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (Remark) states `W₃^FTD` is **not** to be identified as the literal standard lattice self-energy — "the role of G\* is *algebraic*, not a direct identification of the lattice Green's function." The program therefore studies **periods** (rigorous number theory) and treats any literal-QFT-self-energy reading as `[CONJECTURE]`, not given.

---

## §2 — Thesis (already-established part)

The one-loop lattice Feynman integral's period is pinned to a CM point **by the lattice structure function**, and the body-centered (multiplicative triple-cosine) choice pins it to the lemniscatic point ℤ[i] = Γ(1/4) = FTD's G\*. This is **not** a generic "discreteness → genus-1" fact — SC gives disc −24 and FCC gives Γ(1/3). So **G\* is a BCC selection at one loop**, confirmed computationally (`lattice_period_map.py`: BCC 0.0065%, SC 0.0006%, FCC → 1.34469 ≈ literature ℤ[ω]). This restates `DERIV_WATSON_GSTAR_IDENTITY` §7.4 in Feynman-integral language; it is `[THEOREM]`-backed, not new.

---

## §3 — The genuine white space (what the program adds)

1. **The two-loop BCC period is uncomputed as a closed form.** The only two-loop lattice work is a numerical scalar sunset (`I_sunset = 0.1168` on 32³) on the *simple-cubic* propagator `1/(k̂²+m²)`, never the BCC (multiplicative) propagator, and never as a closed-form period; `verify_two_loop.py` (2L-7) flags the explicit BZ² integration `[OPEN]`. **Update:** `lattice_two_loop_bcc.py` (this session) now computes the BCC sunset *value* `I(μ²)=Σ_x G_BCC(x)³` (L-converged), but shows the period-carrying **finite part is fit-model-unstable at double precision** (§4 M2) — the closed-form period remains `[OPEN]` and needs arbitrary precision.
2. **The elliptic-Feynman-integral bridge is absent.** "Bessel moment," "elliptic polylogarithm," "sunrise/banana," "Calabi–Yau motive" appear **nowhere** in the corpus. FTD's lattice sums *are* Bessel moments (Broadhurst–Bailey–Borwein–Glasser); naming that ties them to a mature literature.
3. **Feynman-integral geometry of the genus outcomes is unframed.** The CM/genus content lives only in the number-theory register; connecting it to loop-order geometry (genus-1 sunrise, K3/banana) is unbuilt.

---

## §4 — Milestones

- **M1 — one-loop period map. `[DONE]`** — `lattice_period_map.py`. Computationally confirms the §7.4 `[THEOREM]`: BCC→ℤ[i], FCC→ℤ[ω], SC→disc −24. Contribution: a runnable cross-check + the Feynman-integral framing. **Novelty: confirmation, not discovery.**

- **M2 — the two-loop BCC period. `[OPEN]` — attempted; precision-limited.** Compute the two-loop sunrise/sunset on the BCC propagator and identify its period. **Method:** coordinate-space form `Σ_x G_BCC(x)³` (sunset at external p=0), with `G_BCC(x)` the FFT of `1/(1−σ_BCC(k)+μ²)` on an `L³` BCC lattice; IR-regulate with a small mass `μ²`; take `μ²→0` and `L→∞`; PSLQ the extrapolated value against the basis `{Γ(1/4)-powers/π^a, Γ(1/3)-powers/π^b, weight-3 modular L-values, ζ(3), π^n}`. **Falsifier:** if the period lands on the ℤ[i]/Γ(1/4) family → the BCC regulator is *lemniscatic at two loops* (a strong new `[THEOREM]`-candidate); if it lands on Γ(1/3)/modular → even BCC climbs, and G\*'s lemniscatic character is a one-loop-only fact.

  **Attempt (this session): `scripts/exploration/lattice_two_loop_bcc.py`** (cupy/GPU backend, numpy/CPU fallback). The sunset structure is `I(μ²) = −A·log(μ²) + B + O(μ²log μ²)` — the massless 3D tail `G(x)~c/|x|` makes `G(x)³~1/|x|³` IR-log-divergent, so the period/genus content is the **finite part `B`**, not the raw value. Result: the **raw** `I(μ²)` is L-converged (max |I₁₂₈−I₉₆| = 6.7×10⁻⁴ across the μ² window), but the **finite part `B` is fit-model-unstable** — 52% swing between a 2-parameter log fit (B≈0.29) and a 3-parameter `+C·μ²` fit (B≈0.61) at L=128. **The instability is a μ²-window (lever-arm) artifact, not a hard precision floor:** the swing shrinks monotonically as the box grows — measured **52% (L=128) → 38% (L=256) → 30% (L=384)** — with the 2-param and 3-param `B` converging from both sides. So the μ²→0 log-subtraction, not the FFT, is the ceiling, and it is **GPU-addressable** for the *coarse* question: a large-L run (`FTD_BCC_BIG=1`, L≈768, μ²≈10⁻⁴ on an RTX 5090) can plausibly pin `B` to ~1% and decide the **Γ(1/4) vs Γ(1/3) family** (the M2 falsifier). The *full closed-form* period ID is a separate, harder job that still needs arbitrary precision (20–30-digit `mpmath`/`Arb` propagator + Richardson tower). Verdict unchanged: `[OPEN]`; promotes no tag.

  **Feasibility:** the coarse genus discrimination is a large-L double-precision GPU run (the script's `FTD_BCC_BIG=1` mode, cupy on the RTX 5090 per CLAUDE.md hardware notes); the full PSLQ closed form needs arbitrary precision. The double-precision FFT attempt above measured *why* — the finite-part swing and its lever-arm scaling — and hands the large-L run to the GPU box.

- **M3 — the deterministic-resolvent framing. `[OPEN]`.** Recast the loop expansion as the resolvent series `(1−zM)^{−1}=Σ zⁿMⁿ` of the **deterministic** lattice operator — no `∫e^{iS}` — making explicit that FTD's discrete Feynman calculus is a return-amplitude/walk series (which is why it sidesteps the confinement obstruction that blocks `Z=∫dU e^{−S}`). Connects to §11 (`[CLOSED, structural obstruction]`).

---

## §5 — Honest ceiling (the guards)

1. **It cannot force α.** The coupling multiplying the diagrams is MC-T4.3 (`[FOUNDATIONAL OBSTRUCTION]`). No re-summation of parameter-free lattice periods produces 1/137; the Phase-G result already showed the emergent Coulomb is the parameter-free Green's function, *not* α. Keep `x₊=1/α` at `[SMC]`.
2. **It is not a path integral.** FTD's substrate is deterministic — no `∫e^{iS}` sum-over-histories. The calculus is the deterministic resolvent series (M3). Do not import stochastic-path-integral claims.
3. **Respect the self-energy caveat.** `W₃^FTD` is algebraic, *not* asserted as the literal standard lattice self-energy (`DERIV_CONTINUUM_LIMIT_QED` Remark). The program's rigorous output is **period identities** (`[THEOREM]`); any literal-QFT reading is `[CONJECTURE]`.
4. **Do not inherit c2/c3's status.** `c1` is `[DERIVED]` (one-loop, 0.8%); `c2` is `[PARAMETRIC]` (83% from sunset); `c3+` are `[MOTIVATED]` rational fits to framework integers, not computed integrals. The program computes *periods*, a different (cleaner) object; it must not launder the parametric coefficients into "derived."
5. **Separate the registers.** Period identities (Watson/Bessel-moment number theory) are `[THEOREM]`-grade; physics identifications (that a given period *is* a coupling/observable) are `[CONJECTURE]`. FTD has over-claimed at exactly this seam before.

---

## §6 — Cross-references

`DERIV_WATSON_GSTAR_IDENTITY.md` (one-loop `[THEOREM]` + CM/genus table Part VII); `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (why BCC is multiplicative); `EXPLR_HIGHER_DIM_WATSON.md` (higher-D `[THEOREM]`); `DERIV_ONE_LOOP_LATTICE_ALPHA.md` + `verify_two_loop.py` (numerical loop machinery, the `[OPEN]` BZ²); `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (the `[SMC]` continuum reading + the self-energy caveat); `FTD_Discrete_Continuous_Bridge.tex` §"Genus-0 Projection"; `scripts/exploration/lattice_period_map.py` (M1); `scripts/exploration/lattice_two_loop_bcc.py` (M2 attempt — value computed, finite part precision-limited); `SPEC_OPEN_MATH_BY_SECTOR.md` (MC-T4.3, the α obstruction this program cannot close). External anchor for M2: Broadhurst / Bailey–Borwein–Broadhurst–Glasser Bessel-moment program; Zucker "70+ Years of the Watson Integrals."
