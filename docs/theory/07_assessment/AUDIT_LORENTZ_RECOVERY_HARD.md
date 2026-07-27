# Audit — Lorentz Recovery Hard Gate

**Registry:** FTD-0407  
**Status:** `[SCOPED NO-GO — current default free-flux update]` + `[OPEN — interacting and multi-sector recovery]`  
**Verdict:** `LEADING-ORDER-FLUX-ONLY`  
**Recomputing verifier:** [`proof_lorentz_recovery_hard.py`](../../../scripts/proofs/proof_lorentz_recovery_hard.py) (27/27 exact/source-contract checks)

---

## 0. Verdict

FTD has established a useful but much narrower fact than Lorentz recovery:

> The production 18-point spatial Laplacian has an isotropic quartic term, so directional phase-speed differences first appear at order `|q|^4`.

That is restoration of **spatial rotations** in the long-wavelength free-flux sector. It is not restoration of boosts. Combining the spatial operator with the engine's actual discrete-time update exposes an earlier, rotationally invariant but boost-violating correction:

\[
\theta^2
=\frac13 S_2-\frac1{54}S_2^2-\frac{11}{9720}S_2^3
+\frac1{216}S_2Q_4-\frac1{270}Q_6+O(|q|^8),
\]

where `q_i = a k_i`, `theta = omega dt`, and `S_2,Q_4,Q_6` are the pure even-power sums defined in §2. The `S_2^2` term is a dimension-six preferred-frame correction in a 3+1-dimensional quadratic action. The old anisotropy campaign is blind to it because it compares spatial directions at fixed `|q|`; `S_2^2` is identical in every direction.

The current status is therefore:

| Claim | Independent status |
|---|---|
| Quartic spatial term is rotationally invariant | `[THEOREM — production stencil]` |
| Directional phase-speed spread begins as `|q|^4` | `[DERIVED — free spatial symbol]` |
| Free flux has one relativistic cone at leading quadratic order | `[DERIVED — leading long-wavelength term]` |
| Current default update is Lorentz covariant through dimension six | **False**; exact coefficient is `-S_2^2/54` |
| `C_WAVE=1/sqrt(3)` is uniquely fixed by CFL saturation of the production stencil | **False**; exact ceiling is `C_WAVE^2 <= 3/4` |
| Stable dimension-six cancellation is reachable by retuning `C_WAVE` in the current update | **False**; cancellation needs `r^2=1`, stability needs `r^2<=3/4` |
| A finite-range explicit escape exists | `[THEOREM — candidate outside current axioms]`; the radius-two witness in §3.1 is stable at `r=1` and cancels dimension six, but violates P4's nearest-Moore dependency bound |
| A P4-preserving temporal escape exists | `[THEOREM — free linear pole]` + `[SELECTED IMPLEMENTATION PROTOTYPE]`; FTD-0408 alternates kicks `+3/13,-1/13`, is full-band stable, and cancels dimension six, but changes the leading flux speed to `1/sqrt(13)` |
| Flux, manifested matter, Wilson matter, and gravity share one renormalized cone | `[PARTIAL/OPEN]`; FTD-0413 aligns selected free flux and Wilson matter through q4, while manifested matter, q6, gauge, gravity, interactions, and production integration remain open |
| CPT-even dimension-six breaking is radiatively protected | **False as a protection argument**; CPT-even dimension-four preferred-frame operators are allowed |
| Physical Lorentz invariance has been recovered | `[OPEN]` |

All statements above are finite-lattice statements. The Taylor coefficients are exact derivatives of a trigonometric polynomial. For any fixed `q_0 < pi`, Taylor's theorem supplies a finite bound `|R_8(q)| <= C(q_0)|q|^8` on `|q_i| <= q_0`; no infinite-volume ontology is required.

---

## 1. Source contract and exact update

The default free, single-substrate path is fixed by four source facts:

1. [`field_operators.h`](../../../engine/include/ftd/field_operators.h) applies
   `(1/3) face_sum + (1/6) edge_sum - 4 center`.
2. With the FTD-0408 prototype toggle OFF (the default),
   [`phase_read.cpp`](../../../engine/src/render_bridge_phases/phase_read.cpp)
   sets `delta_j = C_WAVE^2 * lap`.
3. [`phase_write.cpp`](../../../engine/src/render_bridge_phases/phase_write.cpp) performs the unit-step kick-drift update
   `wave_vel += delta_j; flux += wave_vel`.
4. [`term_toggles.h`](../../../engine/include/ftd/term_toggles.h) defaults to `FULL`, with both alternate wave integrators off.

