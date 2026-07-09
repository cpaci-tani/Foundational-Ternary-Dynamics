# FOUND — Electron-Primary Gauge: the proper entry into dimensionful units

**Tag:** [SYNTHESIS] + [RECOMMENDATION]. A gauge *choice* under FTD-0137, not a theorem; it promotes nothing and moves no tag. The gravity-as-output step rides at its source tags ([DERIVED ~0.19%] for the mass ladder, [SELECTION] for the √(2π) factor, [SMC] for the α_G identification — FTD-0015/FTD-0131).
**Program:** dimensional-boundary. **LEDGER:** maintenance-log line; content rides on FTD-0137 (gauge freedom), FTD-0015 (m_e/m_P ladder), FTD-0059/FTD-0096 (no-gos), FTD-0368 (grade-0 closure).
**Verification:** numerics reproduced inline (mpmath); the gate map is `DERIV_DIMENSIONAL_GATE.md`.
**Audience:** anyone choosing *how* FTD should enter absolute units — and why the current Planck-primary default is the wrong entry.

---

## §1 — The claim

FTD's content is dimensionless; reaching SI requires importing the Buckingham floor of **3** independent constants (grade-0 closure, FTD-0368). Two of the three are the **universal** unit-fixing constants every physical theory imports — `ℏ` and `c` (and `c`'s dimensionless value `1/√3` is even *native* to FTD). The real choice is the **one remaining scale**. The proper choice is the electron mass:

> **Electron-primary gauge.** Import `{ℏ, c, m_e}`. Derive everything else — all masses (via predicted ratios), all lengths and times (via `ℏ, c`), and **the Planck mass, ℓ_P, and G** (via the α-ladder).

Every dimensionful quantity `Q` of dimension `(a,b,c)` is then
$$Q_{\text{SI}} = \hat q \cdot m_e^{\,a}\Big(\tfrac{\hbar}{m_e c}\Big)^{b}\Big(\tfrac{\hbar}{m_e c^2}\Big)^{c},$$
with `q̂` the dimensionless FTD number (the gate of `DERIV_DIMENSIONAL_GATE.md`, specialized so the length unit is the Compton wavelength `ƛ_C = ℏ/m_e c ≈ 3.86×10⁻¹³ m` and the time unit is `ℏ/m_e c² ≈ 1.29×10⁻²¹ s`).

**Beyond the universal constants `{ℏ, c}`, electron-primary imports exactly ONE scale (`m_e`).**

---

## §2 — Why the current Planck-primary default over-imports

Planck-primary (the SPEC_FTD default, FTD-0041) fixes `a_phys ≡ ℓ_P` **and** `K_B = m_e`. Since `ℓ_P = √(ℏG/c³)`, importing `ℓ_P` (given `ℏ, c`) is equivalent to **importing `G`**. So beyond `{ℏ, c}`, Planck-primary imports **two** scales: `G` (via `ℓ_P`) and `m_e`.

But those two are **not independent** — FTD predicts the dimensionless ratio
$$\frac{m_e}{m_P} = \frac{m_e}{\sqrt{\hbar c/G}} = K\,\alpha^{11}, \qquad K \equiv \sqrt{2\pi}\cdot\tfrac{16}{3},$$
which is a *relation between `G` and `m_e`*. Importing both endpoints of a predicted ratio is one import too many, and the redundant one is **gravitational** — exactly the sector a discrete-gravity framework is trying to *explain*, not assume. `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` §4.1 already flags this con ("borrows continuous-physics machinery ℏ, G, c"); electron-primary is the resolution.

---

## §3 — The derivation chain

Import `m_e` (with `ℏ, c` universal). Then, in order:

1. **All other masses** — predicted ratios: `m_μ = 207 m_e`, `m_τ = 3477 m_e`, `m_p = 1836.47 m_e`, … (dimensionless, calibration-free).
2. **All lengths / times** — `ℏ = c = 1` fixes them off `m_e`: length unit `ƛ_C = ℏ/m_e c`, time unit `ℏ/m_e c²`.
3. **The Planck mass** — invert the ladder: `m_P = m_e/(K α¹¹)`.
4. **The Planck length** — `ℓ_P = ℏ/(m_P c)` (**derived**, not imported).
5. **Newton's G** — `G = ℏc/m_P² = ℏc\,(K α¹¹)²/m_e²` (**derived**).

Steps 3–5 are the move: the scales Planck-primary *imports*, electron-primary *outputs*.

---

## §4 — Gravity becomes a prediction (0.38%)

