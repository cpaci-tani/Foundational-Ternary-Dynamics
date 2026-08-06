# FTD-0415 — Lorentz Radiative-Closure Operator Inventory

**Date:** 2026-07-22  
**Status:** `[THEOREM — symmetry permission and invariant count]` + `[DERIVED — minimum tuning count]` + `[POWER COUNTING — not an FTD loop calculation]` + `[OPEN — interacting matching]`  
**Verdict:** `MARGINAL-OPERATORS-ALLOWED; RADIATIVE-PROTECTION-NOT-SHOWN`  
**Verifier:** `scripts/proofs/proof_lorentz_radiative_operator_inventory.py`

---

## 0. Outcome

FTD-0414 makes the selected free tree-level speed spread proportional to
`(ka)^4`. With either documented `a=ell_P` calibration, that direct term is
tiny at observed energies. This does **not** make the suppression technically
natural.

The exact substrate symmetries used here—translations, spatial `O_h`, parity,
CPT, and gauge symmetry where applicable—permit independent CPT-even
dimension-four temporal and spatial kinetic operators. Spatial cubic symmetry
equalizes the three axes for scalar and gauge limiting speeds; it does not
exchange time with space and does not equate different species. For a spatial
vector field it permits an additional cubic-only marginal gradient invariant.

Therefore the leading empirical Lorentz risk is not the known direct q4 term.
It is the uncomputed coefficient of every allowed dimension-four
preferred-frame operator after interactions and blocking.

---

## 1. Exact symmetry inventory

Let `n^mu=(1,0,0,0)` denote the preferred tick direction. The lattice has no
exact operation that rotates `n` into a spatial axis. Consequently, whenever
a continuum field description exists, the following pairs are separately
allowed.

### 1.1 Gauge field

In the preferred frame,

$$
\mathcal L_A=\frac{Z_E}{2}\mathbf E^2
-\frac{Z_B}{2}\mathbf B^2.
$$

Both terms are gauge invariant, `O_h` scalars, parity even, CPT even, and of
dimension four. Their ratio gives the photon limiting speed,
`c_A^2=Z_B/Z_E`. No listed exact symmetry requires `Z_E=Z_B`.

FTD-0114 does not supply the missing relation. Its proved content is the
metric-independent differential identity `d²=0` (the lattice Bianchi
identities). That does not define the constitutive Hodge star or make the
action invariant under electric-magnetic interchange. Its former stronger
duality wording is corrected concurrently with FTD-0415.

### 1.2 Dirac matter

For every matter irrep or species `r`,

$$
\mathcal L_{\psi_r}=
Z_{t,r}\,\bar\psi_r i\gamma^0D_0\psi_r
+Z_{s,r}\,\bar\psi_r i\gamma^iD_i\psi_r-m_r\bar\psi_r\psi_r.
$$

The two kinetic terms are separately allowed and CPT even. Field
normalization rescales both coefficients together and cannot remove
`Z_{s,r}/Z_{t,r}`. A common time-unit choice can fix one sector's cone, but
every remaining independent ratio is observable relative to it.

### 1.3 Scalar mode

For any scalar carrier,

$$
\mathcal L_\phi=\frac{Z_{t,\phi}}2(\partial_t\phi)^2
-\frac{Z_{s,\phi}}2\sum_i(\partial_i\phi)^2.
$$

Again, `O_h` fixes equality among spatial axes but leaves
`Z_{s,\phi}/Z_{t,\phi}` free.

### 1.4 Native spatial vector

For `A_ij=partial_i J_j`, four linearly independent spatial quadratic
invariants are allowed by the signed-permutation representation of `O_h`:

$$
\operatorname{tr}(A^TA),\qquad
(\operatorname{tr}A)^2,\qquad
\operatorname{tr}(A^2),\qquad
\sum_i A_{ii}^2.
$$

The verifier proves invariance under all 48 signed permutations and proves
linear independence with an exact nonzero determinant. The fourth term is not
`SO(3)` invariant: for `A=diag(1,0,0)`, a 45-degree rotation changes it from
`1` to `1/2`. Thus cubic symmetry permits a marginal spatial anisotropy for a
vector field even before higher-derivative lattice artifacts are considered.

This is a permission theorem, not a claim that every coefficient is generated
or nonzero in the current engine.

---

## 2. Minimum tuning statement

For a gauge sector plus `N` inequivalent matter species, there are at least
`N+1` independent limiting-speed ratios before choosing units. One global
rescaling of time removes one ratio, leaving at least `N` physically relative
cones. Scalars, gravity, bound-state clocks, and the extra native-vector
gradient structures add further independent coefficients unless an exact
symmetry or a calculated RG attraction relates them.

