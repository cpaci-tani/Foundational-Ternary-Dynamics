# Pre-Registration · Phase II — Wilson-Dirac Matter Sector + g-2 Measurement

**Status:** PRE-REGISTRATION (committed BEFORE any measurement; hash-locked via git tag `preregister-phase-ii-wilson-dirac-g2-v1`).
**Date:** 2026-05-03
**Pre-registration discipline:** follows FTD-0097 / FTD-0121 / FTD-0123 / FTD-0124 / FTD-0125 methodology. Phase II is a multi-phase campaign (~4-6 weeks total); each sub-phase has its own internal pre-registration milestones, all rooted in this document.

---

## 0 · Why this campaign

Phase I (FTD-0125) verified that FTD's wave-propagation channel realizes the master-quadratic-derived coupling `g_FTD² = 1/x_+` self-consistently. That answered: **does the inserted coupling propagate?** Yes.

The next question — **does FTD with this coupling reproduce QED phenomenology when matter is added?** — requires a matter sector. FTD's spontaneous matter emergence produces colored quarks, not electrons (FTD-0076), and Clifford structure for Dirac fermions is closed-negative on FTD's native fields (FTD-0073 mode-erasure no-go). So matter must be **inserted by Branch-B selection**: standard Wilson-Dirac fermions on the lattice, with the coupling fixed by `g_FTD = √(1/x_+)`.

The CODATA-grade test of this insertion is the electron anomalous magnetic moment `a_e = (g − 2)/2`. QED's tree-level prediction is `a_e = α/(2π) ≈ 1.16×10⁻³` (Schwinger 1948). If FTD's lattice dynamics reproduce this with `α = g_FTD² = 1/x_+`, that's strong evidence FTD-as-QED at the relevant scale, distinct from "we inserted α and got it back." It would be a real prediction: the engine's Lorentz dynamics combined with the master-quadratic coupling produce the same `α/(2π)` Schwinger anomaly QED does.

This pre-registration **does NOT** claim to derive α from FTD axioms. It tests a different question: given FTD's coupling [DERIVED from Theorem 2] and Wilson-Dirac matter [Branch-B INSERTED], does FTD's electron anomalous magnetic moment match QED's Schwinger value?

---

## 1 · Scope and out-of-scope

**In scope:**
- Wilson-Dirac fermion action specification (Phase II.1)
- C++/CUDA implementation of the Wilson-Dirac toggle in the engine (Phase II.2)
- Single-electron-in-uniform-B-field stable configuration (Phase II.3)
- Cyclotron + spin-precession measurement (Phase II.4)
- `a_e` extraction + Schwinger comparison (Phase II.5)
- LEDGER FTD-0126 row reporting verdict

**Out of scope:**
- Higher-loop α corrections (α/π)², (α/π)³ — would require lattice loop machinery FTD does not have
- Dirac fermion derivation from FTD axioms (FTD-0073 closed-negative; this is a Branch-B insertion)
- Full positron asymmetry / CP violation
- Anomalous magnetic moment from non-perturbative QCD (only QED-sector tested)
- Resolution of MC-T4.3 (axioms→α derivation chain stays open)

---

## 2 · Phase breakdown (hash-locked)

| Phase | Title | Deliverable | Estimated effort |
|---|---|---|---|
| **II.1** | Wilson-Dirac action specification | `SPEC_WILSON_DIRAC_FTD.md` — exact lattice action, fermion-substrate coupling rule, gauge-link convention, Wilson r-parameter choice, doubler-handling note | 2–3 days theory |
| **II.2** | Engine implementation | `engine/include/ftd/wilson_dirac.h` + `engine/src/wilson_dirac.cpp` (CPU) + `engine/cuda/wilson_dirac.cu` (GPU) + new toggle `wilson_dirac` in TermToggles + integration into `phase_write` (matter source) and `phase_forces` (Lorentz coupling). CTest harness `test_wilson_dirac_smoke.cpp`. | 1–2 weeks C++/CUDA |
| **II.3** | Single-electron-in-B-field stable config | Benchmark `benchmark_dirac_electron_in_B.cpp` — initialise single electron in uniform B-field; verify cyclotron orbit closes within tolerance; energy conservation < 1% over many revolutions; lifetime exceeds the spin-precession period. | 1 week tuning |
| **II.4** | Cyclotron + spin precession measurement | Benchmark `benchmark_g_minus_2.cpp` — measure cyclotron frequency `ω_c` from position-time tracking and spin-precession frequency `ω_s` from spin-vector tracking. Output: `(ω_c, ω_s, B, t_total)` table. | 1 week |
| **II.5** | a_e extraction + Schwinger comparison | Python post-processor `scripts/proofs/proof_phase_ii_g_minus_2.py` — compute `a_e = (ω_s − ω_c)/ω_c`, compare to Schwinger `α/(2π)` with `α = 1/x_+` (FTD-native). Verdict per pre-registered outcomes (§4). | 1 week |

