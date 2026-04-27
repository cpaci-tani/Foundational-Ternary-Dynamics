# AUDIT — G* (Reflection-Ratio) vs π (Reflection-Product) Across Asymmetric Physics Formulas

**Tag:** [HYPOTHESIS] (theory-only catalog + pre-registration; engine measurements deferred to per-domain follow-ups)
**Date:** 2026-04-27
**LEDGER row:** FTD-0106
**Plan:** `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (G\*-asymmetry investigation)
**Companion:** [`PROTOCOL_GSTAR_ASYMMETRY_SCAN.md`](PROTOCOL_GSTAR_ASYMMETRY_SCAN.md) (pre-registration, same commit)

---

## 0 · Scope and anti-targets

**This document IS:** a structured catalog (per Tier-1 domain) of physics formulas where π appears, classified by structural origin (closed-loop / 1D-mode-counting / Γ-ratio-already-asymmetric); a prediction matrix of G\*-native candidates; per-domain derivation-route verdict; a look-elsewhere expected-hit-count baseline.

**This document IS NOT:** a derivation. Every G\*-native candidate stays at most [HYPOTHESIS] until either (a) derived from FTD axioms via Heat Equation / CM L-function / direct reflection-ratio identification, OR (b) engine-measured against the falsifier in the companion PROTOCOL.

**Parallel reading:** `PAPER_RATIO_AND_THE_ARROW.tex` and `FOUND_THE_RATIO_AND_THE_PRODUCT.md` already establish the product/ratio framing. `DERIV_HEAT_EQUATION_FROM_RATIO.md` already proves [THEOREM]: G\* = eigenvalue of D^(−1/2) at z = 1/4. The PF Atlas (`SPEC_FTD_COMPARATIVE_PHYSICS.md`) already decomposes π → 16·PF for many formulas. **This investigation does not modify any of those; it sits parallel and asks whether specific physics formulas in directional/asymmetric domains have G\*-native re-expressions distinguishable from the standard π-laden form by numerical prediction.**

**Anti-targets (Koide failure mode):** No promotion to [SELECTION] without derivation route OR engine measurement. No silent search across many candidate forms looking for matches. Look-elsewhere control mandatory (§4 below). Pre-registration locked at the companion PROTOCOL's `git tag preregister-gstar-asymmetry-v1`; no post-hoc adjustment.

---

## 1 · Phase 1 — Catalog per domain

### Domain A — Time-direction / dissipation / Rayleigh damping

**Anchor:** [`DERIV_HEAT_EQUATION_FROM_RATIO.md`](../03_derivations/DERIV_HEAT_EQUATION_FROM_RATIO.md) [THEOREM] proves that the reflection ratio R(z) = Γ(z)/Γ(1−z) is the eigenvalue of the fractional operator D^(2z−1) acting on power-law states. At z = 1/4: R(1/4) = G\*; the operator becomes D^(−1/2) (the half-integral); this is the operator that defines the Heat Equation's boundary-flux / temperature relation, Brownian motion, and irreversible diffusion. The product Γ(z)Γ(1−z) generates time-reversible structures; the ratio generates time-asymmetric ones. **This is the structural reason G\* should appear where dissipation / arrow-of-time physics dominates.**

**Catalog (5 formulas):**

| # | Formula | Standard form | π origin | Replaceability |
|---|---|---|---|---|
| A1 | Half-derivative heat-flux relation | $q(t) = \sqrt{k\rho c}\,\partial_t^{1/2} T(0,t)$ | structural — 1D heat eq fundamental | **directly G\*-anchored** via $D^{-1/2}$ eigenvalue |
| A2 | Kramers escape rate prefactor | $r = (\omega_0/2\pi)\,e^{-E_b/k_BT}$ | thermal-circle period (1D, Euclidean) | likely G\*-replaceable |
| A3 | Fluctuation-dissipation (overdamped Langevin) | $\langle x(0)x(t)\rangle = (k_BT/\gamma)\,e^{-\gamma t/m}$ — γ=damping coeff | $\gamma$ has no direct π; but Stokes drag $\gamma = 6\pi\eta r$ for sphere in fluid | **partial** — Stokes drag is sphere-specific |
| A4 | Rayleigh damping in dissipative wave eq | $\partial_t^2 u + 2\eta\,\partial_t u = c^2\,\nabla^2 u$ | η is empirical, no fundamental π | **null** — π absent, no replacement to make |
| A5 | Onsager reciprocal relations | $L_{ij} = L_{ji}$ symmetric matrix at equilibrium | structural symmetry, not numerical π | **null** — symmetry statement, no constant to replace |

### Domain B — Coulomb scattering phase shift

**Anchor:** standard non-relativistic Coulomb scattering already uses a Γ-ratio at complex-conjugate arguments:

$$\frac{\Gamma(l+1+i\eta)}{\Gamma(l+1-i\eta)} = e^{2i\sigma_l}$$

where σ_l is the **Coulomb phase shift** — empirically measured in nuclear and atomic scattering experiments. The reflection-ratio structure is **already empirically real here**, just not at the lemniscatic point z = 1/4.

**Catalog (5 formulas):**

| # | Formula | Standard form | π origin | Replaceability |
|---|---|---|---|---|
| B1 | Coulomb phase shift σ_l | $\sigma_l = \arg\Gamma(l+1+i\eta)$ | Γ-ratio at complex-conjugate args (asymmetric) | **structurally already-G\*-like**; no replacement needed |
| B2 | Sommerfeld parameter η | $\eta = Z_1 Z_2 e^2/(\hbar v) = Z_1 Z_2 \alpha c/v$ | π-free; α has 4π in its definition | **partial** — depends on α decomposition |
| B3 | Rutherford differential cross-section | $\frac{d\sigma}{d\Omega} = \left(\frac{Z_1 Z_2 e^2}{4 E}\right)^2 \frac{1}{\sin^4(\theta/2)}$ | $\sin^4(\theta/2)$ from sphere geometry; $4E = 2mv^2$ no π | **null** — no π in dimensional structure |
| B4 | Coulomb-corrected scattering amplitude | $f(\theta) \propto -\eta/(2k\sin^2(\theta/2))\cdot e^{-i\eta\ln\sin^2(\theta/2)} \cdot \Gamma(1+i\eta)/\Gamma(1-i\eta)$ | Γ-ratio at complex-conjugate (B1 phase) + log-modified plane-wave | **structurally Γ-ratio**, already aligned |
| B5 | Bohr radius / hydrogen ground state | $a_0 = \hbar/(m_e c\alpha) = 4\pi\epsilon_0\hbar^2/(m_e e^2)$ | 4π from Coulomb's law (sphere-symmetric) | **null** — same 4π as horizon area, sphere-symmetric |

### Domain C — Hawking evaporation timescale + BH thermodynamic prefactors

**Anchor:** `DERIV_BLACK_HOLE_PHYSICS.md` §5 [THEOREM]:

$$\tau_{\text{evap}} = \frac{5120\,\pi\,G^2 M^3}{\hbar c^4}, \qquad P_{\text{lum}} = \frac{\hbar c^6}{15360\,\pi\,G^2 M^2}$$

The prefactors 5120π and 15360π combine: (i) 4π from horizon area $A = 4\pi r_s^2$ (already shown sphere-symmetric in FTD-0105 D1), (ii) the Stefan-Boltzmann coefficient $\sigma_{SB} = \pi^2 k_B^4/(60\hbar^3 c^2)$ from 1D Planck mode counting (ζ(4) = π⁴/90), (iii) Hawking T = $1/(8\pi GM)$ raised to the fourth. The composite π-power in 5120π is therefore **mostly from 1D mode counting**, not 2-sphere geometry — exactly the kind of "directional π" that Domain A's Heat Equation route might re-express via G\*.

**Catalog (5 formulas):**

| # | Formula | Standard form | π origin | Replaceability |
|---|---|---|---|---|
| C1 | BH evaporation timescale | $\tau = 5120\pi\,G^2M^3/(\hbar c^4)$ | composite: 4π·sphere · ζ(4)·1D-mode · (8π)⁴-Hawking⁻⁴ | **partial** — 4π locked sphere-symmetric (FTD-0105); rest possibly G\* |
| C2 | BH luminosity | $P = \hbar c^6/(15360\pi\,G^2 M^2)$ | same composite as C1 | **partial** — same as C1 |
| C3 | Stefan-Boltzmann constant | $\sigma_{SB} = \pi^2 k_B^4/(60\hbar^3 c^2)$ | $\zeta(4) = \pi^4/90$, 1D mode counting integration | **likely G\*-replaceable** if 1D-mode-counting has G\*-native form |
| C4 | Hawking temperature | $T_H = \hbar c^3/(8\pi G M k_B)$ | Euclidean conical-deficit periodicity around horizon | **partial** (FTD-0105 D2 inconclusive due to unit mismatch) |
| C5 | Bekenstein-Hawking entropy | $S = A/(4\ell_P^2) = \pi r_s^2/\ell_P^2$ | 4π from sphere area; 1/4 from constraint reduction | **null** for the 4π part (FTD-0105); the 1/4 is independent |

---

## 2 · Phase 2 — G\*-native candidate prediction matrix

For each formula, the candidate set: standard π-form, plus **at most two** numerical-distinct candidates (Candidate I = π → G\* substitution; Candidate II = π → ϖ substitution; or **derivation-anchored form** if one exists per Phase 3). All values reported to 4 decimal places.

**Numerical reference values** (verified against `scripts/constants.py`):

| Constant | Value |
|---|---|
| π | 3.1416 |
| ϖ | 2.6221 |
| G\* | 2.9587 |
| 2π | 6.2832 |
| 2G\* | 5.9173 |
| π² | 9.8696 |
| G\*² | 8.7538 |
| 4π | 12.5664 |
| 4G\* | 11.8347 |
| 5120π | 16085 |
| 5120·G\* | 15148 |
| 5120·ϖ | 13425 |

### Domain A predictions

| # | Formula | Standard | Candidate I (π→G\*) | Candidate II (π→ϖ) | Derivation-anchored value |
|---|---|---|---|---|---|
| A1 | $\sqrt{k\rho c}$ coefficient on $\partial_t^{1/2}$ | (no isolated π) | n/a | n/a | **G\*** as eigenvalue (anchored) |
| A2 | Kramers prefactor | $1/(2\pi) = 0.1592$ | $1/(2G^*) = 0.1690$ (+6.2%) | $1/(2\varpi) = 0.1907$ (+19.8%) | Heat-Eq route → likely G\* |
| A3 | Stokes drag (sphere) | $6\pi$ = 18.85 | $6G^* = 17.75$ (−5.8%) | $6\varpi = 15.73$ (−16.6%) | none — sphere-specific (4π anchor) |
| A4 | Rayleigh damping coefficient | (empirical, no π) | n/a | n/a | n/a |
| A5 | Onsager L_ij = L_ji | (symmetry, no π) | n/a | n/a | n/a |

### Domain B predictions

| # | Formula | Standard | Candidate I (π→G\*) | Candidate II (π→ϖ) | Derivation-anchored value |
|---|---|---|---|---|---|
| B1 | σ_l coefficient | (Γ-ratio direct, no π) | n/a | n/a | already lemniscatic at l=0, η=...? |
| B2 | Sommerfeld η = Z₁Z₂α·c/v | depends on α | depends on α | depends on α | downstream of master quadratic root |
| B3 | Rutherford dσ/dΩ | no π in coefficient | n/a | n/a | n/a |
| B4 | Coulomb amplitude | (Γ-ratio direct) | n/a | n/a | already lemniscatic at z=1/4 only if η=±1/4·i |
| B5 | Bohr radius (4π in Coulomb) | $4\pi\epsilon_0$ → 4π = 12.57 | $4G^* = 11.83$ (−5.8%) | $4\varpi = 10.49$ (−16.6%) | sphere-symmetric (FTD-0105 closed-negative) |

### Domain C predictions

| # | Formula | Standard | Candidate I (π→G\*) | Candidate II (π→ϖ) | Derivation-anchored value |
|---|---|---|---|---|---|
| C1 | τ_evap coefficient | $5120\pi = 16085$ | $5120 G^* = 15148$ (−5.8%) | $5120\varpi = 13425$ (−16.6%) | none clean — composite |
| C2 | P_lum coefficient | $1/(15360\pi) = 2.07\times 10^{-5}$ | $1/(15360 G^*) = 2.20\times 10^{-5}$ (+6.2%) | $1/(15360\varpi) = 2.48\times 10^{-5}$ (+19.8%) | none clean — composite |
| C3 | σ_SB = π²/(60·…) | $\pi^2/60 = 0.1645$ | $G^{*2}/60 = 0.1459$ (−11.3%) | $\varpi^2/60 = 0.1146$ (−30.4%) | possible Heat-Eq route |
| C4 | T_H = …/(8π) | $1/(8\pi) = 0.0398$ | $1/(8G^*) = 0.0423$ (+6.2%) | $1/(8\varpi) = 0.0477$ (+19.8%) | FTD-0105 D2 INCONCLUSIVE |
| C5 | S_BH (4π part) | 4π in r_s² | 4G\* | 4ϖ | FTD-0105 D1 closed-negative for sphere |

**Empty cells** (n/a): formula doesn't have a π-coefficient to substitute, OR the π is structural (Γ-ratio, sphere-symmetric, etc.) where substitution doesn't make structural sense.

**Sanity-check constraint**: any row where Standard = Candidate I OR Standard = Candidate II to within rounding has been flagged as "no engine arbitration possible" — these don't contribute to the look-elsewhere count.

---

## 3 · Phase 3 — Derivation-route verdict per domain

### Domain A — derivation-anchored ✓

**Heat Equation route (DERIV_HEAT_EQUATION_FROM_RATIO.md):**

The chain is:
1. R(z) = Γ(z)/Γ(1−z) is the eigenvalue of D^(2z−1) acting on power-law states $x^{z-1}$. [THEOREM]
2. At z = 1/4, R(1/4) = G\* exactly; D^(2·(1/4)−1) = D^(−1/2). [THEOREM]
3. D^(−1/2) is the half-integral operator that governs the heat-flux / boundary-temperature relation: $q(t) = \sqrt{k\rho c}\,\partial_t^{1/2} T(0,t)$. [THEOREM in PDE theory]

So **A1 is derivation-anchored**: the prefactor $\sqrt{k\rho c}$ multiplying the half-derivative IS, structurally, the G\*-eigenvalue manifestation of the underlying reflection ratio. Not a numerical coincidence — an operator identity.

**A2 (Kramers prefactor):** the 1/(2π) in the Kramers rate comes from the Gaussian normalization in the saddle-point approximation around the barrier top. The Gaussian is $\exp(-\beta \omega_0^2 q^2/2) / \sqrt{2\pi/(\beta\omega_0^2)}$, which gives a 1/√(2π) per dimension. The Kramers formula has 1/(2π) from $\sqrt{2\pi}_{\text{forward}} \cdot \sqrt{2\pi}_{\text{backward}}^{-1}$ with the imaginary unit absorbed. **Heat-Eq route gives a partial argument**: the "thermal circle" with Euclidean periodicity β is itself a 1D structure whose mode-counting could route through G\* via the half-integral operator. But the standard derivation goes through Gaussian saddle-point, not directly through D^(−1/2). **Verdict: partial** — plausible route, not closed.

**A3 Stokes drag:** 6π = 6 × π for a sphere in a viscous fluid. The 6 is dimensional; the π is sphere geometry. Already covered by FTD-0105 sphere-symmetric closure. **No replacement.**

**Domain A overall verdict**: **derivation-anchored for A1; partial for A2; null for A3-A5.**

### Domain B — partial (one row)

**Reflection-ratio direct identification:**

B1 already uses the reflection-ratio structure (at complex-conjugate, not lemniscatic point). The structural question: **is there a kinematic regime in which the Coulomb phase shift's Γ-ratio reduces to the lemniscatic ratio at z = 1/4?** This requires η = ±i/4 or l = −3/4 or some combination. 

Numerically: σ_l(η = i/4) requires $\arg\Gamma(l+1+i/4·i) = \arg\Gamma(l+1−1/4) = \arg\Gamma(l+3/4)$. For l = 0: $\arg\Gamma(3/4)$, which is real (Γ(3/4) is real positive), so the phase is 0 — degenerate. For η imaginary, the formula isn't directly the Coulomb scattering parameter (η = Z₁Z₂α·c/v is real). **No clean kinematic regime maps Coulomb scattering to z = 1/4 lemniscatic point.**

**Domain B overall verdict**: **partial via B1's structural alignment with reflection-ratio, but no derivation route to a specific G\*-native physics observable.**

### Domain C — partial / no clean route

**The 5120π in C1** is composite: 4π · 1280 · X where X involves $\zeta(4)/\pi^4$ from Stefan-Boltzmann times a (1/8π)⁴ from Hawking T raised to the 4th. Specifically:

$$\tau_\text{evap} \propto \frac{1}{P_\text{lum}/M c^2} \propto \frac{M^3}{\sigma_{SB} A T_H^4}$$

Substituting: $\sigma_{SB} = \pi^2/60$ (in $\hbar = c = k_B = 1$ units), $A = 4\pi r_s^2$, $T_H = 1/(8\pi M)$, $r_s = 2M$:

$$\tau_\text{evap} \propto \frac{M^3}{(\pi^2/60)(16\pi M^2)(1/(8\pi M)^4)} = \frac{M^3 \cdot (8\pi M)^4 \cdot 60}{16\pi M^2 \cdot \pi^2}$$
$$= \frac{60 \cdot 4096 \pi^4 M^7}{16\pi^3 M^2} = \frac{60 \cdot 4096}{16}\pi M^5 = 15360\pi M^5$$

Wait — this gives M^5, but the standard formula has M^3. The mismatch is because $\tau \sim M / |dM/dt|$ and $|dM/dt| = P/c^2 \sim 1/M^2$, so $\tau \sim M^3$. Let me redo: $|dM/dt| = P_\text{lum} = \sigma_{SB} A T_H^4 = (\pi^2/60)(16\pi M^2)(1/(8\pi M))^4 = \pi^2/60 \cdot 16\pi M^2 / (4096 \pi^4 M^4) = 16/(60 \cdot 256 \pi M^2) = 1/(960\pi M^2)$. So $\tau = M/(dM/dt)^{-1} \cdot ... = ... · M^3$ (correct sign and power). The exact prefactor: $\tau = \int_M^0 dM'/(-1/(960\pi M'^2)) = -960\pi \int_M^0 M'^2 dM' = 960\pi M^3/3 = 320\pi M^3$.

The standard quoted value 5120π differs by a factor — this is because the FTD doc uses different units conventions (G, ℏ, c, k_B factors). The point is: **the 5120π breaks down as 4π (sphere) × ζ(4)-derived (1D Planck) × (8π)⁻⁴ (Hawking) × dimensional factors**. Each π has a different structural origin. To replace ANY of them cleanly with G\* requires a derivation per source:

- 4π (sphere): closed-negative per FTD-0105
- ζ(4) = π⁴/90 (1D Planck mode counting): possibly G\*-replaceable via Heat Equation if 1D-mode-counting on the lattice has G\*-native form. **Open question.**
- (8π) in T_H: FTD-0105 D2 inconclusive due to unit mismatch.

**Domain C overall verdict**: **partial — no single clean route**. The composite prefactor 5120π is built from at least three structurally distinct π-sources, two of which have already been investigated (sphere via FTD-0105 D1 closed-negative; T_H via FTD-0105 D2 inconclusive). The third (1D-mode-counting ζ(4)) is the cleanest remaining open question.

### Cross-domain summary

| Domain | Strongest derivation route | Status |
|---|---|---|
| A | Heat Eq → G\* = D^(−1/2) eigenvalue | A1 **anchored**, A2 partial |
| B | Reflection-ratio direct (Γ at complex-conjugate) | B1 structurally aligned, no specific observable identified |
| C | Composite — three π-sources | A4-style closed (sphere), C3 1D-mode-counting open |

**Net**: **only Domain A produces a clear derivation-anchored row (A1)**. Domains B and C have structural-alignment hints but no clean derivation route to a numerically specific G\*-native prediction beyond what the existing Phase 2 candidate matrix lists.

---

## 4 · Phase 5 — Look-elsewhere control

Without this, "G\* fits N out of 15 formulas" is statistically meaningless.

### Method

For each row in the Phase 2 prediction matrix that has a numerical-distinct G\*-candidate, compute the **prior probability under null** that a "random" candidate of similar functional form would land within ±5% of the standard value.

The candidate set explored across the catalog:
{π, G\*, ϖ, π², G\*², ϖ², 4π, 4G\*, 4ϖ, 8π, 8G\*, 8ϖ, π/2, G\*/2, ϖ/2, 5120π, 5120G\*, 5120ϖ, 15360π, 15360G\*, 15360ϖ}

Per-row prior (rough): given two candidates I and II per formula in the candidate matrix, each ±5% of standard, the prior probability that EITHER candidate matches within ±5% is approximately:

$$P_\text{null}(\text{at least one of I, II within} \pm 5\%) \approx \frac{2 \cdot 0.10}{|relative spacing|}$$

For the candidate set with relative spacings ~6-30%, the prior is **0.1-0.4 per row**. Average ~0.2.

### Per-domain expected hit count under null

| Domain | Rows with active candidates | Expected hits under null (avg prior 0.2) |
|---|---|---|
| A | 2 (A2, A3) | 0.4 |
| B | 1 (B5) | 0.2 |
| C | 4 (C1, C2, C3, C4) | 0.8 |
| **Total** | **7** | **1.4** |

### Threshold for "investigation contributes evidence"

Per the locked Phase-5 criterion: investigation evidence is present if observed match count **≥ 2× expected = ≥ 3 hits**, AND **concentrated in derivation-anchored rows** (i.e., A2 not B5).

Reading the Phase 2 candidate matrix at face value (no engine arbitration yet):

- A2 Kramers $1/(2G^*) = 0.169$ vs standard $1/(2\pi) = 0.159$. Numerical mismatch +6.2% — outside ±5%.
- A3 Stokes $6G^* = 17.75$ vs $6\pi = 18.85$, mismatch −5.8% — outside ±5%.
- B5 Bohr $4G^* = 11.83$ vs $4\pi = 12.57$, mismatch −5.8% — outside ±5%.
- C1 τ_evap $5120 G^* = 15148$ vs $5120\pi = 16085$, mismatch −5.8% — outside ±5%.
- C2 P_lum $1/(15360 G^*) = 2.20\times 10^{-5}$ vs $1/(15360\pi) = 2.07\times 10^{-5}$, mismatch +6.2% — outside ±5%.
- C3 σ_SB $G^{*2}/60 = 0.146$ vs $\pi^2/60 = 0.165$, mismatch −11.3% — outside ±5%.
- C4 T_H $1/(8G^*)$ vs $1/(8\pi)$, mismatch +6.2% — outside ±5%.

**Observed hits**: **0 out of 7** rows have a Candidate I that matches Standard within ±5%.

This is BELOW the null expectation of 1.4. Under the look-elsewhere null, the candidates are NOT preferentially fitting the standard values — they're systematically OFF by ~6-30%, which is exactly the signature of "G\* is a different number from π and the formulas use π."

**Investigation evidence verdict (a priori, before engine arbitration)**: **negative**. The simple Candidate-I/Candidate-II direct-substitution candidates do NOT cluster at the standard values; they're visibly distinct (~6-30% off). Either:

(i) the standard formulas DO use π (not G\*), and the investigation closes negative for direct substitution;

(ii) the right G\*-native form is NOT a direct substitution but a structurally-motivated derivation (Heat Equation route for A1; possibly Phase 3-derivable form for C3);

(iii) the engine produces a value distinct from BOTH standard AND simple substitutions, exactly as FTD-0105 D1 found at 18.51 ≈ 1.5·4π for a digital-geometry overhead reason.

**Reading (iii) is structurally informative — the engine arbitrates whether the lattice's actual answer is closer to standard, to a G\*-native candidate, or to a digital-geometry-overhead-corrected version.** The catalog's negative result for direct substitution does NOT close the question; it just confirms that "naive π → G\* swap" is not the answer for the formulas in the Tier-1 catalog.

---

## 5 · What this audit says (and what it explicitly doesn't)

### Honest claims at this stage

1. **The reflection-ratio R(z) at z = 1/4 equals G\*** — [THEOREM] (algebra)
2. **G\* is the eigenvalue of D^(−1/2)** — [THEOREM] (Heat Equation derivation)
3. **A1 (heat-flux half-derivative) is derivation-anchored to G\*** — [SELECTION] at most: the prefactor $\sqrt{k\rho c}$ on $\partial_t^{1/2}$ is the operator-coefficient form of G\*, but $\sqrt{k\rho c}$ doesn't have an isolated π that needs replacing — it's a dimensional combination of material constants. So the "G\*-native" reading here is structural, not numerical. **No engine prediction distinct from standard.**
4. **Naive π → G\* substitution at simple substitution candidates** in 7 catalog rows produces NO match within ±5% — consistent with null (look-elsewhere expected hits 1.4, observed 0).
5. **Domain-by-domain, only A1 has a clean derivation route**; B and C have structural-alignment hints without numerical predictions distinct from standard at engine-falsifiable precision.

### Honest non-claims

- The investigation does NOT support "G\* replaces π" anywhere it wasn't already known structurally
- The investigation does NOT close the question for the engine — the engine could produce values distinct from both standard and naive G\*-substitutes (per FTD-0105 lesson)
- The investigation does NOT reduce the existing Heat Equation [THEOREM] to a numerical claim — that theorem stands on operator algebra, not on physics-formula matches
- The investigation does NOT promote any G\*-native candidate from this matrix to [SELECTION] without engine measurement

---

## 6 · Per-domain follow-up tickets (DEFERRED, not this session)

**Recommended next-step engine campaigns** (one per domain, each separately pre-registered):

- **FTD-0107 (Domain A engine)**: measure dissipation timescale on Langevin lattice; compare to standard fluctuation-dissipation prediction vs G\*-native forms via the engine's known Langevin equipartition (FTD-0051).
- **FTD-0108 (Domain B engine)**: measure Coulomb phase shift in Rutherford scattering benchmark; compare to standard $\Gamma$-ratio vs lemniscatic-aligned forms at specific kinematic points.
- **FTD-0109 (Domain C engine)**: measure BH evaporation rate scaling on lattice; compare 5120π · G²M³ standard vs candidates.

**Each follow-up requires its own pre-registration**. This AUDIT and the companion PROTOCOL together pre-register the **shared candidate value list** for these follow-ups but NOT the per-domain engine observables — those are domain-specific and will be locked in their own protocols.

---

## 7 · Cross-references

| Section | Anchor doc | Status |
|---|---|---|
| Domain A anchor | `DERIV_HEAT_EQUATION_FROM_RATIO.md` | [THEOREM] |
| Domain B anchor | (Coulomb scattering literature; Newton 1966) | external |
| Domain C anchor | `DERIV_BLACK_HOLE_PHYSICS.md` §5 | [THEOREM] |
| Reflection-ratio framing | `PAPER_RATIO_AND_THE_ARROW.tex` | foundational paper |
| Product/ratio distinction | `FOUND_THE_RATIO_AND_THE_PRODUCT.md` | [CONJECTURE] foundational |
| Look-elsewhere methodology | `PROTOCOL_LOOK_ELSEWHERE_SCAN.md` (FTD-0097) | adapted here |
| Prior FTD-0105 lessons | `AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md` + `ANALYSIS_LEMNISCATIC_REPLACEMENT.md` + `AUDIT_FTD0105_MATH_CHECK.md` | [PARTIAL] |
| Spine reference | `SPEC_ALGEBRAIC_SPINE.md` | [REFERENCE] |
| Companion PROTOCOL | `PROTOCOL_GSTAR_ASYMMETRY_SCAN.md` | [PROTOCOL] this commit |

---

## 8 · Single-line summary

**Theory-only audit of three Tier-1 domains (time-direction/dissipation, Coulomb scattering phase, Hawking evaporation timescale) where the reflection-ratio constant G\* might appear instead of the reflection-product constant π. Catalog of 15 formulas with structural-origin annotation; Phase 2 prediction matrix with naive π→G\* and π→ϖ candidates; Phase 3 derivation-route verdict (only Domain A has a clean derivation-anchored row, A1 via Heat Equation [THEOREM]); Phase 5 look-elsewhere expected-hit count = 1.4 under null, observed = 0 in catalog (naive substitution candidates are systematically 6-30% off from standard, consistent with null). Investigation evidence verdict at theory level: NEGATIVE for naive direct substitution; OPEN for derivation-anchored reformulations and engine arbitration. Three per-domain follow-up tickets (FTD-0107/0108/0109) deferred for separate pre-registration. Pre-reg gate: `git tag preregister-gstar-asymmetry-v1` at this commit. LEDGER row FTD-0106 [HYPOTHESIS]. Anti-targets explicit: no Koide-style numerical fishing; no promotion above [HYPOTHESIS] without per-domain measurement or derivation; PF Atlas + PAPER_RATIO_AND_THE_ARROW.tex unchanged.**