Eliminating `wave_vel` gives the exact second-order recurrence

\[
J^{n+1}-2J^n+J^{n-1}=c_{\rm lat}^2\,\Delta_{18}J^n.
\]

For a finite-torus Fourier mode `J^n_x = J_0 exp[i(q dot x-theta n)]`, this becomes

\[
4\sin^2\!\left(\frac{\theta}{2}\right)=r^2 M_{18}(q),
\qquad r:=\frac{c\,\Delta t}{a},
\]

with the exact positive spatial symbol

\[
M_{18}(q)=4-\frac23\sum_i\cos q_i
-\frac23\sum_{i<j}\cos q_i\cos q_j.
\]

This fully discrete relation, not `omega^2=c^2 M_18`, is the dispersion relation of the default engine update.

---

## 2. Exact low-momentum expansion

Define

\[
\begin{aligned}
S_2&=q_x^2+q_y^2+q_z^2,\\
Q_4&=q_x^4+q_y^4+q_z^4,\\
Q_6&=q_x^6+q_y^6+q_z^6.
\end{aligned}
\]

Direct expansion of the exact symbol gives

\[
M_{18}=S_2-\frac{S_2^2}{12}
+\frac{S_2Q_4}{72}-\frac{Q_6}{90}+O(|q|^8).
\]

The quartic term is exactly rotationally invariant. The first cubic-direction dependence is the complete `S_2Q_4/72-Q_6/90` tensor, at sixth order in `M_18`. This is the valid content of the historical `p approximately 4` anisotropy result: because phase speed divides by `|q|`, a sixth-order symbol difference becomes a fourth-order relative phase-speed difference. Earlier versions used the name `Q_6` for a different cubic discriminator; FTD-0414 replaces that collision with the explicit pure-power basis above.

Solving the **fully discrete** relation order by order gives

\[
\boxed{
\theta^2=r^2S_2
+\frac{r^2(r^2-1)}{12}S_2^2
+\frac{r^4(4r^2-5)}{360}S_2^3
+\frac{r^2}{72}S_2Q_4-\frac{r^2}{90}Q_6+O(|q|^8)
}.
\]

At the production value `r^2=1/3`,

\[
\theta^2=\frac13S_2-\frac1{54}S_2^2
-\frac{11}{9720}S_2^3+\frac1{216}S_2Q_4
-\frac1{270}Q_6+O(|q|^8).
\]

The hierarchy is consequently:

- `S_2`: common leading cone;
- `S_2^2`: rotationally invariant **boost violation**, dimension six in 3+1 dimensions;
- `S_2Q_4` and `Q_6`: the complete cubic rotational violation at dimension eight in 3+1 dimensions.

The old Phase-1A directional comparison measured the last item and could not constrain the first Lorentz-violating item.

---

## 3. CFL correction and a scoped no-go

Write `a_i=cos q_i`. The spatial symbol is multi-affine on the cube `a_i in [-1,1]`, so its extrema occur at the eight vertices. Exact evaluation gives

\[
M_{18}^{\max}=M_{18}(\pi,\pi,0)=\frac{16}{3}.
\]

The centered explicit recurrence is stable only if

\[
r^2M_{18}^{\max}\le4
\quad\Longrightarrow\quad
r^2\le\frac34.
\]

Thus `r^2=1/3` is stable but does **not** saturate the production stencil's CFL bound. The value `1/sqrt(3)` came from the six-neighbor Laplacian's maximum eigenvalue `12`, while the engine of record uses the normalized 18-point operator with maximum `16/3`.

The dimension-six coefficient vanishes nontrivially only at

\[
r^2=1.
\]

That lies outside the stability interval. Therefore:

> **Scoped no-go.** No choice of `C_WAVE` makes the current nearest-time, production-18-point explicit update both stable and free of the dimension-six on-shell dispersion correction.

The obstruction is not peculiar to the selected 18-point weights. Let `(f,e,h)` be arbitrary face, edge, and corner weights on the 26-neighbor Moore shell. Unit normalization and quartic rotational isotropy require

\[
f+4e+4h=1,\qquad e+2h=\frac16.
\]

Every member of this family obeys

\[
M(\pi,\pi,0)=8f+16e=\frac{16}{3}.
\]

Hence no normalized, quartic-isotropic nearest-Moore spatial stencil can repair the conflict while retaining the same centered explicit time symbol. Recovery requires a different temporal stencil/auxiliary update, spatial reach beyond the nearest Moore shell, or a different local spacetime architecture.