**Total estimate:** 4–6 weeks. Not session-tractable; each phase is its own work-block.

---

## 3 · Theoretical specification (sketch — full version in Phase II.1 deliverable)

### 3.1 Wilson-Dirac action

On the lattice with sites `n ∈ Z³`, lattice spacing `a` (= `ℓ_P` per FTD calibration), and time-step `τ` (= `√3 ℓ_P / c`), introduce a 4-component spinor `ψ(n, t)`. The Wilson-Dirac action is

$$
S_F = \sum_{n} \bar\psi(n) \left[ \sum_{\mu} \gamma^\mu \frac{\psi(n+\hat\mu) - \psi(n-\hat\mu)}{2a} - \frac{r}{2a} \sum_{\mu} \left( \psi(n+\hat\mu) - 2\psi(n) + \psi(n-\hat\mu) \right) + m \psi(n) \right]
$$

where `r` is the Wilson parameter (canonical: `r = 1`) lifting the doublers.

### 3.2 Fermion-substrate coupling

The fermion couples to the FTD flux field via the standard minimal-coupling rule:

$$
\partial_\mu \psi \to (\partial_\mu - i g_{\mathrm{FTD}} A_\mu) \psi
$$

where `A_μ` is the gauge field constructed from the FTD flux `J_i` via the projection convention of `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` and `g_FTD = √(1/x_+)` is the FTD-native coupling [DERIVED] from Phase I.

### 3.3 Magnetic field

A uniform magnetic field `B = B_0 ẑ` is implemented via a gauge-link configuration `U_x(n) = exp(i a g_FTD A_x(n))` with `A_x(n) = -B_0 n_y` (Landau gauge). Standard lattice prescription.

### 3.4 What is being inserted vs derived

- **Inserted (Branch-B selection):** the Wilson-Dirac action `S_F` (standard lattice QED matter); the gauge field `A_μ` from flux projection; the magnetic field `B_0 ẑ` as initial condition.
- **Derived from FTD spine:** the coupling `g_FTD = √(1/x_+)` (master quadratic [THEOREM] + Phase I [DERIVED]).
- **Engine-level [SELECTION]:** the choice of Wilson `r = 1`, the lattice spacing-to-physical-scale calibration `a ≡ ℓ_P`, the time-step `τ = √3 ℓ_P / c`.

---

## 4 · Pre-registered outcomes (hash-locked)

After the campaign completes, exactly one of the following will be reported in LEDGER FTD-0126:

| Outcome | Numerical criterion (committed BEFORE measurement) | LEDGER consequence |
|---|---|---|
| **A. SCHWINGER MATCH** | `\|a_e^FTD − α/(2π)\| / (α/(2π)) < 5%` | Significant positive result. FTD with master-quadratic-derived coupling reproduces QED's tree-level Schwinger anomaly. Promotes the FTD-native-coupling story from "consistency check" to "lattice-EFT-derives-Schwinger-anomaly within engine precision." Paper D candidate. |
| **B. SCHWINGER NEAR-MATCH** | `5% ≤ rel_err < 50%` | Partial match — engine reproduces order of magnitude and sign but not coefficient. Investigation: lattice-Wilson finite-volume corrections, doubler residue, gauge-link discretization. Closure path: extrapolate to L → ∞ and refine. |
| **C. SCHWINGER MISS** | `rel_err ≥ 50%` OR `g − 2` of opposite sign OR no detectable anomaly | Engine does NOT reproduce Schwinger's anomaly with the master-quadratic coupling. Possible interpretations: (i) Wilson-Dirac discretization artifacts dominant at engine precision; (ii) flux-projection gauge field not equivalent to QED A_μ; (iii) FTD-native coupling does NOT play the role of QED α at the matter sector. Outcome C would be a substantive negative result requiring careful diagnosis before drawing structural conclusions. |
| **D. INFRASTRUCTURE FAILURE** | Phase II.3 fails to produce a stable single-electron orbit | Phase II.4-II.5 cannot run. Document failure mode; redesign II.2/II.3. Not a verdict on FTD; a verdict on the implementation. |