FTD-0413's tree choice aligns one selected flux clock with one selected Wilson
matter clock through q4. It does not force the marginal ratios above to remain
equal. The alignment is therefore a tree-level matching condition, not a
custodial symmetry.

---

## 3. Collins percolation exposure

If a higher-derivative Lorentz-breaking coefficient is written `C6/M^2`, a
preferred-frame loop cutoff `Lambda` permits the power-counting contribution

$$
\delta c_4\sim\frac{g^2}{16\pi^2}C_6\frac{\Lambda^2}{M^2}.
$$

For `Lambda` and `M` both of order `a^-1`, the external factor `(ka)^4`
disappears. The correction is loop suppressed, not Planck suppressed by the
observed particle energy. CPT does not forbid it because the target operators
are CPT even.

This is the naturalness mechanism highlighted by Collins, Perez, Sudarsky,
Urrutia, and Vucetich in
[PRL 93, 191301 (2004)](https://arxiv.org/abs/gr-qc/0403053). Their example
does not calculate an FTD loop coefficient, and neither does the verifier in
this audit. The exact conclusion established here is narrower: FTD's declared
symmetries do not forbid the counterterms to which that mechanism would feed.

---

## 4. What would close LR-3

Radiative closure requires one frozen interacting action or transfer object
covering the selected flux and matter carriers, followed by either:

1. a 1PI two-point calculation extracting every allowed dimension-three/four
   coefficient and its mixing with the q6/q8 lattice operators; or
2. a nonperturbative blocking measurement of the same complete coefficient
   basis.

A positive result must show one of the following, quantitatively:

- an exact custodial symmetry relating the temporal and spatial terms;
- an RG-attractive common cone with a convergence rate sufficient for bounds;
- a hierarchy between the Lorentz-breaking and interaction-mediation scales;
- or explicit counterterm tuning, counted as an imposed calibration.

The statements “the lattice correction is CPT even,” “the direct term starts
at q4,” and “the lattice spacing is Planckian” do not close this gate.

---

## 5. Status table

| Claim | Status |
|---|---|
| Independent gauge `E^2`/`B^2` coefficients are allowed | `[THEOREM — symmetry permission]` |
| Independent matter/scalar time-space coefficients are allowed | `[THEOREM — symmetry permission]` |
| Four independent `O_h` vector-gradient quadratics exist | `[THEOREM — exact enumeration]` |
| At least one relative-cone parameter per additional species remains after fixing units | `[DERIVED]` |
| `delta c4 ~ (g^2/16pi^2) C6 Lambda^2/M^2` | `[POWER COUNTING]` |
| A nonzero FTD loop coefficient of that size | not calculated |
| Radiatively stable common cone | `[OPEN — HARD]` |

Exact microscopic Lorentz symmetry is not required. Technical control of the
allowed marginal preferred-frame coefficients is.

---

## 6. FTD-0416--0419 successors

`AUDIT_LORENTZ_RG_ATTRACTION.md` evaluates the strongest standard perturbative
escape as an explicitly external IR surrogate. The anisotropic-QED common cone
is attractive, but simultaneous charge running limits the suppression to
`(alpha_IR/alpha_UV)^((N_f+2)/N_f)`, no better than `1/137^3` under its stated
optimistic perturbative assumptions. It also proves that the selected
`A=P_T J` connection map is spatially nonlocal. The complete FTD full-BZ
coefficient matrix required in §4 remains open.

`AUDIT_LORENTZ_LOCAL_LINK_FLUX.md` then freezes a separate exactly local gauge
candidate by adopting an independent link connection. Its unit-plaquette
action supplies a stable photon pole at selected `c_A²=1/7`, but loses the
q4-improved free comparison and does not yet have a conserved ternary-history
current.

`AUDIT_LORENTZ_SPACETIME_WILSON.md` (FTD-0418) adds the compatible one-tick
nearest-neighbour Wilson regulator, lifts all 15 non-origin spacetime corners,
and derives the complete one-/two-photon Ward vertices plus a local gauge-
fixed propagator. The gauge-matter regulator is therefore frozen on the
selected Branch-B local-link branch.

`AUDIT_LORENTZ_FULL_BZ_MATCHING.md` (FTD-0419) evaluates one complete `xi=1`
QED_L-like step scheme. The Ward-complete result is
`delta_match/g²=-0.32696906(5)`, closing automatic one-loop cancellation
negative and requiring a dimension-four counterterm in that scheme. The
coefficient is off-shell and scheme-specific; it does not yet supply the
gauge-independent pole match, real-time unitarity, or ternary current required
for a physical verdict.
