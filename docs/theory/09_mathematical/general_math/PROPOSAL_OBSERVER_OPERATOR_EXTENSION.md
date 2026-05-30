# Proposal · Observer Operator Extension for FQCR Model V (C1 sketch)

**Date:** 2026-05-08
**Status:** [PROPOSAL / SKETCH] — outlines the C1 closability path from [`EXPLR_FQCR_OBSERVER_TESTS_SUITE.md`](../fqcr_program/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md) §5. Not derived; not numerically verified end-to-end. The structural constraint identified in §5 is, however, a [THEOREM] of the existing operator stack.
**Tag impact:** none. This is brainstorming the operator extension that would, if realized, upgrade FTD-0013 [SMC] toward [DERIVED]. No upgrades performed here.
**Companion:** [`EXPLR_FQCR_OBSERVER_TESTS_SUITE.md`](../fqcr_program/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md), [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md), [`REF_REFERENCE_FRAME_VOCABULARY.md`](../01_reference/REF_REFERENCE_FRAME_VOCABULARY.md).

---

## §1 — The C1 question

[`EXPLR_FQCR_OBSERVER_TESTS_SUITE.md`](../fqcr_program/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md) §5 closed with three paths for SPEC_FQCR §6 Test 3:

- **(C1)** Extend the operator stack to include observer / vacuum-polarization content, so $x_+(t)$ matches QED running.
- **(C2)** Reinterpret $t$ as a non-RG parameter (genus, holographic radial, etc.).
- **(C3)** Accept $t = 1$ identification as structural-only.

This doc sketches what (C1) would look like. It does not perform the calculation.

---

## §2 — FTD's existing "observer" inventory

[`REF_REFERENCE_FRAME_VOCABULARY.md`](../01_reference/REF_REFERENCE_FRAME_VOCABULARY.md) (2026-05-01) explicitly disambiguates "observer." The relevant entries:

| Vocabulary term | Meaning | Operator-theoretic content |
|---|---|---|
| **Reference frame** | A voxel/region instantiating reference frame structure (canonical: 27-block center) | Spatial location; not an operator |
| **Reference frame structure** | The configuration that gives reference frame structure (3³ block + Moore-26) | Combinatorial; not an operator |
| **Reference frame projection** | Operator mapping a reference frame structure's full state to its self-readout | **Operator** — corresponds to observable-algebra restriction |
| **Frame-relative readout** | Output of a reference frame projection | The "observed" value |
| **Frame-relative eigenmode** | Eigenstate of the reference frame projection | The master quadratic spectrum $\{x_+, x_-\}$ are *hypothesized* to be frame-relative eigenmodes (MC-T4.3) |
| **Observation layer** | A subalgebra of physical observables | **Operator-algebra restriction** |
| **Active-frame system** | A reference frame structure realizing frame dynamics in dynamics | Different from observer in the measurement sense |

The vocabulary doc explicitly states (line 83): *"'Observer' conflates three things: pick one."*

For C1 — making FQCR Model V give physical RG running — the operator-theoretic reading we need is **observation layer**: a subalgebra restriction that traces over the part of the operator content that's "not observed" and leaves an effective theory of the observed sector.

This is the algebraic-QFT reading of measurement. In QED specifically: the observable subalgebra is the gauge-invariant photon sector; the integrated-out sector is the fermion fields. The integrate-out operation produces vacuum polarization, which gives running α(μ).

---

## §3 — Reading: Observer = Trace-out over the Fermion Sector

In QED:

$$
\mathcal{Z}_{\text{full}}[\mathcal{A}] = \int \mathcal{D}\psi \mathcal{D}\bar\psi \mathcal{D}\mathcal{A} \, e^{-S_\text{QED}[\mathcal{A}, \psi]}
$$

The **photon-only effective action** comes from integrating out the fermions (the "observer" = "what the photon sees"):

$$
\mathcal{Z}_\text{photon}[\mathcal{A}] = \int \mathcal{D}\mathcal{A} \, e^{-S_\text{eff}[\mathcal{A}]}, \qquad
S_\text{eff}[\mathcal{A}] = S_\text{Maxwell}[\mathcal{A}] - \log\det(\not{D}[\mathcal{A}]).
$$

The functional determinant $\det \not{D}$ encodes the running of $\alpha(\mu)$ via vacuum polarization. At one loop with leptons of mass $m_l$:

$$
\alpha^{-1}(\mu) - \alpha^{-1}(0) = -\frac{2}{3\pi} \sum_{l\,:\,m_l < \mu} \log(\mu/m_l).
$$

**The observer in the QED-running sense is the trace-out / observation-layer restriction over the fermion fields.** That is the operator we need to add to FQCR Model V.

---