The load-bearing consequence: with `m_e` anchored, the gravitational fine-structure ratio of one electron is a prediction, not a fit:
$$\alpha_G(e,e) = \Big(\frac{m_e}{m_P}\Big)^2 = (K\,\alpha^{11})^2 \approx 1.745\times10^{-45},$$
vs. the measured `1.752×10⁻⁴⁵` — a **0.38% match** (FTD-0015; CLAUDE.md gravity row; the `G_N = 1/100` *identification* is separately falsified, FTD-0131, and is unrelated to this ratio). Numeric check: `m_e/m_P = Kα¹¹ ≈ 4.177×10⁻²³` (0.19% vs CODATA `4.185×10⁻²³`); squared → `1.745×10⁻⁴⁵`.

So electron-primary doesn't just remove the `G` import — it converts `G` into a sub-percent **output** of the α-ladder FTD already owns.

---

## §5 — The four properness tests

| Test | Planck-primary (current default) | **Electron-primary (proper)** |
|---|---|---|
| **Minimal** | 2 scales beyond `{ℏ,c}` (`ℓ_P`, `m_e`) — redundant | **1** scale beyond `{ℏ,c}` (`m_e`) |
| **Non-circular** | imports **G** (via `ℓ_P`) | **G derived** from the ladder |
| **Operational** | anchor (Planck scale) is 16–25 orders beyond any feasible lattice | anchor (`m_e`) is a lab number |
| **Spine-centered** | anchors a scale the spine *predicts* (`m_P`) | anchors the scale the spine is *built around* (`m_e`) |

Electron-primary is the *operational inverse* of the doc's existing **cluster-primary** gauge (FTD-0137 §4.2, which anchors `K_B ≡ m_P` and derives `m_e` — anchoring the **unmeasurable** endpoint). Same gauge freedom; electron-primary anchors the **measurable** endpoint.

---

## §6 — It does not violate the no-gos

`THEOREM_A_PHYS_NO_GO` (FTD-0059) forbids deriving a length from **Axiom Zero alone**; `THEOREM_MU_NO_GO` (FTD-0096) the same for mass. Electron-primary derives `ℓ_P` from `{ℏ, c, m_e}` — **imported** constants — not from Axiom Zero. The single mass import (`m_e`) is the no-gos' irreducible-import made explicit; the ladder that relates `m_e` to `m_P` is a dimensionless prediction, so using it to *convert* one imported anchor into other scales is legitimate, not a smuggled derivation. Grade-0 closure (FTD-0368) is respected: three constants cross the gate; electron-primary just chooses the G-free three.

---

## §7 — Honest caveats

- **`c` and `ℏ` remain imports.** SI cannot be reached below 3 constants; the win is *which* three (universal + leptonic, G-free) — not a smaller count in absolute terms. The "one import" framing is *beyond the universal constants*.
- **The gravity output rides at [SMC].** `m_e/m_P = Kα¹¹` is `[DERIVED ~0.19%]` with the `√(2π)` factor `[SELECTION]`; `α_G` is FTD-0015/FTD-0131 `[SMC]`. "G is a prediction" is *structurally* proper but epistemically `[SMC]`, not `[THEOREM]`. No promotion.
- **Dimensionless predictions are unchanged.** This is a gauge choice (FTD-0137); the falsifiable spine (α, mass ratios, mixing angles) is gauge-invariant. Only absolute-unit readings differ, and they agree with Planck-primary wherever the ladder is exact.
- **Adopted as the SPEC_FTD default (2026-07-08).** Electron-primary is now the declared default gauge (SPEC_FTD calibration section, constitution §3.3, LEDGER FTD-0041, CLAUDE.md); the legacy Planck-primary declaration remains a valid alternative gauge (FTD-0137 §4.1). The switch changes **no** prediction (gauge-invariant spine) — it re-anchors the import surface and reclassifies `a_phys`/`G` as derived-at-`[SMC]`.

---

## §8 — Cross-references

`DERIV_DIMENSIONAL_GATE.md` (the 3-slot gate this specializes); `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` (FTD-0137, the four-gauge menu — electron-primary sits alongside §4.1–4.4); `FOUND_DIMENSIONAL_GRADE_CLOSURE.md` (FTD-0368, why 3 imports are irreducible); `THEOREM_A_PHYS_NO_GO.md` (FTD-0059) + `THEOREM_MU_NO_GO_FTD0096.md` (FTD-0096); `SPEC_IMPORT_LEDGER.md` (IMP-K1/K3 pricing); `DERIV_ELECTRON_MASS_MOTIVATION.md` + FTD-0015 (the `m_e/m_P = Kα¹¹` ladder); `docs/theory/03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md` (the α_G(e,e) gravity line).