### 3.1 Constructive radius-two escape — exact but outside P4

The no-go is sharp rather than generic to every finite lattice. Let

\[
u_i=1-\cos q_i,\qquad
O_{21}=\sum_{i\ne j}u_i^2u_j,
\]

and define

\[
M_{R2}=M_{18}-\frac14O_{21}+\frac98u_xu_yu_z.
\]

The added terms begin at sixth order in momentum, so

\[
M_{R2}=S_2-\frac1{12}S_2^2+O(|q|^6).
\]

An exact tensor-Bernstein certificate on 64 rational sub-boxes of `u_i in [0,2]` proves

\[
0\le M_{R2}(q)\le4
\]

throughout the Brillouin zone. The upper bound is attained, while `M_R2(pi,pi,pi)=1`, so the construction does not create an all-corner zero. With the same centered three-level time update and `r=1`, it is therefore stable and its full pole satisfies

\[
\theta^2=S_2+O(|q|^6),
\]

with the complete dimension-six correction cancelled. The surviving sixth-order term in `theta^2` is cubic rather than Lorentz invariant, so this advances the free pole only through LR-1; it does not complete Lorentz recovery.

The escape has a nonzero `cos^2(q_x)cos(q_y)` coefficient. Its real-space support therefore reaches displacement `(2,1,0)` and permutations: Chebyshev radius two. Applying it in one tick violates P4's one-Moore-shell dependency rule and doubles the microscopic support speed. This gives a concrete architecture choice rather than a verbal one:

- keep P4 and design a stable auxiliary/local-time architecture that realizes the same improved pole without a radius-two one-tick dependency; or
- amend P4 to finite radius two, adopt `M_R2`, change the clock calibration from `r^2=1/3` to `r^2=1`, and re-run every causality/common-cone gate.

No production code adopts this witness. It is an exact existence proof and a minimal research target.

### 3.2 P4-preserving period-two escape — implemented prototype

FTD-0408 closes the architecture choice left open above without extending the
one-tick spatial reach. Alternating the existing local kick coefficient as
`kappa_even=3/13`, `kappa_odd=-1/13` gives the exact two-tick pole

\[
\sin^2\theta=\frac{M_{18}}{13}+\frac{3M_{18}^2}{676}.
\]

The right-hand side is bounded by `272/507<1` on the full production band and
its low-momentum expansion is `theta²=S2/13+O(q^6)`, with the entire quartic
term absent. The effective `M18²` is generated by two consecutive P4-local
steps; no tick reads beyond the nearest Moore shell.

This does not close Lorentz recovery. It introduces a selected period-two clock
and alternate anti-kick, leaves dimension-eight cubic breaking, and changes the
bare flux speed from `1/sqrt(3)` to `1/sqrt(13)`. LR-1 passes for the default-off
CPU free-linear prototype; LR-2 immediately fails pending common-cone
recalibration. See [`AUDIT_LORENTZ_P4_PERIOD2.md`](AUDIT_LORENTZ_P4_PERIOD2.md).

---

## 4. Why the existing correlator benchmark is insufficient

[`benchmark_lorentz_recovery.cpp`](../../../engine/tests/benchmark_lorentz_recovery.cpp) compares one axis-aligned plane wave's temporal and spatial correlators after rescaling by the leading speed. That is a free-mode dispersion diagnostic. It does not test a boost transformation, an interacting two-point function, or equality of limiting speeds across species.

For the benchmark mode the exact relation is

\[
\theta(k)=2\arcsin\!\left[r\sin(k/2)\right],
\]

not `theta=2r sin(k/2)`. At fixed mode number the mismatch accumulates with separation; fitting its pointwise residual to a decreasing power of `r` inside one lattice is not a general Lorentz-recovery exponent. A scaling claim requires comparing a locked observable across a sequence of smaller dimensionless momenta, with the exact temporal symbol included.

Passing that benchmark is necessary evidence that the free flux update follows its own dispersion. It is not evidence that the interacting theory is Lorentz covariant.

---

## 5. Common-cone gate is partial, not closed

Lorentz recovery requires one operational cone for every observable low-energy sector, not one cone per module.

### 5.1 Imported Wilson matter

The standalone [`wilson_dirac.h`](../../../engine/include/ftd/wilson_dirac.h)
module uses an independent continuous-time clock and is not integrated into
`RenderBridge`. FTD-0412 later corrected its real-time operator: the historical
code evolved spatial `D_W` directly and its special-spinor norm test did not
establish an energy spectrum. Real-time evolution now uses a Hermitian Wilson
Hamiltonian with explicit `spatial_speed`, whose default remains `1`. For a
massless axis mode with `a=r_W=c_s=1`, the corrected exact free dispersion is

