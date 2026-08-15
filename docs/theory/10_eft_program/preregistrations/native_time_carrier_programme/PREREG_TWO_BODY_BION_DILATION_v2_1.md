# Preregistration: two-body bion dilation v2.1 (declared deviation model)

**Date locked:** 2026-08-15
**Status before execution:** `[PREREGISTERED — DEVIATION-MODEL CAMPAIGN]`
**Programme:** Universality Programme P1
(`SCOPE_UNIVERSALITY_PROGRAMME_v1.md`, DRAFT).
**Parents:** the FTD-1009 chain (locks `7e6840e9`/`8c97446d`/`84a61db8`/
`04e69d99`, all gates-failed, signal recorded as diagnostic) and the P1a
residual diagnostic (commit `2412115e`), which fixed the model form below
**before** this lock.

## 1. The data-selected model (diagnostic of record, quoted)

The P1a diagnostic regenerated all event sequences and found: **residual
structure secular-dominant** (rms up to 8.0×10⁻², u-scaling exponent
+2.91) over periodic (rms ≤ 3.4×10⁻², scaling +0.76, and the apparent
line at 0.0105 cycles/event is the second FFT bin in every cell — a
detrending artifact matching neither the site-crossing nor the envelope
reference). Per the diagnostic's own declared selection rule, the model is
the quadratic proper-stage form:

> **t_n(u) = γ̂ · t_n(0) · (1 + b₁·m + b₂·m²)**, m = (n − n_mid)/n_mid
> over matched events n ∈ [10, 200), with (b₁, b₂) per-cell nuisance
> parameters and γ̂ the physics estimand. Exact linear least squares; no
> iteration; no frequency window anywhere.

## 2. Pins

| artifact | SHA-256 |
|---|---|
| instrument `derive_two_body_bion_dilation_v2_1.py` | `49B4150444CF439F5AC90D748DDA4421CFD2D11FD6B4B2BE5E489F4BDFAFA295` |
| frozen v2 physics cells (imported) | `47DD26FC52ACD1050FD44A411F3003F560E3A6FDEA8338330871CEE9EDB18DBD` |

Physics cells byte-identical to v2 (lattice, integrator, proper-covariant
preparation, interpolated-peak probe, λ ∈ {0.03, 0.05}, fit u/C ∈ {0.25,
0.40, 0.50}, blind held-out 0.60, N = 8192 volume check). Event arrays are
**persisted** in the results JSON this time. No cell was fit under this
model before the lock (the diagnostic characterized residuals of the *v2*
fit; the v2.1 estimator runs here first).

## 3. Gates (declared; none loosened from v2)

G1 ≥ 60 matched events per cell. **G2 model residual R² > 0.9995 per fit
cell** — tighter than v2's raw 0.999, since the declared model must absorb
what broke it. G3 volume < 1% on γ̂. G4 per-λ γ̂ agreement < 3% at each
fit u. G5 blind held-out: pooled p̂ predicts γ̂(0.60) within 3% before the
held-out cells are read. Secondary estimand, no gate: the secular
amplitude's u-scaling exponent.

## 4. Outcomes

- **CONSISTENT WITH THE ADOPTED LAW + DECLARED LATTICE MODEL** — all
  gates and |p̂ − 1| ≤ 0.05: books the first quantitative composite-clock
  lattice-deviation measurement (γ̂ per cell + secular nuisance), the
  adopted clock law surviving its two-body substrate test under a declared
  deviation model. Conditional per the programme's binding ceiling.
- **DEVIATION CANDIDATE** — gates pass, p̂ outside the band:
  replication-gated escalation against FC-2's clock clause.
- **EXECUTION GATES FAILED** — per the FTD-1009 stopping discipline this
  is the chain's third-strike category: the surrogate line halts and the
  residual question routes to the engine replication (S2′). No further
  surrogate repairs.

Artifacts: console log; `results/two_body_bion_dilation_v2_1.json` (with
event arrays); LEDGER row post-run; lock tag
`preregister-two-body-bion-dilation-v2-1`.