## §4 — The structural coefficient: a [THEOREM] from the existing stack

Before sketching the extension, an observation that constrains it:

**[THEOREM] (derived in this doc, machine-precision verified in [`scripts/exploration/explore_fqcr_t_scale_map.py`](../../../scripts/exploration/explore_fqcr_t_scale_map.py)).** The slope $\partial x_+ / \partial R$ of the FQCR Model V branch readout at the base point ($R = 1$, $N \to \infty$) is

$$
\left.\frac{\partial x_+}{\partial R}\right|_{R=1} = -\frac{2(G^*)^{3/2}}{\sqrt{4G^* - 1}} = -\frac{G^*}{\delta} \approx -3.092
$$

where $\delta = \sqrt{(4G^*-1)/(4G^*)}$.

*Proof.* From $x_+(R) = 8(G^*)^2 + 4(G^*)^{3/2}\sqrt{4G^* - R}$, differentiate: $\partial_R x_+ = -2(G^*)^{3/2}/\sqrt{4G^* - R}$. At $R = 1$, this equals $-2(G^*)^{3/2}/\sqrt{4G^* - 1}$. Algebraic simplification using $\sqrt{4G^* - 1} = 2\sqrt{G^*}\,\delta$ gives $-G^*/\delta$. ∎

**Numerical:** $G^*/\delta \approx 2.9587/0.9568 \approx 3.092$.

This is the **chain-rule factor** between any "running of $R$" and "running of $x_+$." For the extension to give QED-faithful α running:

$$
\frac{\partial x_+}{\partial \log\mu} = -\frac{G^*}{\delta} \cdot \frac{\partial R}{\partial \log\mu} = -\frac{2 N_\text{active}}{3\pi}.
$$

So:

$$
\boxed{\frac{\partial R}{\partial \log\mu} = +\frac{2\delta\,N_\text{active}}{3\pi\,G^*} \approx +0.0686 \cdot N_\text{active}.}
$$

This is the **target slope** that any observer-operator extension must produce. Notably it is **not** $-2/(3\pi)$ directly: it differs by a factor $\delta/G^* \approx 0.323$, or equivalently ~$1/3$. The fermion-loop content in $R(t)$ must be normalized by this factor.

(The factor-of-3 here is structurally suggestive: $G^*/\delta \approx 3.09$ is close to but not exactly $N_c \approx 3.024$. Whether this is structural or numerical coincidence is open.)

---

## §5 — Sketch of the operator extension

### §5.1 — Where the new operators live

Add to the FQCR operator stack:

$$
\mathcal{T}_N^{\text{ext}}(t) = \mathcal{T}_N(t) \cup \{D^{(e)}_N, D^{(\mu)}_N, D^{(\tau)}_N, \ldots\}
$$

where each $D^{(l)}_N$ is a **truncated Dirac-style operator** for lepton $l$, parameterized by its mass in lattice units $a_l := m_l / m_*$. With the dimensional-map calibration $m_* = m_e / K_B = 1\,\text{MeV}$:

| Lepton | $a_l = m_l / m_*$ |
|---|---:|
| electron ($m_e = 0.511$ MeV) | $0.511$ |
| muon ($m_\mu = 105.7$ MeV) | $105.7$ |
| tau ($m_\tau = 1777$ MeV) | $1777$ |

Each operator generates a contribution $B^{(l)}_N(t)$ to the response function:

$$
R^{\text{ext}}_N(t) = 1 + \lambda_N(4it) + A_N(t) + \sum_l B^{(l)}_N(t).
$$

### §5.2 — What $B^{(l)}_N(t)$ should look like

The structural constraints are:

1. **Decoupling:** $B^{(l)}_N(t) \to 0$ as $\mu(t) \to 0$ (lepton heavier than scale; loop frozen).
2. **Logarithmic running:** $\partial B^{(l)}/\partial \log\mu \to +2\delta/(3\pi G^*)$ as $\mu(t) \to \infty$ (lepton loop active; QED $b_0$ saturated).
3. **Smooth threshold:** crossover at $\mu \sim m_l$, smooth (no kinks).
4. **Modular structure:** if FQCR is to remain in the same operator-determinant style as $\Psi_N$, then $B^{(l)}$ should be a logarithmic derivative of a finite-product partition function.

A natural candidate (Bose-style for now; fermion sign conventions to be sorted):

$$
B^{(l)}_N(t) := C \cdot \log\det\bigl[1 - Q^{a_l}\,\Pi_N\bigr]
$$

where $\Pi_N$ is a projection / heat-kernel operator on the same finite spectrum as $L_{1/4,N}$, $C$ is a coefficient encoding $b_0$ structure, and $Q = e^{-2\pi t}$. The truncation $\Pi_N$ keeps the construction reframe-compatible.

