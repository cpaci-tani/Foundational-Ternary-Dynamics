# Quarter-Rotation on the 2-Sphere: A Visualization Companion to FQCR

**Date:** 2026-05-08
**Status:** Exploratory — visualization companion to [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md). Not a new theorem; not on the algebraic spine; no LEDGER row.
**Epistemic class:** [THEOREM] for the algebraic identities in §2 and §3 (elementary). [REFERENCE] for the table in §4.
**Category:** 9 (Mathematical Connections)

---

## Depends On

- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) — Definition 1 ($J^2 = -I$), Definition 2 (quarter-twisted spectra), Proposition 4 ($z^2 - sz + 1 = 0$).
- [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) — Theorem 1 ($G^* = \Gamma(1/4)/\Gamma(3/4)$), Theorem 2 (master quadratic).
- [`REF_QCR_TRILOGY_BRIDGE.md`](REF_QCR_TRILOGY_BRIDGE.md) — branch-side / curve-side / compatibility correspondence (FTD-0144).

---

## Honesty Note

This is a one-purpose pedagogy document. It records a clean 3D realization of the quarter-rotation operator $J$ on the 2-sphere, plus a side-by-side table of the three trace values $s$ that show up across FTD/FQCR. **No new physics claim, no new mathematical theorem.** The motivation is hygiene: an external note (`quarter_rotation_split_geometry_Gstar.md`) silently identified $s = G^*$ in the projective recurrence $z^2 - sz + 1 = 0$. That selection is **not** the canonical FQCR / master-quadratic trace. This doc keeps the bits of that note that are independently useful and pins down which trace is which.

---

## §1 — What FQCR already gives us