\[
E^2=\sin^2q+(1-\cos q)^2
=q^2-\frac{q^4}{12}+O(q^6),
\]

so its default raw leading speed is `1`, while the default flux speed is
`1/sqrt(3)`. Selecting `c_s²=1/7` can align the FTD-0411 leading slope, but
FTD-0412 proves that no scalar Wilson `r` also matches the q4-free flux pole.
A coordinate rescaling can normalize one sector, not make unequal sector
ratios disappear.

FTD-0413 enlarges the kinetic symbol to the face-diagonal Moore ansatz
`K_i=sin(q_i)[(1-2b)+b(cos(q_j)+cos(q_k))]`. Inside that ansatz, q4
cancellation uniquely fixes `b=1/3`, `r²=4/3`; with selected `c_s²=1/7`, the
free flux and Wilson-matter cones agree through q4 and all seven doublers stay
gapped. This is a default-off, reverse-solved standalone prototype. The poles
disagree at q6, the clocks differ (flux Floquet versus matter RK4), and no
manifested/gauge/gravity or interacting cone is included.

### 5.2 Manifestation dependency cone

The state sector updates from the Moore neighborhood, whose dependency hull reaches an axis neighbor at Euclidean distance `1`, a face diagonal at `sqrt(2)`, and a body diagonal at `sqrt(3)` per tick. The flux-wave cone advances at leading speed `1/sqrt(3)` voxel per tick. FC-2 may declare Lorentz scope only for weak flux, but physical matter and detectors live in the manifestation sector. No current theorem proves that the faster microscopic dependency channels are operationally unable to carry a signal into low-energy observables.

The required result is a measured or proved **common-cone theorem** covering flux, matter, bound states, clocks, gauge excitations, and gravity. Sector-scoping the metric avoids a contradiction inside the free flux equation; it does not recover physical Lorentz invariance.

---

## 6. The dimension-six/CPT protection argument fails

A preferred timelike direction `n^mu` plus spatial cubic symmetry, translations, gauge symmetry, parity, and CPT still permits CPT-even dimension-four operators. Schematic examples are

\[
\begin{aligned}
\mathcal O_{J,4}&=(n\!\cdot\!\partial J_a)^2,\\
\mathcal O_{F,4}&=n_\mu F^{\mu\nu}n^\rho F_{\rho\nu},\\
\mathcal O_{\psi,4}&=i\,n_\mu n_\nu\,\bar\psi\gamma^\mu
\overleftrightarrow D^{\nu}\psi.
\end{aligned}
\]

These operators distinguish time from space while preserving CPT. Cubic symmetry can equalize the three spatial axes; it cannot equate temporal and spatial kinetic coefficients, and it cannot force different particle species to share a speed.

Consequently, “the first tree-level correction is CPT-even dimension six” supplies no radiative protection. If a dimension-six coefficient `C_6/M^2` participates in loops regulated at a preferred-frame cutoff `Lambda`, power counting permits

\[
\delta c_4\sim\frac{g^2}{16\pi^2}\,C_6\frac{\Lambda^2}{M^2}.
\]

When the Lorentz-breaking and cutoff scales are both the lattice scale, `Lambda approximately M approximately a^{-1}`, the correction is loop-suppressed but not Planck-suppressed by the external energy. CPT does not remove it because the generated operator is also CPT even.

This is the precise exposure identified by Collins, Perez, Sudarsky, Urrutia, and Vucetich: a preferred UV regulator coupled to ordinary interactions generically feeds unsuppressed Lorentz violation into renormalizable kinetic terms unless a protection mechanism or tuning is supplied. Their calculation is not automatically the one-loop calculation of FTD; FTD still needs its own interacting action and matching calculation. It does, however, invalidate the claim that dimension and CPT parity alone constitute that mechanism.

Known ways around the naturalness problem add substantive structure: an exact custodial symmetry, supersymmetry with controlled breaking, a large hierarchy between the Lorentz-breaking and mediation scales, an RG attractor demonstrated for all sectors, or explicit counterterm tuning. None is presently derived for the FTD update.

---

## 7. Hard recovery contract

No future “Lorentz recovered” claim is licensed until all gates below pass.