For each lepton: when $Q^{a_l} \approx 0$ (small $t$, heavy lepton frozen), $B^{(l)} \approx 0$. When $Q^{a_l} \approx 1$ (large $t$ or light lepton, fully on-shell), the determinant develops zeros and $B^{(l)}$ grows logarithmically.

A fermionic version replaces $(1 - Q^{a_l n})$ with $(1 + Q^{a_l n})$ (Pauli statistics):

$$
B^{(l)}_{N,\text{fermion}}(t) := C_l \cdot \sum_{n=1}^N \log\bigl(1 + Q^{a_l n}\bigr)
$$

with $C_l = -2 \delta / (3 \pi G^* \cdot Q_l^2 \cdot\text{normalization})$ for charge $Q_l = -1$.

The **specific form** must be chosen such that:

- $\partial B^{(l)}/\partial \log\mu \to 0$ for $\mu \ll m_l$ (decoupling).
- $\partial B^{(l)}/\partial \log\mu \to +2\delta/(3\pi G^*)$ for $\mu \gg m_l$ (running active).
- The transition region is structurally smooth.

Identifying the right form is a model-building problem.

### §5.3 — The chicken-and-egg with the t-scale map

The fermion thresholds are at specific $\mu$-values ($m_e, m_\mu, m_\tau$), which require a $t \leftrightarrow \mu$ map. But the t-scale map is precisely what we're trying to derive via C1.

Resolution: solve self-consistently. Pick an ansatz t-scale map (e.g., heat-kernel: $\mu = m_*/t$). Define $B^{(l)}_N(t)$ with thresholds at $t = m_*/m_l$. Compute $x_+(t)$ across the muon and tau thresholds. Verify the slope at each plateau matches QED. Iterate the t-map if not.

Under heat-kernel Map A with $m_* = m_e$:

| Threshold | $a_l$ | $t_l = 1/a_l$ |
|---|---:|---:|
| electron | 1 | 1.0 |
| muon | 207 | 0.0048 |
| tau | 3477 | 0.000288 |

Under Map A, the muon threshold sits at $t \approx 0.0048$ — **below the FQCR Landau-like point at $t_* \approx 0.062$**. So Map A is structurally too compressed to reach muon and tau thresholds. The t-scale map needs to extend further toward small $t$, OR the framework needs continuation past the Landau-like point.

This is a real structural challenge. Possible resolutions:

- **(R1)** A different t-scale map with wider $\mu$-range. Logarithmic Map B compressed even more, so doesn't help. A faster-than-linear map ($\mu = m_e \cdot t^{-p}$ for $p > 1$) might extend the range.
- **(R2)** Continuation of the FQCR machinery past $t_* \approx 0.062$, where the discriminant is complex but the analytic continuation might still encode physical content. This is reminiscent of analytic continuation past Landau singularities in QFT.
- **(R3)** Interpret $t_* \approx 0.062$ as a structural feature *separate from* the running region, and have the running encoded entirely in $0.062 < t < 1$, with thresholds compressed into that interval via a non-trivial t-scale map.

(R3) is the cleanest if it works, because it preserves the framework's structural boundary. Under (R3), the t-scale map would need to compress all three lepton thresholds into the $(0.062, 1)$ interval — concretely:

- $t = 1$ ↔ $\mu \ll m_e$ (deep IR; α = 137.036)
- $t \approx 0.5$ ↔ $\mu \approx m_e$ (electron threshold)
- $t \approx 0.2$ ↔ $\mu \approx m_\mu$ (muon threshold)
- $t \approx 0.07$ ↔ $\mu \approx m_\tau$ (tau threshold)
- $t = 0.062$ ↔ $\mu = $ Landau-pole-analog or unification scale

This requires a t-scale map of the form $\mu(t) = m_e \cdot f(t)$ where $f$ is a specific squashing function. The form is constrained by the threshold positions and by the slopes at each plateau.

---

## §6 — What would close C1

The technical work needed to actually upgrade FTD-0013 [SMC] toward [DERIVED] via C1:

1. **Specify the operator $B^{(l)}_N(t)$ form** structurally (not phenomenologically). The form should follow from the operator stack's existing logic — heat-kernel / determinant structure — not from "fit to QED."
2. **Prove or compute** that $\partial B^{(l)}/\partial\log\mu$ has the right asymptotics (decoupling below threshold, $+2\delta/(3\pi G^*)$ above).
3. **Derive the t-scale map** as the squashing function that places the lepton thresholds correctly. Equivalently: derive what the heat-kernel parameter $t$ "really is" so that the threshold positions fall out structurally, not by tuning.
4. **Verify the resulting $x_+(t)$ matches QED running** across $\mu \in [m_e, m_\tau]$ to phenomenologically-relevant precision.

