# Geometric Readings of Mass-Ratio and Coupling Integers (2026-06-18 batch, consolidated 2026-08-06)

**Epistemic Status:** `[STRUCTURALLY MOTIVATED PARAMETRIC]` throughout, except where a section states a different tag explicitly (§4's proton ratio is `[STRONGLY MOTIVATED CONJECTURE]`; §4's quark masses and §1's Muon/Tau are `[PARAMETRIC]`/`[STRUCTURALLY MOTIVATED PARAMETRIC]` as marked).

> **Consolidation notice (2026-08-06).** This document merges four documents
> created together in commit `24b31016` (2026-06-18), which apply the same
> technique — partition a Moore-neighborhood boundary (or the 26-neighbor
> shell) into Face/Edge/Corner geometric classes by $O_h$ symmetry, then
> add/subtract small integers until the count matches a known experimental
> target — to four different targets: lepton mass ratios (§1), the Weinberg
> angle and strong coupling (§2), baryon/quark masses (§3), and the atomic
> spectrum/periodic-table shell structure (§4). The 2026-08-06 docs audit
> found this batch a genuine consolidation candidate: each section keeps its
> own worked example and its own specific tag/retraction provenance
> (per the audit's own caution — a merge "must not dilute [the] retraction
> provenance into one generic disclaimer"), but the shared technique, shared
> origin commit, and (for three of the four) shared retraction history
> belong in one document rather than four.
>
> **Shared provenance.** Three of the four original documents
> (`DERIV_LEPTON_MASS_GEOMETRY.md` §1, `DERIV_WEINBERG_STRONG_GEOMETRY.md`
> §2, `DERIV_BARYON_AND_QUARK_GEOMETRY.md` §3) received an unauthorized
> `[THEOREM]` promotion in commit `fdc483d0`/`24b31016` that was RETRACTED —
> §1 and §3 on 2026-07-01 (FTD-0348; adjudicated), §2 on 2026-08-06 (per
> LEDGER FTD-0018/FTD-0020, having been missed by the 2026-07-01 sweep). All
> three retractions found the same defect: the geometric reading is a
> substitution identity chosen to land on an already-known integer, not a
> forcing chain, and fails the FTD-0097/0189 look-elsewhere bar. The fourth
> (`DERIV_DISCRETE_ATOMIC_SPECTRUM.md`, §4) was never promoted to `[THEOREM]`
> — its header tag was always `[STRONGLY MOTIVATED CONJECTURE]` — but its
> body prose carried the same unhedged, derivation-implying rhetoric and was
> corrected the same day (2026-08-06) for that reason, not for a tag change.
>
> The four original documents are archived at
> `docs/theory/03_derivations/archive/closed_negative/` (§1–§3, the
> retracted-THEOREM trio) and `docs/theory/03_derivations/archive/resolved/`
> (§4, tag-unchanged rhetoric correction), each with a banner pointing here.
> No epistemic tag is changed by this consolidation; every tag below is
> copied verbatim from its source document.

---

## §1 · Lepton mass ratios (Muon 207, Tau 3477)

**Source:** `DERIV_LEPTON_MASS_GEOMETRY.md`. **Tag:** `[STRUCTURALLY MOTIVATED PARAMETRIC]` (corrected 2026-07-01, FTD-0348 — was `[THEOREM]`).

> **Correction notice.** The demotion of record for the μ/τ mass-ratio formulas
> (`TRACKER_OPEN_ITEMS.md` §"Lepton Mass Ratios": demoted to `[STRUCTURALLY MOTIVATED
> PARAMETRIC]` in `SPEC_SM_REPLACEMENT_COMPLETE.md` and `[IMPOSED]` in `FOUND_AXIOM_ZERO.md`,
> "as they lack a rigorous derivation from the core FTD lattice Lagrangian") was never
> propagated to this document's `[THEOREM]` header. The Fable specialist review
> (`AUDIT_FABLE_SPECIALIST_REVIEW_2026-07-01.md`) additionally found: (i) each move in the
> L₃-shell counting below (why L₃, why reject the 8 corners, why subtract exactly 3 "Dirac
> defect nodes") is chosen to land on 207 — a structural *rationalization* of a known
> integer, not a forcing derivation; (ii) attributing the residual 0.11% gap "strictly to
> QED vacuum polarization" is an unfalsifiable promissory note — both masses in the ratio
> are already dressed pole masses, no bare theory is defined in which a "bare 207" acquires
> a computable −0.112% QED shift, and no such computation is offered. The geometric reading
> below is retained as *motivation* at the corrected tag; the original document's claim to
> "retract the parametric approaches" in favor of a derivation is itself withdrawn.

In FTD, leptons are stable topological knots (flux loops) possessing spin-1/2 symmetry. To achieve stability at higher energies, the electron knot radially expands and phase-locks onto the larger $L_n$ lattice boundaries — a geometric *reading*, not a derivation, per the correction notice above.

### 1.1 The Muon Mass Ratio (207)

The Muon is the first radial expansion of the Lepton topological structure, phase-locking to the $L_3$ Moore boundary.

1. **Boundary Selection:** The $L_3$ boundary defines a $7 \times 7 \times 7$ cubic envelope. The total number of nodes on this boundary is $7^3 - 5^3 = 218$.
2. **Phase-Space Decomposition:** By the Moore Layer Theorem, the 218 boundary nodes rigorously decompose into:
   - **Faces** ($p$-block geometry): $6 \times (5 \times 5) = 150$ interior nodes.
   - **Edges** ($d$-block geometry): $12 \times 5 = 60$ interior nodes.
   - **Corners** ($f$-block geometry): 8 deep-transverse nodes.
3. **Lepton Resonant Confinement:** A localized lepton loop requires tight bounding. It exclusively occupies the connected Faces and Edges ($150 + 60 = 210$ phase nodes), strictly rejecting the 8 Corners which correspond to macroscopic spatial diffusion.
4. **Spin-1/2 Symmetry Breaking:** To establish a stable spin-1/2 quantization axis, the topological knot must explicitly break continuous spatial symmetry, consuming exactly 3 nodes corresponding to the Cartesian Triad ($N_c = 3$ dimensional degrees of freedom) as the Dirac defect core.
5. **Reading:**
   $210 \text{ (Confinement Nodes)} - 3 \text{ (Dirac Defect Nodes)} = 207$

The bare discrete mass ratio of the Muon reads as **207**. The minor 0.11% variance to the experimental dressed mass ($206.768$) was attributed to QED vacuum polarization in the original document — per the correction notice, this attribution is an unfalsifiable promissory note, not a computation.

### 1.2 The Tau Mass Ratio (3477)

The Tau is the extreme maximal resonance, pushed to the **$L_{12}$ boundary**.

1. **Boundary Selection:** The $L_{12}$ boundary (length 25) contains exactly $25^3 - 23^3 = 3458$ nodes.
2. **Generational Phase-Locking:** As the third generation, the Tau must maintain topological coherence with the inner Muon layer ($L_3$) to prevent immediate structural collapse. It achieves this by phase-locking the massive $L_{12}$ boundary specifically to the central $L_3$ Cartesian Cross.
3. **Cartesian Cross Capacity:** The exact number of nodes in a 3D Cartesian cross spanning the full $L_3$ volume is $7 + 7 + 7 - 2 = 19$ nodes.
4. **Reading:**
   $3458 \text{ (Boundary Nodes)} + 19 \text{ (Inner Phase-Lock)} = 3477$

The bare discrete mass ratio of the Tau reads as **3477**.

### 1.3 Honest framing

The original document's conclusion ("the parametric equations $3b_3(b_3+N_c)-N_c$ are formally abolished... the Muon and Tau are proven to not be arbitrary copies") is **not carried forward** — per the correction notice, this is exactly the overclaim the Fable review retracted. The Muon and Tau geometric readings above are motivation for the existing parametric formulas, not a replacement derivation.

---

## §2 · Weinberg angle and strong coupling

**Source:** `DERIV_WEINBERG_STRONG_GEOMETRY.md`. **Tag:** `[STRUCTURALLY MOTIVATED PARAMETRIC]` (corrected 2026-08-06 — was `[THEOREM: LATTICE PROJECTION]`).

> **Correction notice.** This document's 2026-06-18 `[THEOREM: LATTICE PROJECTION]`
> upgrade (commit `24b31016`) is **RETRACTED**, per LEDGER.md FTD-0018/FTD-0020
> (correction of record 2026-06-19, adjudicated): it is a substitution identity, not
> a forcing chain. It fails the FTD-0097/0189 look-elsewhere bar — a competitor
> ratio fits sin²θ_W better (2/9 at 0.31% vs this document's 3/13 at 3.5%) and
> another fits α_s better (2/17 at 0.29% vs 7/59 at 0.63%) — and the standing
> zero-promotion discipline. The document's own citation of "per FTD-0259" as
> justification was bogus: it collided with the real FTD-0259 (Mechanism-α), which
> says nothing about this claim. §1 and §3 of this document received this same
> correction on 2026-07-01; this section was missed by that sweep until the
> 2026-08-06 docs audit caught it. The Moore-layer geometric reading of $N_{eff}=13$,
> the Cartesian/face-diagonal/body-diagonal decomposition, and $b_3=7$ from the QCD
> beta function are real structural content and are retained below as *motivation*,
> not derivation — the canonical tags are `[STRUCTURALLY MOTIVATED PARAMETRIC]` for
> both sin²θ_W = 3/13 (FTD-0018) and α_s(M_Z) = 7/59 (FTD-0020).

### 2.1 The Weinberg Angle: 13-Axis Moore Projection

In the standard formulation of FTD, the Weinberg angle was posited as $\sin^2\theta_W = N_c/N_{eff} = 3/13 \approx 0.230769$.

**The Degrees of Freedom (The Denominator):** FTD operates on a 3D discrete lattice with a 26-connected Moore neighborhood. Every vector $\vec{v}$ to a neighbor has an antipodal counterpart $-\vec{v}$. Therefore, the number of independent spatial axes (effective degrees of freedom, $N_{eff}$) available for information propagation is exactly:
$$N_{eff} = \frac{26}{2} = 13 \text{ axes}$$

**The Cartesian Basis (The Numerator):** The 13 spatial axes uniquely decompose into:
- **3 orthogonal Cartesian axes** (face-centers of the bounding cube: $\pm x, \pm y, \pm z$)
- **6 2D face-diagonal axes** (edge-centers)
- **4 3D body-diagonal axes** (corners)

Total: $3 + 6 + 4 = 13$. The $SU(3)$ strong force (Color, $N_c = 3$) operates exclusively on the 3 orthogonal Cartesian axes.

**Electroweak Unification Geometry:** The weak mixing angle defines the projection between the electromagnetic $U(1)_Y$ and the weak $SU(2)_L$ forces. In FTD, the weak force (mediated by chirality flux) propagates across the *entire* 13-axis Moore stencil. Electromagnetism, as the Coulomb limit, is bound by the macroscopic orthogonal Cartesian geometry. Therefore, the weak mixing angle reads as the geometric projection of the orthogonal Cartesian sub-lattice onto the full Moore neighborhood:
$$\sin^2\theta_W = \frac{\text{Cartesian Axes}}{\text{Total Moore Axes}} = \frac{3}{13} \approx 0.230769$$
*(Standard Model experimental value: 0.2312. Error: 0.19%.)*

**Reading:** The factor $3/13$ has a geometric motivation as a projection of lattice anisotropy — but a competitor ratio (2/9) fits the CODATA value more closely (0.31% vs 3.5%), so this is not a forcing derivation.

### 2.2 The Strong Coupling: Dirac-Moore Fixed Point

The strong coupling at the Z-pole was posited as $\alpha_s(M_Z) = b_3 / (b_3 + 4N_{eff}) = 7/59 \approx 0.1186$. The denominator 59 was heavily criticized in prior audits ("59 is not structural; 2/17 fits better").

**The Gluon Anti-Screening Term ($b_3$):** From the standard QCD beta function, $b_3 = \frac{11 N_c - 2 n_f}{3}$. For FTD parameters ($N_c = 3, n_f = 6$), $b_3 = 7$.

**The Fermionic Vacuum Polarization ($4N_{eff}$):** A discrete Dirac spinor requires 4 complex components to support parity and matter-antimatter symmetry, so the total fermionic degrees of freedom available for vacuum polarization is $4 \times 13 = 52$.

**The Topological Fixed Point:** Combining the gluon and fermionic contributions:
$$\alpha_s(M_Z) = \frac{b_3}{b_3 + 52} = \frac{7}{59}$$

**Reading:** The arithmetic 7 + 52 = 59 is exact given the stated inputs, and $b_3=7$ is structurally motivated by the standard QCD beta function — but a competitor ratio (2/17) fits α_s(M_Z) more closely (0.29% vs 0.63%), and the 52 = 4×13 denominator term was not independently forced before the target was known.

---

## §3 · Baryon and quark masses

**Source:** `DERIV_BARYON_AND_QUARK_GEOMETRY.md`. **Tag:** proton ratio `[STRONGLY MOTIVATED CONJECTURE]` (via the prior $N_{eff}/\alpha$ formula) · the $L_9$ "knot" re-spelling `[PARAMETRIC]` · the six quark masses `[PARAMETRIC]` (all corrected 2026-07-01, FTD-0348 — was `[THEOREM]`).

> **Epistemic note (adjudicated).** The `[THEOREM]` promotion (commit `fdc483d0`)
> is **RETRACTED** — these are substitution identities, not forcing chains; they fail the
> FTD-0097/0189 look-elsewhere bar and the standing zero-promotion discipline.
> **Honest tags:** the proton mass ratio is a `[STRONGLY MOTIVATED CONJECTURE]` via the
> prior formula $m_p/m_e = N_{eff}/\alpha + N_{base}\cdot N_{eff} + N_c$ (≈173 ppm); the six
> quark masses are `[PARAMETRIC]` (tuned integer recipes; $m_t$ imports $Z=118$/Oganesson
> from chemistry). **Genuine motivation:** the prior proton formula uses three Moore integers
> *plus* $\alpha$, so it is harder to dismiss as a bare rational fit; the quark recipes have no
> such structural backing. `[THEOREM]` is re-earnable only behind a pre-registered
> look-elsewhere control.

In FTD, fractional charge ($\pm 1/3, \pm 2/3$) is associated with a geometric fracturing of the 3D Moore layers: quarks are incomplete topological defects that cannot exist independently and bind via $SU(3)$ color flux to form complete phase-space boundaries (baryons). The constructions below are recorded for provenance; their honest epistemic status is set per subsection.

### 3.1 The Proton Mass Ratio

**The motivated formula — `[STRONGLY MOTIVATED CONJECTURE]` (≈173 ppm):**
$$ \frac{m_p}{m_e} = \frac{N_{eff}}{\alpha} + N_{base}\cdot N_{eff} + N_c = 1781.47 + 52 + 3 = 1836.47, $$
a **173 ppm** match (5.8× experimental precision, ~30 ppm). Because it consumes three independent Moore integers *and* $\alpha$ (not a bare $p/q$), it sits at the master-quadratic's epistemic tier: `[STRONGLY MOTIVATED CONJECTURE]`, not a derivation. This is the preferred form. See `proof_proton_electron_ratio.py` / `proof_complete_sm.py`.

**The $L_9$ "phase-space knot" re-spelling — `[PARAMETRIC]`:** The integer $1836$ can also be **written** as
$$ \underbrace{6\times 17^2 + 12\times 17}_{1938\ (L_9\ \text{Faces}+\text{Edges})} \;-\; \underbrace{6\times 17}_{102\ (\text{"}SU(3)\text{ edge defect"})} = 1836. $$
This is **strictly less informative** than the preferred formula above: it is integer-only (zero sub-integer content — it cannot reproduce the $.15$ in the measured $1836.15$), and the objects it invokes ($L_9$ "bounded phase space", the number $1938$, and the $102 = 1938 - 1836$ "defect") did not exist in the framework before commit `fdc483d0` and were reverse-engineered to land on $1836$. It is a substitution identity. Do not cite it as a derivation; prefer the preferred form above.

### 3.2 The Quark Mass Spectrum — `[PARAMETRIC]` (all six)

**Reviewer / discipline flag.** All six quark masses are integer-combination *fits* with no independent structural derivation on the lattice. Each recipe (1–4 hand-selected terms) is chosen to land near a measured value; the reverse-engineering of integer combinations from experimental masses is exactly the fishing pattern the project's epistemic-discipline rules prohibit, and the family as a whole fails any look-elsewhere control (six tunable recipes over the integer lattice will hit six targets by construction).

| Quark | Recipe (as constructed) | Tag |
|---|---|---|
| Up | Unilateral Triad = 4 | `[PARAMETRIC]` (~5% off — outside even a loose band) |
| Down | $L_1$ Face = 9 | `[PARAMETRIC]` |
| Strange | $L_7$ Face + $L_2$ + Core = 183 | `[PARAMETRIC]` |
| Charm | $L_{10}$ + $L_2$ = 2484 | `[PARAMETRIC]` |
| Bottom | $L_{18}$ + $L_4$ + $L_1$ + Triad = 8170 | `[PARAMETRIC]` |
| Top | $L_{118}$ = 334170 (**imports $Z=118$/Oganesson from the periodic table** — an external chemistry input with no substrate basis; still ~1% off) | `[PARAMETRIC]` |

Correct reporting: "given the framework integers, integer-combination fits reproduce the six quark masses to a few percent." No theorem status.

### 3.3 Honest framing

The proton ratio is a `[STRONGLY MOTIVATED CONJECTURE]` carried by the prior $N_{eff}/\alpha$ formula; the $L_9$ knot re-spelling is a less-informative `[PARAMETRIC]` substitution identity; and the six quark masses remain `[PARAMETRIC]` integer fits with no structural derivation and a fatal look-elsewhere exposure. There is no derivation of the baryon/quark mass hierarchy here.

---

## §4 · Atomic spectrum and periodic-table shell structure

**Source:** `DERIV_DISCRETE_ATOMIC_SPECTRUM.md`. **Tag:** `[STRONGLY MOTIVATED CONJECTURE]` — **unaudited** (unchanged; this section's rhetoric was corrected 2026-08-06, not its tag).

> **Correction notice.** Unlike §1–§3 above, this section's tag was never
> promoted to `[THEOREM]` — it has carried `[STRONGLY MOTIVATED CONJECTURE]`
> since creation, matching its canonical listing in
> `docs/theory/07_assessment/REF_CLAIMS_MATRIX.md` (rows **ATOMIC-1**,
> **ATOMIC-2**, both explicitly marked "UNAUDITED, out of adjudication
> scope"). It carries no LEDGER `FTD-` id of its own — the LEDGER's only
> Helium entry, `FTD-0279`, is the unrelated mean-field SCF campaign
> (`ANALYSIS_HELIUM_LATTICE_SCF_v1.md`) and does not cover the claims below.
> What needed correcting was the body prose: despite the correct `[SMC]`
> tag, the original text asserted unhedged, derivation-implying language
> ("a strict geometric consequence," "not a consequence of spherical
> differential equations, but a direct enumeration," "upgraded to a native
> geometric prediction") stylistically identical to the pre-correction
> rhetoric §1–§3 had to retract. The screening parameter $\sigma_{FTD} =
> G^*/10$ and the $2/8/18/32$ shell-capacity split are post-hoc geometric
> readings chosen to match already-known targets (the Helium ground state,
> the periodic table) — the same pattern as §1's muon-207 rationalization.
> `REF_CLAIMS_MATRIX.md` records both claims with their own open caveats:
> ATOMIC-1 ">0.1% discrepancy unexplained"; ATOMIC-2 "Incompatible shell
> filling order observed."

This section offers a geometric *reading* of the multi-electron atomic spectrum and the periodic table shell capacities via the topological geometry of the 3D Moore neighborhood — motivation, not derivation. It bypasses continuous $\mathbb{R}^3$ variational techniques and spherical harmonics with a discrete counting exercise; whether that exercise reflects the actual architecture of the atom, rather than a post-hoc fit to already-known targets, is unaudited.

### 4.1 The Helium Ground State: $G^*/10$ Screening

In standard continuous quantum mechanics, the Helium ground state has no analytical solution due to the electron-electron Coulomb repulsion integral, requiring a continuous variational screening parameter $\sigma_{cont} = 5/16 = 0.3125$. This approximation yields a ground state energy of $-77.48$ eV, which is $1.5$ eV off from the experimental $-79.005$ eV.

In FTD, the Coulomb interaction is restricted to the **3 orthogonal Cartesian axes**, while the Dirac spinor phase space occupies all **$N_{eff} = 13$ spatial axes** of the Moore neighborhood (§2 above). When two electrons occupy the identical core node (parahelium, spins anti-aligned), they electrostatically screen each other via the remaining transverse axes, where the $1/r$ Cartesian singularity is regularized by the lattice flux:
$$\text{Transverse Axes} = N_{eff} - N_{Cartesian} = 13 - 3 = 10$$

The lattice flux integration measure is governed by the lemniscatic constant $G^* \approx 2.958675$. The FTD screening parameter reads as the total geometric flux distributed evenly across the 10 transverse screening axes:
$$\sigma_{FTD} = \frac{G^*}{10} \approx 0.2958675$$

The effective nuclear charge is $Z_{eff} = Z - \sigma_{FTD} = 2 - 0.2958675 = 1.7041325$, giving ground-state energy
$$E_0 = -2 (Z_{eff})^2 R_y = -5.808135 R_y = -79.023 \text{ eV}$$
*(Experimental: $-79.005$ eV. Error: **0.02%**.)* First ionization energy $E_I = |E_0| - 4 R_y = 24.600$ eV *(experimental $24.587$ eV, error 0.05%)*.

**Reading:** $\sigma_{FTD}=G^*/10$ reproduces the Helium ground state to 0.02% — but the 10-axis screening count was read off the Moore decomposition as a match to the already-known target energy, not independently forced beforehand, and `REF_CLAIMS_MATRIX.md`'s ATOMIC-1 row records an unresolved >0.1%-discrepancy caveat at higher precision. This is a numerical match under a post-hoc geometric reading, not a derivation of the discrete nature of atomic orbitals.

### 4.2 Geometric Reading of the Periodic Shell Structure

The standard model derives the capacities of the periodic table $(2, 8, 18, 32)$ from continuous spherical harmonics ($Y_l^m$ where $\sum_{l=0}^{n-1} 2(2l+1) = 2n^2$). A central node in the FTD 3D lattice is bounded by 26 neighbors, decomposing into **6 Face-centers** (Cartesian orthogonal), **8 Corners** (3D body-diagonals), **12 Edge-centers** (2D face-diagonals).

- **$n=1$ (Capacity 2):** the Central Void/Core Node supports exactly 2 anti-aligned states.
- **$n=2$ (Capacity 8):** the **8 body-diagonal corners** of the Moore bounding box.
- **$n=3$ (Capacity 18):** the **12 edge-centers** plus the **6 face-centers**, $12+6=18$.
- **$n=4$ (Capacity 32):** the $L_2$ boundary contains $5^3-3^3=98$ nodes, decomposing into higher-order parity classes that "natively support" the 32-state capacity.

**Reading:** The $2/8/18/32$ capacities can be read off the Moore bounding-box partition as shown above — but this is a post-hoc match to the already-known $2n^2$ sequence (the $n=4$ count in particular is reached via a next-nearest-neighbor node count, $5^3-3^3=98$, that "decomposes into higher-order parity classes" without an independent derivation of which 32 of those 98 nodes are selected), and `REF_CLAIMS_MATRIX.md`'s ATOMIC-2 row records an incompatible shell-filling order as an unresolved caveat. This is a geometric *reading* of the Periodic Table's integer structure, not a direct enumeration that supersedes the spherical-harmonics account.

### 4.3 Honest framing

1. The Helium ground-state match is a post-hoc geometric reading, not an established prediction (`REF_CLAIMS_MATRIX.md` ATOMIC-1, unaudited).
2. The $2n^2$ shell structure is read against the integer partitions of the Moore bounding box $(2 \text{ core}, 8 \text{ corners}, 18 \text{ faces/edges})$, not shown to be forced by them (`REF_CLAIMS_MATRIX.md` ATOMIC-2, unaudited).

---

## §5 · What this document is, as a whole

Four independent applications of one reused technique (Moore-neighborhood boundary/shell counting by $O_h$ symmetry class, tuned post-hoc to a known target) to four unrelated physics targets. None is a forcing derivation. §1–§3 carry an explicit adjudicated retraction record; §4 carries an unaudited-but-unpromoted `[SMC]` tag with corrected rhetoric. Reading any section as evidence for the others is not warranted — the shared technique's repeated success at landing near known targets is itself the pattern the FTD-0097/0189 look-elsewhere discipline exists to catch, not independent confirmation.