| Gate | Required deliverable | Failure condition |
|---|---|---|
| **LR-0 — full symbol** | Exact temporal-plus-spatial pole for every propagating linearized mode | Use of a spatial-only dispersion or directional anisotropy as a boost test |
| **LR-1 — stable tree improvement** | A local update that is stable on the full Brillouin zone and removes or quantitatively bounds the dimension-six preferred-frame pole term | **Passed in the default-off CPU free-linear prototype by FTD-0408.** This does not imply a common cone or interacting stability. |
| **LR-2 — common cone** | Renormalized limiting-speed matrix for flux, each matter irrep/species, bound states, clocks, gauge modes, and gravity; one redundant overall time normalization removed | Any independent leading speed remains untuned or unmeasured |
| **LR-3 — interacting closure** | One action/transfer object generating the coupled dynamics, followed by 1PI two-point matching or a nonperturbative equivalent that extracts all symmetry-allowed dimension-3/4 coefficients | “Tree d=6 and CPT even” used as protection without calculating operator mixing |
| **LR-4 — Ward/unitarity compatibility** | Gauge Ward identities, positive physical spectrum, and common-cone stability after interactions and blocking | A free correlator pass used to infer interacting covariance |
| **LR-5 — phenomenology** | Map the extracted coefficients into the SME basis and compare sector by sector with current bounds, including threshold and birefringence constraints | Planck suppression asserted before lower-dimensional coefficients are bounded |
| **LR-6 — operational boosts** | At least two composite clocks/rods in relative motion reproduce the same finite-lattice boost law with a predeclared error scaling and no faster manifestation signal | One axis-aligned wave-clock or one field species used as the whole test |

FTD-0411–0413 take the selected BCC-time/SC+FCC-space branch. The free flux and
standalone Wilson-matter poles now agree through q4 at `c²=1/7`; the next
forced decision is a common exact discrete q6 clock or an explicit acceptance
and matching of the dimension-eight mismatch. Manifested matter, gauge,
gravity, and LR-3 through LR-6 remain untouched.

FTD-0409 closes the first fixed-cone alternatives: scalar temporal periods two
and three and the minimal positive-gap Hermitian auxiliary cannot retain
`c²=1/3` while cancelling dimension six and remaining full-band stable. It also
constructs a stable degree-four target trace, so the remaining free problem is
its P4-local realizability, not abstract polynomial stability.

---

## 8. Canonical corrections caused by this audit

1. FTD-0092 remains a valid spatial-rotation result, but its “Pillar 3 (Lorentz covariance) PASS” consequence is superseded.
2. `C_WAVE=1/sqrt(3)` remains the selected production value, but not the unique CFL saturation of the production stencil.
3. The direct free-flux dispersion remains Planck-suppressed under the Planck-spacing calibration; that statement is explicitly **tree-level and sector-scoped**. It cannot be promoted to “no observable vacuum Lorentz violation” until LR-2 through LR-5 pass.
4. The Wilson-Dirac campaign validates the imported operator against its own analytic spectrum. FTD-0413 additionally establishes a selected free flux/Wilson-matter cone through q4 only; it does not establish a live or interacting common cone.

---

## 9. External technical anchors

- Collins et al., [“Lorentz invariance and quantum gravity: an additional fine-tuning problem?”](https://arxiv.org/abs/gr-qc/0403053), *Phys. Rev. Lett.* **93**, 191301 (2004).
- Colladay and Kostelecky, [“Lorentz-Violating Extension of the Standard Model”](https://arxiv.org/abs/hep-ph/9809521), *Phys. Rev. D* **58**, 116002 (1998).
- Groot Nibbelink and Pospelov, [“Lorentz Violation in Supersymmetric Field Theories”](https://arxiv.org/abs/hep-ph/0404271), *Phys. Rev. Lett.* **94**, 081601 (2005).
- Pospelov and Shang, [“On Lorentz violation in Horava-Lifshitz type theories”](https://arxiv.org/abs/1010.5249), *Phys. Rev. D* **85**, 105001 (2012). Their viable suppression requires a separate hierarchy `Lambda_HL << M_P`, not CPT parity alone.
- Maccione et al., [“Planck-scale Lorentz violation constrained by Ultra-High-Energy Cosmic Rays”](https://arxiv.org/abs/0902.1756), *JCAP* 04 (2009) 022.
- Kostelecky and Russell, [“Data Tables for Lorentz and CPT Violation”](https://arxiv.org/abs/0801.0287), January 2026 update.

---

## 10. Reproduction

```powershell
python scripts/proofs/proof_lorentz_recovery_hard.py
```

Expected terminal line:

```text
VERDICT  LEADING-ORDER-FLUX-ONLY
```