[`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) Definition 1 fixes $J^2 = -I$ as the primitive conjugacy operator. Definition 3 + Proposition 4 reduce the symmetric recurrence $u_{m+1} + u_{m-1} = s\,u_m$ to its projective form

$$
z_{m+1} = s - 1/z_m, \qquad z^2 - s\,z + 1 = 0, \qquad z_+ z_- = 1.
$$

For real $s$ with $|s| > 2$, the discriminant is positive; both branches are real and reciprocal. Setting $s = 2\cosh\chi$ gives $z_\pm = e^{\pm\chi}$. None of this is new; it is standard for any monic reciprocal quadratic with real trace.

The question this doc addresses is: **which value of $s$?**

---

## §2 — The quarter-rotation split-metric identity ([THEOREM], elementary)

Let $J : \mathbb{R}^2 \to \mathbb{R}^2$ act as multiplication by $i$ on $\mathbb{C} \simeq \mathbb{R}^2$, so $J^2 = -I$. Then for any real $x, y$:

$$
x^2 + (J y)^2 = x^2 - y^2.
$$

**Proof.** $(Jy)^2 = J^2 y^2 = -y^2$. ∎

On the unit circle parameterized by $x = \cos\theta$, $y = \sin\theta$:

$$
x^2 - y^2 = \cos^2\theta - \sin^2\theta = \cos(2\theta).
$$

Hence the Bernoulli lemniscate $r^2 = a^2 \cos(2\theta)$ is the radial readout of the quarter-rotated circular kernel. Standard. The reason to write it down here is that the original note's "circle → quarter-rotation → split metric → light-cone OR hyperbola OR lemniscate" pictorial chain is genuinely useful for the manuscript, but every step is elementary.

---

## §3 — 3D quarter-rotation preserving sphere closure ([THEOREM], elementary)

This is the only piece of the external note that is not redundant with material already in the FTD/FQCR doc-set. Define

$$
p(\theta, \phi) := \big(\cos\theta,\ \cos\phi\,\sin\theta,\ \sin\phi\,\sin\theta\big).
$$

$\theta$ runs around the original circle in the $xy$-plane; $\phi$ rotates the second coordinate into the $xz$-plane. At $\phi = 0$ this is the unit circle in the $xy$-plane; at $\phi = \pi/2$ the second axis has fully tipped into the third dimension.

**Sphere closure.** $\cos^2\phi + \sin^2\phi = 1$, so

$$
x^2 + y^2 + z^2 = \cos^2\theta + (\cos^2\phi + \sin^2\phi)\,\sin^2\theta = 1.
$$

Sphere invariant is preserved at every $\phi$.

**Split-metric readout.** Define $M(\theta, \phi) := x^2 + y^2 - z^2$. Then

$$
M(\theta, \phi) = \cos^2\theta + (\cos^2\phi - \sin^2\phi)\,\sin^2\theta = \cos^2\theta + \cos(2\phi)\,\sin^2\theta.
$$

At $\phi = \pi/2$: $\cos(2\phi) = -1$, hence $M(\theta, \pi/2) = \cos^2\theta - \sin^2\theta = \cos(2\theta)$ — the lemniscatic angular kernel.

**One sentence.** A 3D quarter-rotation of one axis in the $\mathbb{R}^3$ embedding moves continuously from a circle (no split signature) to a curve whose split-metric readout is exactly the Bernoulli kernel, while the Euclidean sphere invariant $x^2 + y^2 + z^2 = 1$ holds throughout. Useful for whitepaper / manuscript figures; not load-bearing.

---

## §4 — Three traces in play (read this before identifying $s$ with anything)

The recurrence $z^2 - sz + 1 = 0$ admits any real $s$. Three values of $s$ recur in FTD/FQCR contexts. They give visibly different rapidities and reciprocal branches:

| Trace $s$ | Origin | $\chi = \operatorname{arcosh}(s/2)$ | $q = e^{-\chi}$ | Where it appears |
|---|---|---:|---:|---|
| $G^* \approx 2.95868$ | Quarter-Gamma reflection ratio $\Gamma(1/4)/\Gamma(3/4)$ | $0.94371$ | $0.38918$ | The external note's [SELECTION]; not canonical anywhere in FTD/FQCR. |
| $4\sqrt{G^*} \approx 6.88032$ | FQCR Model V at $R_N(t) = 1$, $N \to \infty$. Equivalently the master quadratic $x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$ rescaled by $x = 4(G^*)^{3/2} z$. | $1.90684$ | $0.14855$ | [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) Proposition 5; [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) Theorem 2. |
| $3$ (integer / golden trace) | $z^2 - 3z + 1 = 0$; roots $\varphi^{\pm 2} = (3 \pm \sqrt{5})/2$ | $0.96242$ | $0.38197 = \varphi^{-2}$ | Comparison baseline; classical golden-ratio identity. |

**Numerical note.** $G^* \approx 2.9587$ is close to $3$ in absolute terms ($\approx 1.4\%$ below). The two rapidities $\chi(G^*) \approx 0.94371$ and $\chi(3) \approx 0.96242$ differ by $\approx 1.9\%$; the suppressed branches $0.38918$ and $0.38197$ differ by $\approx 1.9\%$. **This near-coincidence is not a derivation hook**. The look-elsewhere discipline of [FTD-0097](../07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) applies: any numerical match between two values that are individually $O(1)$ and $\approx 1\%$ apart is chance-level on a per-target basis.

The genuinely structural quantity in FTD's spine is the **second** row, $s = 4\sqrt{G^*}$, because that is the trace forced by the master quadratic. The first row is the trace that drops out if one lazily writes $z^2 - G^* z + 1 = 0$; it has no current dynamical reading.

Verification of these numerics: see `scripts/constants.py` (canonical `G_STAR`, `PHI`); independent reproduction is one Python session away — `arcosh(G_STAR/2)` and `arcosh(2*sqrt(G_STAR))` should match the table to all displayed digits.

---

## §5 — Status

| Item | Statement | Tag |
|---|---|---|
| QRSV-1 | $J^2 = -I \Rightarrow x^2 + (Jy)^2 = x^2 - y^2$ | [THEOREM] (elementary; subsumed by [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) Definition 1) |
| QRSV-2 | $x^2 - y^2 = \cos(2\theta)$ on the unit circle | [THEOREM] (elementary trig identity) |
| QRSV-3 | $p(\theta, \phi)$ preserves $x^2+y^2+z^2 = 1$; $M(\theta, \pi/2) = \cos(2\theta)$ | [THEOREM] (elementary; new presentation) |
| QRSV-4 | The three-trace table in §4 is the canonical disambiguation reference | [REFERENCE] |
| QRSV-5 | $s = G^*$ in the projective recurrence has dynamical meaning | NOT CLAIMED (the external note tagged the corresponding QRG-012 as [CONJECTURE / INTERPRETATION]; this doc takes no position) |
| QRSV-6 | $G^*$-weighted lemniscatic deformations $r_G^2(\theta) = a^2(x_G \cos^2\theta - y_G \sin^2\theta)$ are physically meaningful | NOT CLAIMED (the external note's QRG-013 [CONJECTURE]; not imported here) |

---

## §6 — Cross-references

- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) — canonical $J$-operator + recurrence framework. The branch-side material in §1–§3 is a re-derivation of this in $\mathbb{R}^2 / \mathbb{R}^3$ language.
- [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md) §10 — Theorem 8 (1+i)-tower; same $Z_4$ structural anchor.
- [`REF_QCR_TRILOGY_BRIDGE.md`](REF_QCR_TRILOGY_BRIDGE.md) — branch / curve / compatibility correspondence. The §3 sphere construction is a 3D enrichment of the curve-side picture; not a substitute for it.
- [`EXPLR_HALF_MOBIUS_LEMNISCATE.md`](EXPLR_HALF_MOBIUS_LEMNISCATE.md) — $Z_4$ topology in molecular orbitals; same lemniscatic kernel from a different application angle.
- [`EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md`](EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md) — Born-rule / Joukowski-transform reading of the same circle→lemniscate map. Independent route to the same kernel; not connected to the trace-disambiguation question.

---

## §7 — Provenance

External note `quarter_rotation_split_geometry_Gstar.md` (shared 2026-05-08; not authored by the project). Mathematically correct; large overlap with [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md); sole novel selection ($s = G^*$ directly) was unmotivated and inconsistent with the canonical FQCR / master-quadratic trace $4\sqrt{G^*}$. This doc imports only the elementary §3 sphere construction and adds the §4 disambiguation table that the external note lacked.