### 4.1 Tolerance rationale

The 5% bound for outcome A is generous. At single-electron Wilson-Dirac on a finite lattice, expected sources of deviation include: Wilson-r artifacts (O(a) → O(α a m_e) ~ percent at coarse lattices), finite-volume corrections (~1/L²), gauge-link discretization, and time-stepping errors. A clean 5% match with the correct sign would be interpreted as a positive Schwinger reproduction; tighter matches at L → ∞ would strengthen the verdict.

The 50% bound for outcome B is the boundary between "qualitative agreement" and "outright disagreement." Anything in (5%, 50%) suggests the right physics is happening but with sizable lattice corrections; below 5% suggests the limit is essentially clean.

---

## 5 · Methodological discipline (committed before measurement)

This pre-registration is **hash-locked**:
- The action specification in §3 is fixed (subject only to the Phase II.1 deliverable's full elaboration).
- The phase breakdown in §2 is fixed.
- The pre-registered outcomes A/B/C/D in §4 are fixed.
- The tolerance bounds in §4 are fixed.

Any change to these *after* running the measurement is a methodology violation and must be flagged in the LEDGER as such.

The git tag `preregister-phase-ii-wilson-dirac-g2-v1` is applied to this commit BEFORE any measurement.

### 5.1 Per-phase pre-registration

Each sub-phase II.1–II.5 includes its own internal pre-registration milestones, all rooted in this document:

- **II.1** internal pre-reg: full action spec + worked example → before II.2
- **II.2** internal pre-reg: build-pass + smoke-test result → before II.3
- **II.3** internal pre-reg: stable orbit at chosen (B, m_e, L) → before II.4
- **II.4** internal pre-reg: ω_c and ω_s extraction protocol locked + applied to N seeds → before II.5
- **II.5** internal pre-reg: a_e formula + comparison observable locked → before final verdict

---

## 6 · References

- `SPEC_ALGEBRAIC_SPINE.md` Theorem 2 (master quadratic)
- `PREREG_PHASE_I_NATIVE_COUPLING.md` (Phase I; g_FTD derivation)
- `PREREG_HEEGNER_TOWER_RIGIDITY.md` (FTD-0124 prereg precedent)
- `DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md` (FTD-0073: spontaneous Clifford emergence closed-negative; Branch-B selection required)
- `DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md` (FTD-0076: spontaneous matter is colored quarks, not electrons; Branch-B Dirac is for QED-sector match)
- `SPEC_FTD_EFT_BRIDGE_CONTRACT.md` (Branch-B matter sector is allowed beyond native signed transport)
- Standard lattice QED references: Wilson 1974, Kogut-Susskind 1975, Montvay-Münster textbook
- Schwinger 1948 (QED a_e = α/(2π) at tree level)
- Aoyama-Hayakawa-Kinoshita-Nio (2012-2019) for QED higher-loop coefficients

---

## 7 · Closure criterion

This pre-registration is closed when:
1. All five sub-phases (II.1 through II.5) complete with their own internal pre-registration milestones met
2. One of the four outcomes A/B/C/D is reported with explicit numerical values
3. LEDGER row added (FTD-0126)
4. The git tag `preregister-phase-ii-wilson-dirac-g2-v1` precedes the measurement-result commits in git history

Not closed by: tolerance-tuning, observable redefinition, phase-skipping, or any post-hoc adjustment.

---

## 8 · Honest expectations

Phase II.1 is theory work — clean, bounded, can be done in a focused session.

Phase II.2 is the substantial engineering investment. Adding Wilson-Dirac to a lattice engine is a well-known operation (every lattice-QCD codebase has it) but FTD's specific architecture (substrate flux + ternary state + 26-neighbor Moore + Gauss projection) means the integration is non-trivial. CPU implementation first; CUDA follow-up.

Phase II.3 is where things can stall. Single-electron-in-B-field on a small lattice with Wilson fermions has known issues: doubler residue, magnetic-translation-symmetry breaking, image-charge artifacts on a torus. Tuning may take longer than estimated.

Phase II.4-II.5 are straightforward signal processing if II.3 produces a clean orbit.

The campaign's most likely failure mode is **outcome D** (infrastructure failure at II.3). The most likely success mode is **outcome B** (5-50% match with correct sign and order of magnitude). Outcome A would be a substantive positive result; outcome C would be a substantive negative result. All four outcomes are informative; the design is genuine pre-registration, not a positive-result-only protocol.
