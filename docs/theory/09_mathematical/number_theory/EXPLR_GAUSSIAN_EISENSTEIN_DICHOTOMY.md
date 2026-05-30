# EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY: Why the Master Quadratic is Gaussian, and What the Odd Term Wants

**Tag:** `[EXPLORATORY MATH]` — mixed status; per-claim tags inline
**Date:** 2026-05-30
**LEDGER:** FTD-0237
**Status:** One new `[STRUCTURAL OBSERVATION]` (the $2^4=4^2$ coefficient uniqueness) + a `[CLARIFICATION]` of the $\mathbb{Z}[\omega]\leftrightarrow G^{*3}$ reading + a reframing of the MC-T4.3 odd-term gap (FTD-0235). **No spine change, no tag promotion, no new derivation of physics.**

---

## 0 · The question

A conjecture worth testing: do the master quadratic's two coefficients — $16G^{*2}$ (linear) and $16G^{*3}$ (constant) — carry a **Gaussian** ($\mathbb{Z}[i]$) vs **Eisenstein** ($\mathbb{Z}[\omega]$) structure, paralleling the cubic lattice's square (⟨110⟩, 45°) vs triangular (⟨111⟩) geometry? The motivating intuition came from two observations: (a) lattice field lines locking to 45° directions, and (b) the square/cube and $i$/$\omega$ duality of the two distinguished CM points $\tau=i$ ($|\mathrm{Aut}|=4$) and $\tau=\rho$ ($|\mathrm{Aut}|=6$).

**Result, in one line.** The $\mathbb{Z}[i]\leftrightarrow G^{*2}$ half is exactly right and theorem-grade; the $\mathbb{Z}[\omega]\leftrightarrow G^{*3}$ half is the wrong *label* for a real $D=3$ structure; and a unique integer identity ($2^4=4^2$) explains why no clean Eisenstein twin of the master quadratic exists.

---

## 1 · The geometry is the Moore Layer Theorem `[THEOREM, established]`

The 26-neighbour Moore shell decomposes uniquely as $26 = 6 + 12 + 8$ (see [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](../../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md), [`DERIV_MOORE_GAUGE_STRUCTURE.md`](../../03_derivations/standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md)):

| Shell | Directions | Angle to axes | Polyhedron | Sublattice | Gauge |
|---|---|---|---|---|---|
| 6 faces | ⟨100⟩ | 0° / 90° | octahedron | SC | U(1) |
| 12 edges | ⟨110⟩ | **45° in-plane** | **cuboctahedron** | FCC | SU(2) |
| 8 corners | ⟨111⟩ | **arccos(1/√3) = 54.7356°** | stella octangula | BCC | SU(3) |

Computed checks: $(1,1,1)$ is at 54.7° to *all three* axes ($\sum\cos^2 = 1$); a single vector at 45° to all three is **impossible** ($\sum\cos^2 = 1.5 \ne 1$). The observed "45° everywhere" is the field riding the ⟨110⟩/FCC shell; the tetrahedral structure is the ⟨111⟩/BCC corners (8 corners = two interlocking tetrahedra = stella octangula).

The **cuboctahedron** is the hinge polyhedron: its 14 faces are **6 squares** (normals along ⟨100⟩, the three $C_4$ axes) **+ 8 triangles** (normals along ⟨111⟩, the four $C_3$ axes $= N_{\text{base}}$). It is the unique Moore polyhedron carrying both 4-fold and 3-fold faces.

---

## 2 · $\mathbb{Z}[i] \leftrightarrow G^{*2}$ — correct and load-bearing `[DERIVED]`

The BCC structure factor is the **triple cosine product** $\sigma_{\text{BCC}}(k) = 1 - \cos k_x\cos k_y\cos k_z$. Two consequences follow from the single fact "BCC offsets have all three components nonzero" (see [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)):

1. **Propagator:** $G_{\text{BCC}}(0) = \sum_m [\binom{2m}{m}/4^m]^3 = \Gamma(1/4)^4/(4\pi^3) = G^{*2}/(2\pi)$ — Watson's integral. This is the **trace** $16G^{*2}$ of the readout. `[THEOREM]`
2. **Gauge:** the triple-axis displacement excites all three flux components → SU(3).

