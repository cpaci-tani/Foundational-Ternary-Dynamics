# The Discrete Atomic Spectrum and Shell Structure (ARCHIVED — merged 2026-08-06)

> **Merged into `../standard_model/DERIV_GEOMETRIC_MASS_RATIO_READINGS.md` §4
> (2026-08-06),** alongside three sibling documents from the same 2026-06-18
> batch applying the same technique to different targets. Content preserved
> verbatim there; kept here for provenance per the doc-cleanup skill.

**Status:** Foundational theory [STRONGLY MOTIVATED CONJECTURE] **(unaudited — correction notice added 2026-08-06)**

> **Correction notice.** This document was introduced in commit `24b31016` (2026-06-18),
> the same batch commit that introduced three sibling "geometric reading" documents —
> `DERIV_LEPTON_MASS_GEOMETRY.md`, `DERIV_WEINBERG_STRONG_GEOMETRY.md`, and
> `DERIV_BARYON_AND_QUARK_GEOMETRY.md` — all three of which later received a correction
> (2026-07-01 and 2026-08-06) retracting an unauthorized `[THEOREM]` promotion of the
> same post-hoc geometric-reading pattern: a substitution identity chosen to land on an
> already-known target, not a forcing chain (see those documents' own correction notices,
> citing commit `24b31016`/`fdc483d0` and LEDGER FTD-0018/FTD-0020). Verified via
> `git log --follow`: unlike its three siblings, this document's own Status line was never
> promoted past `[STRONGLY MOTIVATED CONJECTURE]` — the fdc483d0/24b31016 THEOREM-retraction
> provenance therefore does not literally apply to *this* file's header tag, which already
> matches its canonical listing in `REF_CLAIMS_MATRIX.md` (rows **ATOMIC-1**, **ATOMIC-2**,
> both explicitly marked "UNAUDITED, out of adjudication scope") and
> `INDEX_03_DERIVATIONS.md`. This document carries no LEDGER `FTD-` id of its own — the
> LEDGER's only Helium entry, `FTD-0279`, is the unrelated mean-field SCF campaign
> (`ANALYSIS_HELIUM_LATTICE_SCF_v1.md`) and does not cover the claims below.
>
> The overclaim here is in the body prose, not the header tag: despite the correct `[SMC]`
> status, the text below asserts unhedged, derivation-implying language ("a strict
> geometric consequence," "not a consequence of spherical differential equations, but a
> direct enumeration," "upgraded to a native geometric prediction") stylistically
> identical to the pre-correction rhetoric its siblings had to retract. The screening
> parameter $\sigma_{FTD} = G^*/10$ and the $2/8/18/32$ shell-capacity split are post-hoc
> geometric readings chosen to match already-known targets (the Helium ground state, the
> periodic table) — the same pattern the Fable specialist review found in the muon-207
> "$L_3$-shell" rationalization of `DERIV_LEPTON_MASS_GEOMETRY.md`: each move chosen to
> land on a known integer, not independently forced before the target was known.
> `REF_CLAIMS_MATRIX.md` records both claims with their own open caveats: ATOMIC-1
> ">0.1% discrepancy unexplained"; ATOMIC-2 "Incompatible shell filling order observed."
> The geometric readings below are retained as *motivation*, not derivation.

> **Epistemic Note:** This document offers a geometric *reading* of the multi-electron atomic spectrum and the periodic table shell capacities via the topological geometry of the 3D Moore neighborhood — motivation, not derivation, per the correction notice above. It bypasses continuous $\mathbb{R}^3$ variational techniques and spherical harmonics with a discrete counting exercise; whether that exercise reflects the actual architecture of the atom, rather than a post-hoc fit to already-known targets, is unaudited.

---

## 1. The Helium Ground State: $G^*/10$ Screening

In standard continuous quantum mechanics, the Helium ground state has no analytical solution due to the electron-electron Coulomb repulsion integral, requiring a continuous variational screening parameter $\sigma_{cont} = 5/16 = 0.3125$. This approximation yields a ground state energy of $-77.48$ eV, which is $1.5$ eV off from the experimental $-79.005$ eV.

### 1.1 The Lattice Topology of Coulomb Screening
In FTD, the Coulomb interaction is restricted to the **3 orthogonal Cartesian axes**, while the Dirac spinor phase space occupies all **$N_{eff} = 13$ spatial axes** of the Moore neighborhood (see `DERIV_WEINBERG_STRONG_GEOMETRY.md`). 

When two electrons occupy the identical core node (parahelium, spins anti-aligned), they electrostatically screen each other. However, this screening is not a continuous integral; it occurs precisely because the superposition of the two electrons "leaks" into the remaining transverse axes where the $1/r$ Cartesian singularity is regularized by the lattice flux.

The number of transverse (non-Cartesian) screening axes is:
$$\text{Transverse Axes} = N_{eff} - N_{Cartesian} = 13 - 3 = 10$$

### 1.2 The Analytical Screening Parameter
The lattice flux integration measure is universally governed by the lemniscatic constant $G^* \approx 2.958675$ (the discrete lattice analogue of $\pi$). The exact FTD screening parameter is the total geometric flux distributed evenly across the 10 transverse screening axes:
$$\sigma_{FTD} = \frac{G^*}{10} \approx 0.2958675$$

### 1.3 Ground State Energy
Using the exact discrete screening parameter, the effective nuclear charge is:
$$Z_{eff} = Z - \sigma_{FTD} = 2 - 0.2958675 = 1.7041325$$

The ground state energy is:
$$E_0 = -2 (Z_{eff})^2 R_y = -5.808135 R_y$$
With $R_y = 13.605693$ eV, we obtain:
$$\boxed{E_0 = -79.023 \text{ eV}}$$

*(Experimental true value: $-79.005$ eV. Error: **0.02%**)*

The first ionization energy:
$$E_I = |E_0| - 4 R_y = 1.808135 R_y = 24.600 \text{ eV}$$
*(Experimental true value: $24.587$ eV. Error: **0.05%**)*

**Reading:** $\sigma_{FTD}=G^*/10$ reproduces the Helium ground state to 0.02% — but per the correction notice above, the 10-axis screening count was read off the Moore decomposition as a match to the already-known target energy, not independently forced beforehand, and `REF_CLAIMS_MATRIX.md`'s ATOMIC-1 row records an unresolved >0.1%-discrepancy caveat at higher precision. This is a numerical match under a post-hoc geometric reading, not a derivation of the discrete nature of atomic orbitals.

---

## 2. Geometric Reading of the Periodic Shell Structure

The standard model derives the capacities of the periodic table $(2, 8, 18, 32)$ from continuous spherical harmonics ($Y_l^m$ where $\sum_{l=0}^{n-1} 2(2l+1) = 2n^2$). In FTD, the electron shells are structural equivalence classes of the Moore lattice itself.

A central node in the FTD 3D lattice is bounded by 26 neighbors, which uniquely decompose into three symmetry classes:
- **6 Face-centers** (Cartesian orthogonal)
- **8 Corners** (3D body-diagonals)
- **12 Edge-centers** (2D face-diagonals)
Total = 26 neighbors.

### 2.1 The $n=1$ Shell (Capacity: 2)
The $n=1$ shell ($1s$) corresponds to the **Central Void / Core Node** itself. Due to the ternary state algebra and parity inversion, the central node can support exactly 2 anti-aligned states (spin up and spin down).

### 2.2 The $n=2$ Shell (Capacity: 8)
The $n=2$ shell (traditionally $2s, 2p$) corresponds geometrically to the **8 body-diagonal corners** of the Moore bounding box. By occupying the 8 corners, the lattice perfectly supports 8 fermionic states without requiring angular momentum quantum numbers. 
$$\text{Lattice Geometry Capacity: } 8 \equiv n=2 \text{ Shell}$$

### 2.3 The $n=3$ Shell (Capacity: 18)
The $n=3$ shell (traditionally $3s, 3p, 3d$) corresponds geometrically to the sum of the remaining boundary nodes: the **12 edge-centers** plus the **6 face-centers**. 
$$\text{Lattice Geometry Capacity: } 12 + 6 = 18 \equiv n=3 \text{ Shell}$$

### 2.4 The $n=4$ Shell (Capacity: 32)
The $n=4$ shell ($s, p, d, f$) maps to the next discrete Brillouin zone boundary (next-nearest neighbors). The $L_2$ boundary of a 3D Moore lattice contains exactly $5^3 - 3^3 = 125 - 27 = 98$ nodes, which decompose into higher-order parity classes that natively support the 32-state capacity.

**Reading:** The $2/8/18/32$ capacities can be read off the Moore bounding-box partition as shown above — but per the correction notice, this is a post-hoc match to the already-known $2n^2$ sequence (the $n=4$ count in particular is reached via a next-nearest-neighbor node count, $5^3-3^3=98$, that "decomposes into higher-order parity classes" without an independent derivation of which 32 of those 98 nodes are selected), and `REF_CLAIMS_MATRIX.md`'s ATOMIC-2 row records an incompatible shell-filling order as an unresolved caveat. This is a geometric *reading* of the Periodic Table's integer structure, not a direct enumeration that supersedes the spherical-harmonics account.

---

## 3. Epistemic Impact
This document offers a geometric *reading* that relates the phenomenological $N$-body quantum atomic problem to discrete lattice topology — motivation, not derivation, per the correction notice above.

1. The Helium ground-state match is a post-hoc geometric reading, not an established prediction (`REF_CLAIMS_MATRIX.md` ATOMIC-1, unaudited).
2. The $2n^2$ shell structure is read against the integer partitions of the Moore bounding box $(2 \text{ core}, 8 \text{ corners}, 18 \text{ faces/edges})$, not shown to be forced by them (`REF_CLAIMS_MATRIX.md` ATOMIC-2, unaudited).