If steps 1-4 succeed, FTD-0013 has an end-to-end derivation chain: $J^2 = -I \to G^* \to$ master quadratic $+ $ observer-extended R(t) $\to$ QED-faithful $\alpha(\mu)$. That would be a [DERIVED] tag, not [SMC].

---

## §7 — Concrete next computation

**Smallest meaningful test:** introduce a single phenomenological electron-loop term and check whether the framework's running matches QED across the moderate-$\mu$ region $\mu \in [m_e, m_\mu]$ (where only the electron is active).

Specifically: define

$$
B^{(e)}_N(t) := c_e \cdot \log\bigl(1 + (\mu(t)/m_e)^2\bigr)
$$

with $\mu(t) = m_e/t$ (Map A) and tune $c_e$ such that $\partial x_+/\partial\log\mu \to -2/(3\pi)$ for $\mu \gg m_e$. From §4: $c_e = -2/(3\pi)/(G^*/\delta) = -0.0686$. Then check whether the resulting $x_+(t)$ for $t \in (0.0048, 1)$ (above muon threshold = below muon threshold in $\mu$) matches QED's electron-only running quantitatively.

**Outcomes:**

- **If yes** with $c_e = -0.0686$: then the structural normalization factor $\delta/G^*$ has physical meaning, and the C1 path is open. Next step: add muon and tau terms similarly and check across all thresholds.
- **If yes only after adjusting $c_e$**: the normalization isn't structural, and we've rediscovered "QED with a knob fitted to data." [SELECTION], not [DERIVED].
- **If no** even with optimal $c_e$: phenomenological log-form isn't right; need different ansatz.

I have not run this test in this session because the t-scale map question is a prerequisite, and the §5.3 chicken-and-egg means we'd be tuning two things simultaneously. Recommend doing this with a fixed t-scale map first (Map A as starting point) and a single-lepton $B^{(e)}$ term, then iterating.

---

## §8 — Status

| Item | Statement | Tag |
|---|---|---|
| OOE-1 | "Observer" in FTD has 3+ distinct readings; the QED-running-relevant one is observation-layer (trace-out over fermions) | [REFERENCE] (per REF_REFERENCE_FRAME_VOCABULARY) |
| OOE-2 | $\partial x_+/\partial R\big|_{R=1} = -G^*/\delta \approx -3.092$ | [THEOREM] (algebraic, machine-precision verified) |
| OOE-3 | For QED-faithful running, $\partial R/\partial\log\mu = +2\delta/(3\pi G^*) \cdot N_\text{active}$ | [THEOREM] (consequence of OOE-2) |
| OOE-4 | The fermion-loop coefficient in $R(t)$ must be $\approx 1/3$ of naive QED $b_0$ | [THEOREM] (consequence of OOE-3) |
| OOE-5 | Under Map A, muon and tau thresholds sit below FQCR Landau-like point | [THEOREM] (numerical) |
| OOE-6 | The operator-extension form $B^{(l)}_N(t)$ has not been specified structurally | [OPEN] |
| OOE-7 | The t-scale map within the new operator stack has not been derived | [OPEN] |
| OOE-8 | C1 closability of FTD-0013 [SMC] → [DERIVED] | [OPEN — substantial program] |

---

## §9 — Cross-references

- [`EXPLR_FQCR_OBSERVER_TESTS_SUITE.md`](../fqcr_program/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md) §5 — the C1/C2/C3 trifurcation that this proposal addresses.
- [`SPEC_FQCR.md`](../01_reference/SPEC_FQCR.md) §3.2, §6 Test 3 — the [SELECTION] tags this proposal would ultimately upgrade.
- [`REF_REFERENCE_FRAME_VOCABULARY.md`](../01_reference/REF_REFERENCE_FRAME_VOCABULARY.md) — observer disambiguation; "observation layer" reading.
- [`FOUND_THE_RATIO_AND_THE_PRODUCT.md`](../02_foundations/FOUND_THE_RATIO_AND_THE_PRODUCT.md) §5 — the "observer = imaginary axis" reading; complementary to but distinct from the QED-running reading used here.
- [`AUDIT_DUAL_SUBSTRATE_PROVENANCE.md`](../07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md) — the [PROPOSITION] tag on $P = 16G^{*3}$ that may also receive operator-theoretic provenance via this extension.
- [`MATH_LOG_GSTAR_IDENTITY.md`](../number_theory/MATH_LOG_GSTAR_IDENTITY.md) §3.5 — Beilinson-conjecture framing of $G^*$ as a regulator-irreducible-sector generating function; the right framework for understanding what "extending the operator stack" actually means structurally.