And the BCC complex structure is literally $V_{\text{complex}} \cong \mathbb{Z}[i]^2$, with $J$ acting as $i$ ($J^2 = -I$) — see [`DERIV_BCC_COMPLEX_STRUCTURE.md`](../general_math/DERIV_BCC_COMPLEX_STRUCTURE.md) (FTD-0122). So $G^{*2}$ genuinely lives on a $\mathbb{Z}[i]^2$, and the **even term is forced by Watson**. The "Z[i] for G\*²" intuition is exactly right.

---

## 3 · The $2^4 = 4^2$ coefficient uniqueness `[STRUCTURAL OBSERVATION]` (new)

The master-quadratic coefficient **16 is doubly-sourced**, by two independent routes the corpus records separately:

- **Automorphism route:** $16 = |\mathrm{Aut}(E)|^2 = 4^2$ for the lemniscatic CM curve $E: y^2 = x^3 - x$ ([`SPEC_ALGEBRAIC_SPINE.md`](../../01_reference/SPEC_ALGEBRAIC_SPINE.md) §4, Theorem 4).
- **Tower route:** $16 = 2^4$, the $(1+i)$-tower base $2^k$ at level $k=4$, where $2 = |1+i|^2$ is the norm of the ramified prime ([`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md), Theorem 8).

These two routes agree **only because of the unique integer identity**

$$2^4 \;=\; 4^2 \;=\; 16,$$

and $(2,4)$ is the **sole solution** of $a^b = b^a$ with $a \ne b$ (verified). Phrased structurally: for an imaginary-quadratic CM field with extra automorphisms, the two coefficient routes coincide iff

$$(\text{ramified-prime norm})^{|\text{units}|} \;=\; |\text{units}|^2 .$$

The only two such fields are $\mathbb{Q}(i)$ and $\mathbb{Q}(\rho)$ (the only ones with $|\text{units}| > 2$). For the **Eisenstein partner** $\mathbb{Z}[\omega]$ ($|\text{units}| = 6$, ramified prime $(1-\omega)$ of norm 3) the two routes **diverge**:

$$\underbrace{3^6 = 729}_{\text{tower route}} \;\;\ne\;\; \underbrace{6^2 = 36}_{|\mathrm{Aut}|^2\text{ route}} .$$

**Consequence.** The hypothetical equianharmonic master quadratic — which [`PAPER_GSTAR_INTRODUCTION.tex`](../../../papers/PAPER_GSTAR_INTRODUCTION.tex) §16 *mentions* as $y^2 - 36R_3^2\,y + 36R_3^3$ but tags `[OPEN CONJECTURE]` and never constructs — **has no canonical form**: its integer prefactor is ambiguous (36 from automorphisms, or 729 from the base-3 tower), whereas the Gaussian coefficient is over-determined. **There is no clean Eisenstein twin of the master quadratic.** This sharpens the corpus's open-conjecture status to a structural reason.

**Computed constants (mpmath, 40 dp):** $G^* = \Gamma(1/4)/\Gamma(3/4) = 2.95867512$; the d=3 Gauss-analog $G_\rho = 2.78265513844626$ (matches the η-tower back-out and the H1 atlas); the ratio-channel analog $R_3 = \Gamma(1/3)/\Gamma(2/3) = 1.97836$, with $|\eta(\rho)|^{12} = R_3^9/(216\pi^3)$.

**Non-result (honesty).** The harmonic invariant $1/y_+ + 1/y_- = 1$ holds **identically for both** the Gaussian and Eisenstein quadratics (and for any "constant $=$ const-factor $\times$ linear" polynomial), so it does **not** discriminate $\mathbb{Z}[i]$ from $\mathbb{Z}[\omega]$ — confirming [`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md) and [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) §"multiplier underdetermination."

---

## 4 · $\mathbb{Z}[\omega] \leftrightarrow G^{*3}$ — the honest resolution `[CLARIFICATION]`

"Z[ω] for G\*³" is the wrong label, for two reasons, but it points at something real.

1. **$G^{*3}$ is Gaussian, not Eisenstein.** It is a *power of* $G^*$. The genuine Eisenstein constants are different objects ($R_3$, $G_\rho$, built from $\Gamma(1/3)$).
2. **The exponent 3 reads as $D = 3$, not $\omega$.** FTD's own live hint for the odd determinant ([`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`](../../10_eft_program/preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md), FTD-0235) is
   $$16G^{*3} \;=\; |\mu_4|^2 \cdot \!\!\prod_{\text{3 planes}}\!\! (\det\nolimits_\zeta \text{ ratio} = G^*),$$
   "the determinant carries **three** $\det_\zeta$ ratios, one per spatial plane, while the trace carries two." Those three planes are the **three coordinate planes — each a square $\mathbb{Z}[i]$ lattice**, not a hexagonal one. So $G^{*3}$ is best read as **$D=3$ copies of the Gaussian source**, not as an Eisenstein object.
3. **$\mathbb{Z}[\omega]$-as-ring is explicitly ruled out in $\mathbb{Z}^3$** ([`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](../../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md), `[SELECTION]` anchored in Axiom Zero): "Z[ω] lives on a hexagonal lattice… neither embeds in $\mathbb{Z}^3$ respecting ring structure; the cubic-lattice axiom selects $d=1$."

**What survives of the intuition (and it is real).** The three square $\mathbb{Z}[i]$ planes whose product gives $G^{*3}$ are **cyclically permuted by the $C_3$ rotation about the body diagonal** — the ⟨111⟩/triangular-face/tetrahedral 3-fold axis. So the "3-fold" *is* genuinely present, as the **rotational symmetry organizing the determinant's three planes**, even though the planes themselves are Gaussian. The conjecture conflated the *organizing 3-fold rotation* ($C_3 \subset O_h$, real, ⟨111⟩) with an *Eisenstein CM ring* ($\mathbb{Z}[\omega]$, translational, ruled out). The first is correct; the second is not. The honest slogan: **$G^{*3} = $ three $\mathbb{Z}[i]$ planes, glued by $C_3$ — $D$-fold, not $\omega$-fold.**

---

## 5 · Why this matters: the MC-T4.3 odd-term gap `[OPEN]`

The even/odd asymmetry between $G^{*2}$ and $G^{*3}$ **is** the current foundational obstruction (see [`WHERE_WE_LEFT_OFF.md`](../../../WHERE_WE_LEFT_OFF.md) §0.15; [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](../../10_eft_program/derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md)). The readout is a $2\times2$ transfer operator on $V_{\text{complex}} \cong \mathbb{Z}[i]^2$ with $(\mathrm{Tr}, \mathrm{Det}) = (16G^{*2}, 16G^{*3})$:

- **Trace $16G^{*2}$** = Watson Green's function → **forced** `[DERIVED]`.
- **Determinant $16G^{*3}$** = the odd term, *asserted* as the Vieta target, **not derived** (Watson gives $G^{*2}$, not $G^{*3}$) → `[UNDERDETERMINED]` (W-CRIT-2). A clean odd source exists (the J-twisted $\det_\zeta$ ratio $= G^*$, FTD-0234), but the **det↔$\det_\zeta$ structural identity** that would compel the determinant is the missing hinge (FTD-0235).

This note's contribution is to **localize and name** that gap: the trace is the 2-component $\mathbb{Z}[i]$ object (forced by Watson); the determinant wants the 3-plane product; and §3 shows why the natural "Eisenstein forcing" *cannot* supply it — the $2^4=4^2$ over-determination that forces the Gaussian side has no Eisenstein analog. So the odd term is underdetermined not by oversight but because the would-be forcing structure ($\mathbb{Z}[\omega]$) is ontologically rotational-only in $\mathbb{Z}^3$. **This reframes, but does not close, MC-T4.3.**

---

## 6 · A falsifiable next step (ARC-D, engine-native) `[PROPOSED FALSIFIER]`

The surviving MC-T4.3 route is engine-native measurement. The trace/determinant split suggests a concrete test:

> **Measure, in the lattice engine, whether the oriented flux determinant on the 8 ⟨111⟩/BCC corners** (the stella-octangula structure, cyclically permuted by $C_3$) **dynamically realizes the three-plane product $\prod_{\text{3 planes}} G^* = G^{*3}$ — or prove it cannot.**

A confirming measurement would *force* the odd term and close MC-T4.3 positive; a null result closes it negative. Either is a result. The visual signature is exactly the user-observed split: field lines locking to ⟨110⟩ (the square/$\mathbb{Z}[i]$ trace layer) versus the ⟨111⟩ corners (the determinant/3-fold layer). This is a **theory-only proposal**; no engine run is asserted here.

---

## 7 · Scope, caveats, and what is NOT claimed

- The $2^4=4^2$ observation rests on the natural identifications "tower base = ramified-prime norm" and "level = $|\text{units}|$," both of which are themselves `[SELECTION]` per [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) (the harmonic invariant does not force base 2). It is a structural observation, **not a forcing theorem**.
- No physical quantity is derived. The Eisenstein quadratic roots are **not** compared to any physical constant — doing so would be a coincidence search, forbidden by the project's epistemic discipline.
- This does **not** close MC-T4.3, does **not** promote $x_+ = 1/\alpha$ (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]`), and adds **no** spine theorem.
- The central new claim is `[STRUCTURAL OBSERVATION]`; the $\mathbb{Z}[\omega]\to D=3$ correction is `[CLARIFICATION]`; the MC-T4.3 reframing is `[OPEN]`.

---

## 8 · Verification

All numbers computed with `mpmath` (40 dp) in-session, not recalled:
- $G^* = 2.958675119$; back-out from $|\eta(i)|^8 = G^{*4}/(64\pi^2)$ matches to 40 dp.
- $G_\rho = 2.782655138446263$ from $|\eta(\rho)|^{12} = G_\rho^6/(216\pi^3)$, matching `PAPER_GSTAR_H1_ATLAS.tex` d=3.
- $a^b = b^a$, $a<b$, $a,b\le 12$: unique solution $(2,4)$.
- $2^4 = 4^2 = 16$; $3^6 = 729 \ne 6^2 = 36$.
- Harmonic invariant $1/y_+ + 1/y_- = 1$ verified to ~37 dp for the Gaussian and *both* Eisenstein routes → non-discriminating.
- Cuboctahedron ⟨110⟩ shell = 12 vertices; ⟨111⟩ angle = 54.7356°; $\sum\cos^2$ for ⟨111⟩ = 1, for a hypothetical 45°-to-all-axes = 1.5.

## 9 · Cross-references

- Geometry: [`THEOREM_MOORE_LAYER_DECOMPOSITION.md`](../../08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md), [`DERIV_MOORE_GAUGE_STRUCTURE.md`](../../03_derivations/standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md)
- $\mathbb{Z}[i]$ / Watson: [`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`](../../08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md), [`DERIV_BCC_COMPLEX_STRUCTURE.md`](../general_math/DERIV_BCC_COMPLEX_STRUCTURE.md)
- Tower / coefficient: [`SPEC_ALGEBRAIC_SPINE.md`](../../01_reference/SPEC_ALGEBRAIC_SPINE.md) §4 §8, [`THEOREM_HARMONIC_INVARIANT_TOWER.md`](../../03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md), [`EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md`](EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md)
- Eisenstein / equianharmonic: [`PAPER_GSTAR_INTRODUCTION.tex`](../../../papers/PAPER_GSTAR_INTRODUCTION.tex) §16, [`PAPER_GSTAR_ETA_TOWER.tex`](../../../papers/PAPER_GSTAR_ETA_TOWER.tex), [`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`](../../01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md)
- MC-T4.3 odd term: [`FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md`](../../10_eft_program/derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md), [`PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md`](../../10_eft_program/preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md), [`AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md`](../../07_assessment/audits/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md)
- Ledger: [`LEDGER.md`](../../07_assessment/core_ledgers/LEDGER.md) FTD-0237
